#include "web_server_manager.h"

#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "development"
#endif

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
    html += "<a href='/message' class='btn'>Message Config</a> ";
    html += "<a href='/esp32' class='btn'>ESP32 Config</a></nav><br>";
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
    Serial.println("Web server started successfully");
}

void WebServerManager::handleClient() {
    server.handleClient();
}

void WebServerManager::setupRoutes() {
    Serial.println("Setting up web server routes...");
    server.on("/", [this]() { this->handleRoot(); });
    server.on("/wifi", [this]() { this->handleWifiConfig(); });
    server.on("/wifi-save", HTTP_POST, [this]() { this->handleWifiSave(); });
    server.on("/message", [this]() { this->handleMessageConfig(); });
    server.on("/message-save", HTTP_POST, [this]() { this->handleMessageSave(); });
    server.on("/message-text-save", HTTP_POST, [this]() { this->handleMessageTextSave(); });
    server.on("/esp32", [this]() { this->handleEsp32Config(); });
    server.on("/esp32-action", HTTP_POST, [this]() { this->handleEsp32Action(); });
    server.onNotFound([this]() { this->handleNotFound(); });
    Serial.println("Routes configured successfully");
}

void WebServerManager::handleRoot() {
    Serial.println("Handling root page request");
    String html = getHeader("Potail balise sonore");
    
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
    Serial.println("Handling WiFi configuration page request");
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
    Serial.println("Processing WiFi configuration save");
    if (server.hasArg("ssid") && server.hasArg("mode")) {
        String ssid = server.arg("ssid");
        String password = server.arg("password");
        bool isAP = server.arg("mode") == "1";
        uint8_t channel = server.hasArg("channel") ? server.arg("channel").toInt() : 6;
        bool hidden = server.hasArg("hidden");
        
        Serial.printf("New WiFi Settings - SSID: %s, Mode: %s, Channel: %d, Hidden: %s\n",
                     ssid.c_str(),
                     isAP ? "AP" : "Station",
                     channel,
                     hidden ? "Yes" : "No");
        
        if (channel < 1) channel = 1;
        if (channel > 13) channel = 13;
        
        // Update WiFi configuration and save it
        config.setWifiConfig(ssid.c_str(), 
                           password.length() > 0 ? password.c_str() : config.getWifiPassword(), 
                           isAP);
        config.setWifiAdvanced(channel, hidden);
        config.saveConfig();  // Save configuration to flash
        
        Serial.println("WiFi configuration saved to flash memory");
        server.sendHeader("Location", "/");
        server.send(303);
    } else {
        Serial.println("Error: Missing WiFi configuration parameters");
        server.send(400, "text/plain", "Missing parameters");
    }
}

