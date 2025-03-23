#include "web_server.h"

WebServerSPA::WebServerSPA(Config& config, int port) : server(port), config(config) {}

WebServerSPA::~WebServerSPA() {
    server.stop();
}

void WebServerSPA::begin() {
    if (!LittleFS.begin()) {
        Serial.println("Failed to mount LittleFS");
        return;
    }
    
    setupStaticFiles();
    setupApiRoutes();
    server.begin();
    Serial.println("Web server started");
}

void WebServerSPA::setupStaticFiles() {
    // Serve static files with caching headers
    server.serveStatic("/", LittleFS, "/www/index.html", "max-age=86400");
    server.serveStatic("/js", LittleFS, "/www/js", "max-age=86400");
    server.serveStatic("/css", LittleFS, "/www/css", "max-age=86400");
}

void WebServerSPA::setupApiRoutes() {
    // Add CORS pre-flight handler
    server.on("/api/*", HTTP_OPTIONS, [this]() {
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.sendHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
        server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
        server.send(204);
    });

    server.on("/api/config", HTTP_GET, [this]() { handleGetConfig(); });
    server.on("/api/wifi", HTTP_POST, [this]() { handleSetWifiConfig(); });
    server.on("/api/message", HTTP_POST, [this]() { handleSetMessageConfig(); });
    server.on("/api/volume", HTTP_POST, [this]() { handleSetVolume(); });
    server.on("/api/system", HTTP_POST, [this]() { handleSystemAction(); });
}

void WebServerSPA::handleGetConfig() {
    JsonDocument doc;
    
    doc["wifi"]["mode"] = config.isAccessPoint();
    doc["wifi"]["ssid"] = config.getWifiSSID();
    doc["wifi"]["channel"] = config.getWifiChannel();
    doc["wifi"]["hidden"] = config.isHiddenSSID();
    
    doc["message"]["active"] = config.getNumeroMessage();
    doc["message"]["count"] = config.getMessageCount();
    
    JsonArray messages = doc["messages"].to<JsonArray>();
    for(int i = 1; i <= config.getMessageCount(); i++) {
        JsonObject msg = messages.add<JsonObject>();
        msg["id"] = i;
        msg["text"] = config.getMessageText(i);
    }
    
    doc["volume"] = config.getVolume();
    
    sendJsonResponse(doc);
}

void WebServerSPA::handleSetWifiConfig() {
    if (!server.hasArg("plain")) {
        sendJsonError("No JSON data received");
        return;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));
    
    if (error) {
        sendJsonError("Invalid JSON");
        return;
    }

    if (doc["ssid"].is<const char*>() && doc["mode"].is<int>()) {
        String ssid = doc["ssid"].as<String>();
        String password = doc["password"] | "";
        bool isAP = doc["mode"].as<int>() == 1;
        uint8_t channel = doc["channel"] | 6;
        bool hidden = doc["hidden"] | false;

        config.setWifiConfig(ssid.c_str(), 
                           password.length() > 0 ? password.c_str() : config.getWifiPassword(), 
                           isAP);
        config.setWifiAdvanced(channel, hidden);
        config.saveConfig();
        
        sendJsonResponse(doc);
    } else {
        sendJsonError("Missing required fields");
    }
}

void WebServerSPA::handleSetMessageConfig() {
    if (!server.hasArg("plain")) {
        sendJsonError("No JSON data received");
        return;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));
    
    if (error) {
        sendJsonError("Invalid JSON");
        return;
    }

    String action = doc["action"] | "";
    int messageNum = doc["messageNum"] | 0;

    if (action == "setActive") {
        if (messageNum >= 1 && messageNum <= config.getMessageCount()) {
            config.setNumeroMessage(messageNum);
            config.saveConfig();
            sendJsonResponse(doc);
        } else {
            sendJsonError("Invalid message number");
        }
    } else if (action == "add") {
        int newIndex = config.getMessageCount() + 1;
        if (config.setMessageText(newIndex, "New Message")) {
            config.saveConfig();
            sendJsonResponse(doc);
        } else {
            sendJsonError("Failed to add message");
        }
    } else if (action == "remove" && messageNum >= 1) {
        for (int i = messageNum; i < config.getMessageCount(); i++) {
            config.setMessageText(i, config.getMessageText(i + 1));
        }
        config.removeLatestMessage();
        config.saveConfig();
        sendJsonResponse(doc);
    } else {
        sendJsonError("Invalid action");
    }
}

void WebServerSPA::handleSetVolume() {
    if (!server.hasArg("plain")) {
        sendJsonError("No JSON data received");
        return;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));
    
    if (error) {
        sendJsonError("Invalid JSON");
        return;
    }

    if (doc["volume"].is<int>()) {
        int volume = doc["volume"].as<int>();
        volume = constrain(volume, 0, 30);
        config.setVolume(volume);
        config.saveConfig();
        sendJsonResponse(doc);
    } else {
        sendJsonError("Missing volume parameter");
    }
}

void WebServerSPA::handleSystemAction() {
    if (!server.hasArg("plain")) {
        sendJsonError("No JSON data received");
        return;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));
    
    if (error) {
        sendJsonError("Invalid JSON");
        return;
    }

    String action = doc["action"].as<String>();
    
    if (action == "reset") {
        sendJsonResponse(doc);
        delay(500);
        ESP.restart();
    } else if (action == "clear") {
        if (LittleFS.remove("/config.json")) {
            sendJsonResponse(doc);
            delay(500);
            ESP.restart();
        } else {
            sendJsonError("Failed to clear configuration");
        }
    } else {
        sendJsonError("Invalid action");
    }
}

void WebServerSPA::handleClient() {
    server.handleClient();
}

void WebServerSPA::sendJsonResponse(const JsonDocument& doc) {
    String response;
    serializeJson(doc, response);
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Content-Type", "application/json");
    server.send(200, "application/json", response);
}

void WebServerSPA::sendJsonError(const char* message, int code) {
    JsonDocument doc;
    doc["error"] = message;
    String response;
    serializeJson(doc, response);
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.sendHeader("Content-Type", "application/json");
    server.send(code, "application/json", response);
}
