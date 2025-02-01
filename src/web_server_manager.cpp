#include "web_server_manager.h"

WebServerManager::WebServerManager(Config& config, int port) 
    : server(port), config(config) {
}

String WebServerManager::getHeader(const char* title) {
    String html = "<!DOCTYPE html><html><head>";
    html += "<title>" + String(title) + "</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>"
            "body{font-family:Arial;margin:20px;} "
            ".btn{background:#1abc9c;color:white;padding:10px 20px;text-decoration:none;border-radius:5px;display:inline-block;margin:5px;} "
            ".config-item{background:#f8f9fa;padding:15px;margin:10px 0;border-radius:8px;display:flex;align-items:center;} "
            ".config-label{font-weight:bold;min-width:150px;color:#2c3e50;} "
            ".config-value{color:#34495e;flex-grow:1;} "
            ".config-section{margin:20px 0;padding:20px;background:#fff;border-radius:10px;box-shadow:0 2px 4px rgba(0,0,0,0.1);}"
            "</style>";
    html += "</head><body>";
    html += "<h1>" + String(title) + "</h1>";
    html += "<nav><a href='/' class='btn'>Home</a> ";
    html += "<a href='/wifi' class='btn'>WiFi Config</a> ";
    html += "<a href='/message' class='btn'>Message Config</a></nav><br>";
    return html;
}

String WebServerManager::getFooter() {
    return "</body></html>";
}

String WebServerManager::formatConfigItem(const char* label, const String& value) {
    return "<div class='config-item'><div class='config-label'>" + String(label) + 
           "</div><div class='config-value'>" + value + "</div></div>";
}

void WebServerManager::begin() {
    setupRoutes();
    server.begin();
}

void WebServerManager::handleClient() {
    server.handleClient();
}

void WebServerManager::setupRoutes() {
    server.on("/", [this]() { this->handleRoot(); });
    server.on("/wifi", [this]() { this->handleWifiConfig(); });
    server.on("/wifi-save", HTTP_POST, [this]() { this->handleWifiSave(); });
    server.on("/message", [this]() { this->handleMessageConfig(); });
    server.on("/message-save", HTTP_POST, [this]() { this->handleMessageSave(); });
    server.on("/message-text-save", HTTP_POST, [this]() { this->handleMessageTextSave(); });
    server.onNotFound([this]() { this->handleNotFound(); });
}

void WebServerManager::handleRoot() {
    String html = getHeader("ESP32 Web Server");
    
    html += "<div class='config-section'>";
    html += "<h2>Current Configuration</h2>";
    html += formatConfigItem("WiFi Mode", config.isAccessPoint() ? "Access Point" : "Station");
    html += formatConfigItem("SSID", config.getWifiSSID());
    html += formatConfigItem("Active Message", String(config.getNumeroMessage()));
    html += formatConfigItem("Message Text", config.getMessageText(config.getNumeroMessage()));
    html += "</div>";
    
    html += getFooter();
    server.send(200, "text/html", html);
}

void WebServerManager::handleWifiConfig() {
    String html = getHeader("WiFi Configuration");
    
    html += "<div class='config-section'>";
    html += "<h2>Current WiFi Settings</h2>";
    html += formatConfigItem("Current Mode", config.isAccessPoint() ? "Access Point" : "Station");
    html += formatConfigItem("Current SSID", config.getWifiSSID());
    html += formatConfigItem("Current Channel", String(config.getWifiChannel()));
    html += formatConfigItem("SSID Hidden", config.isHiddenSSID() ? "Yes" : "No");
    html += "</div>";

    html += "<div class='config-section'>";
    html += "<h2>Update WiFi Settings</h2>";
    html += "<form action='/wifi-save' method='post'>";
    html += "SSID: <input type='text' name='ssid' value='" + String(config.getWifiSSID()) + "'><br><br>";
    html += "Password: <input type='password' name='password'><br><br>";
    html += "Mode: <select name='mode'>";
    html += "<option value='1' " + String(config.isAccessPoint() ? "selected" : "") + ">Access Point</option>";
    html += "<option value='0' " + String(!config.isAccessPoint() ? "selected" : "") + ">Station</option>";
    html += "</select><br><br>";
    html += "Channel (1-13): <input type='number' name='channel' min='1' max='13' value='" + 
            String(config.getWifiChannel()) + "'><br><br>";
    html += "Hide SSID: <input type='checkbox' name='hidden' " + 
            String(config.isHiddenSSID() ? "checked" : "") + "><br><br>";
    html += "<input type='submit' value='Save' class='btn'>";
    html += "</form>";
    html += "</div>";
    
    html += getFooter();
    server.send(200, "text/html", html);
}

