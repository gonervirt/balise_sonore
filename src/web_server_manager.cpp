#include "web_server_manager.h"

WebServerManager::WebServerManager(Config& config, int port) 
    : server(port), config(config) {
}

String WebServerManager::getHeader(const char* title) {
    String html = "<!DOCTYPE html><html><head>";
    html += "<title>" + String(title) + "</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>body{font-family:Arial;margin:20px;} .btn{background:#1abc9c;color:white;padding:10px 20px;text-decoration:none;border-radius:5px;display:inline-block;margin:5px;}</style>";
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
    server.onNotFound([this]() { this->handleNotFound(); });
}

void WebServerManager::handleRoot() {
    String html = getHeader("ESP32 Web Server");
    html += "<p>Current configuration:</p>";
    html += "<p>WiFi Mode: " + String(config.isAccessPoint() ? "Access Point" : "Station") + "</p>";
    html += "<p>SSID: " + String(config.getWifiSSID()) + "</p>";
    html += "<p>Message Number: " + String(config.getNumeroMessage()) + "</p>";
    html += getFooter();
    server.send(200, "text/html", html);
}

void WebServerManager::handleWifiConfig() {
    String html = getHeader("WiFi Configuration");
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
    html += "<form action='/message-save' method='post'>";
    html += "Message Number (1-9): <input type='number' name='messageNum' min='1' max='9' value='" + 
            String(config.getNumeroMessage()) + "'><br><br>";
    html += "<input type='submit' value='Save' class='btn'>";
    html += "</form>";
    html += getFooter();
    server.send(200, "text/html", html);
}

void WebServerManager::handleMessageSave() {
    if (server.hasArg("messageNum")) {
        int messageNum = server.arg("messageNum").toInt();
        if (messageNum >= 1 && messageNum <= 9) {
            config.setNumeroMessage(messageNum);
            server.sendHeader("Location", "/");
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