void WebServerManager::handleMessageConfig() {
    Serial.println("Handling message configuration page request");
    String html = getHeader("Message Configuration");
    
    html += "<div class='config-section'>";
    html += "<h2>Current Message Settings</h2>";
    html += formatConfigItem("Active Message", String(config.getNumeroMessage()));
    html += formatConfigItem("Active Message Text", config.getMessageText(config.getNumeroMessage()));
    html += "</div>";

    // Add CSS for radio button styling and button container
    html += "<style>"
            ".msg-container { margin: 10px 0; }"
            ".msg-radio { display: none; }"
            ".msg-label { display: block; padding: 10px; background: #f0f0f0; cursor: pointer; }"
            ".msg-radio:checked + .msg-label { background: #b0e0e6; }"
            ".button-container { margin: 20px 0; display: flex; justify-content: space-between; }"
            ".button-group { display: flex; gap: 10px; }"
            "</style>";

    // Updated JavaScript for message selection handling
    html += "<script>"
            "let selectedMessageNum = " + String(config.getNumeroMessage()) + ";"
            "function updateMessageText(messageNum) {"
            "  selectedMessageNum = messageNum;"
            "  const label = document.querySelector(`label[for='msg${messageNum}']`);"
            "  if (label) {"
            "    const text = label.textContent.split(': ')[1];"
            "    document.getElementById('messageText').value = text;"
            "    document.getElementById('messageNumber').value = messageNum;"
            "  }"
            "}"
            "document.addEventListener('DOMContentLoaded', function() {"
            "  const radios = document.getElementsByName('messageNum');"
            "  for(let radio of radios) {"
            "    radio.addEventListener('change', function() {"
            "      updateMessageText(this.value);"
            "    });"
            "  }"
            "  // Set initial text based on selected radio button"
            "  const selected = document.querySelector('input[name=\"messageNum\"]:checked');"
            "  if(selected) {"
            "    updateMessageText(selected.value);"
            "  }"
            "});</script>";

    // Message selection with radio buttons and action buttons
    html += "<div class='config-section'>";
    html += "<h2>Message Management</h2>";
    html += "<form action='/message-save' method='post'>";
    
    // Message list with radio buttons
    for(int i = 1; i <= config.getMessageCount(); i++) {
        if (String(config.getMessageText(i)).length() > 0) {  // Only show non-empty messages
            html += "<div class='msg-container'>";
            html += "<input type='radio' class='msg-radio' name='messageNum' value='" + 
                    String(i) + "' id='msg" + String(i) + "' " +
                    (config.getNumeroMessage() == i ? "checked" : "") + ">";
            html += "<label class='msg-label' for='msg" + String(i) + "'>";
            html += "Message " + String(i) + ": " + String(config.getMessageText(i));
            html += "</label></div>";
        }
    }

    // Button container with groups
    html += "<div class='button-container'>";
    html += "<div class='button-group'>";
    html += "<button type='submit' name='action' value='setActive' class='btn'>Set Active Message</button>";
    html += "</div>";
    html += "<div class='button-group'>";
    html += "<button type='submit' name='action' value='add' class='btn'>Add New Message</button>";
    html += "<button type='submit' name='action' value='remove' class='btn'>Remove Selected</button>";
    html += "</div>";
    html += "</div>";
    html += "</form>";
    html += "</div>";

    // Updated message text editing section
    html += "<div class='config-section'>";
    html += "<h2>Edit Selected Message</h2>";
    html += "<form action='/message-text-save' method='post'>";
    html += "<input type='hidden' id='messageNumber' name='number' value='" + 
            String(config.getNumeroMessage()) + "'>";
    html += "<textarea id='messageText' name='text' rows='2' cols='40' "
            "placeholder='Select a message to edit'></textarea><br>";
    html += "<input type='submit' value='Save Message' class='btn'>";
    html += "</form>";
    html += "</div>";
    
    html += getFooter();
    server.send(200, "text/html", html);
}

void WebServerManager::handleMessageSave() {
    Serial.println("Processing message configuration save");
    if (server.hasArg("messageNum")) {
        int messageNum = server.arg("messageNum").toInt();
        
        if (server.hasArg("action")) {
            String action = server.arg("action");
            Serial.printf("Message action: %s\n", action.c_str());
            
            if (action == "setActive") {
                // Set active message
                if (messageNum >= 1 && messageNum <= config.getMessageCount()) {
                    Serial.printf("Setting active message to %d\n", messageNum);
                    config.setNumeroMessage(messageNum);
                    config.saveConfig();
                    Serial.println("Active message updated successfully");
                } else {
                    Serial.println("Error: Invalid message number for active message");
                    server.send(400, "text/plain", "Invalid message number");
                    return;
                }
            }
            else if (action == "remove" && messageNum >= 1) {
                // Remove message and shift others up
                for (int i = messageNum; i < config.getMessageCount(); i++) {
                    config.setMessageText(i, config.getMessageText(i + 1));
                }
                config.removeLatestMessage();  // Decrement message count
                config.saveConfig();  // Ensure changes are saved
                
            } else if (action == "add") {
                // Add new message at next available index
                int currentCount = config.getMessageCount();
                int newIndex = currentCount + 1;
                config.setMessageCount(newIndex);
                
                if (config.setMessageText(newIndex, "New Message")) {
                    Serial.printf("Added new message at index %d\n", newIndex);
                    // Message count is automatically updated in setMessageText
                    config.saveConfig();  // Ensure changes are saved
                } else {
                    Serial.println("Failed to add new message");
                    server.send(500, "text/plain", "Failed to add message");
                    return;
                }
            }
        }
        
        server.sendHeader("Location", "/message");
        server.send(303);
    } else {
        Serial.println("Error: Missing message parameters");
        server.send(400, "text/plain", "Missing parameters");
    }
}