void WebServerManager::handleWifiSave() {
    if (server.hasArg("ssid") && server.hasArg("mode")) {
        String ssid = server.arg("ssid");
        String password = server.arg("password");
        bool isAP = server.arg("mode") == "1";
        uint8_t channel = server.hasArg("channel") ? server.arg("channel").toInt() : 6;
        bool hidden = server.hasArg("hidden");
        
        if (channel < 1) channel = 1;
        if (channel > 13) channel = 13;
        
        config.setWifiConfig(ssid.c_str(), 
                           password.length() > 0 ? password.c_str() : config.getWifiPassword(), 
                           isAP);
        config.setWifiAdvanced(channel, hidden);
        
        server.sendHeader("Location", "/");
        server.send(303);
    } else {
        server.send(400, "text/plain", "Missing parameters");
    }
}

void WebServerManager::handleMessageConfig() {
    String html = getHeader("Message Configuration");
    
    html += "<div class='config-section'>";
    html += "<h2>Current Message Settings</h2>";
    html += formatConfigItem("Active Message", String(config.getNumeroMessage()));
    html += formatConfigItem("Active Message Text", config.getMessageText(config.getNumeroMessage()));
    html += "</div>";

    // Add CSS for radio button styling
    html += "<style>"
            ".msg-container { margin: 10px 0; }"
            ".msg-radio { display: none; }"
            ".msg-label { display: block; padding: 10px; background: #f0f0f0; cursor: pointer; }"
            ".msg-radio:checked + .msg-label { background: #b0e0e6; }"
            "</style>";

    // Message selection with radio buttons
    html += "<form action='/message-save' method='post'>";
    for(int i = 1; i <= Config::MAX_MESSAGES; i++) {
        html += "<div class='msg-container'>";
        html += "<input type='radio' class='msg-radio' name='messageNum' value='" + 
                String(i) + "' id='msg" + String(i) + "' " +
                (config.getNumeroMessage() == i ? "checked" : "") + ">";
        html += "<label class='msg-label' for='msg" + String(i) + "'>";
        html += "Message " + String(i) + ": " + String(config.getMessageText(i));
        html += "</label></div>";
    }
    html += "<input type='submit' value='Set Active Message' class='btn'>";
    html += "</form><br><hr><br>";

    // Message text editing section
    html += "<h2>Message Texts</h2>";
    html += "<form action='/message-text-save' method='post'>";
    html += "<select name='number'>";
    for(int i = 1; i <= Config::MAX_MESSAGES; i++) {
        html += "<option value='" + String(i) + "'>" + String(i) + "</option>";
    }
    html += "</select><br>";
    html += "<textarea name='text' rows='2' cols='40'></textarea><br>";
    html += "<input type='submit' value='Save Message' class='btn'>";
    html += "</form>";
    
    html += getFooter();
    server.send(200, "text/html", html);
}

void WebServerManager::handleMessageSave() {
    if (server.hasArg("messageNum")) {
        if (server.hasArg("action")) {
            String action = server.arg("action");
            int messageNum = server.arg("messageNum").toInt();
            
            if (action == "remove" && messageNum >= 1 && messageNum <= Config::MAX_MESSAGES) {
                // Remove message and shift others up
                for (int i = messageNum; i < Config::MAX_MESSAGES; i++) {
                    config.setMessageText(i, config.getMessageText(i + 1));
                }
                config.setMessageText(Config::MAX_MESSAGES, "");
                
            } else if (action == "add") {
                // Find first empty slot and add new message
                for (int i = 1; i <= Config::MAX_MESSAGES; i++) {
                    if (String(config.getMessageText(i)).length() == 0) {
                        config.setMessageText(i, "New Message");
                        break;
                    }
                }
            }
        } else {
            // Set active message
            int messageNum = server.arg("messageNum").toInt();
            if (messageNum >= 1 && messageNum <= Config::MAX_MESSAGES) {
                config.setNumeroMessage(messageNum);
            }
        }
        server.sendHeader("Location", "/message");
        server.send(303);
    } else {
        server.send(400, "text/plain", "Missing parameters");
    }
}

void WebServerManager::handleMessageTextSave() {
    if (server.hasArg("number") && server.hasArg("text")) {
        int number = server.arg("number").toInt();
        String text = server.arg("text");
        
        if (number >= 1 && number <= Config::MAX_MESSAGES) {
            config.setMessageText(number, text.c_str());
            server.sendHeader("Location", "/message");
            server.send(303);
        } else {
            server.send(400, "text/plain", "Invalid message number");
        }
    } else {
        server.send(400, "text/plain", "Missing parameters");
    }
}

void WebServerManager::handleNotFound() {
    server.send(404, "text/plain", "Not found");
}