void WebServerManager::handleMessageTextSave() {
    Serial.println("Processing message text save");
    if (server.hasArg("number") && server.hasArg("text")) {
        int number = server.arg("number").toInt();
        String text = server.arg("text");
        
        Serial.printf("Saving text for message %d: '%s'\n", number, text.c_str());
        
        if (number >= 1 && number <= config.getMessageCount()) {
            // Update the message text in config for the selected message
            if (config.setMessageText(number, text.c_str())) {
                config.saveConfig();
                Serial.println("Message text saved successfully to flash memory");
                server.sendHeader("Location", "/message");
                server.send(303);
            } else {
                Serial.println("Error: Failed to save message text");
                server.send(500, "text/plain", "Failed to save message");
            }
        } else {
            Serial.println("Error: Invalid message number");
            server.send(400, "text/plain", "Invalid message number");
        }
    } else {
        Serial.println("Error: Missing message text parameters");
        server.send(400, "text/plain", "Missing parameters");
    }
}

void WebServerManager::handleEsp32Config() {
    Serial.println("Handling ESP32 configuration page request");
    String html = getHeader("ESP32 onfig");
    
    html += "<div class='config-section'>";
    html += "<h2>System Information</h2>";
    html += formatConfigItem("Firmware Version", FIRMWARE_VERSION);
    html += formatConfigItem("Compile Date", __DATE__);
    html += formatConfigItem("Compile Time", __TIME__);
    html += "</div>";

    html += "<div class='config-section'>";
    html += "<h2>System Actions</h2>";
    html += "<form action='/esp32-action' method='post' style='display:flex; gap:10px;'>";
    html += "<button type='submit' name='action' value='reset' class='btn' "
            "onclick='return confirm(\"Are you sure you want to reboot the ESP32?\")'>Reboot ESP32</button>";
    html += "<button type='submit' name='action' value='clear' class='btn' "
            "style='background:#e74c3c;' "
            "onclick='return confirm(\"Are you sure you want to clear all configuration?\")'>Clear Configuration</button>";
    html += "</form>";
    html += "</div>";
    
    html += getFooter();
    server.send(200, "text/html", html);
}

void WebServerManager::handleEsp32Action() {
    if (server.hasArg("action")) {
        String action = server.arg("action");
        
        if (action == "reset") {
            Serial.println("Rebooting ESP32...");
            server.send(200, "text/plain", "Rebooting...");
            delay(500);
            ESP.restart();
        }
        else if (action == "clear") {
            Serial.println("Clearing configuration...");
            if (LittleFS.remove("/config.json")) {
                Serial.println("Configuration file removed");
                server.send(200, "text/plain", "Configuration cleared. Rebooting...");
                delay(500);
                ESP.restart();
            } else {
                Serial.println("Failed to remove configuration file");
                server.send(500, "text/plain", "Failed to clear configuration");
            }
        }
    } else {
        server.send(400, "text/plain", "Missing action parameter");
    }
}

void WebServerManager::handleNotFound() {
    Serial.println("404 Not Found: " + server.uri());
    server.send(404, "text/plain", "Not found");
}
