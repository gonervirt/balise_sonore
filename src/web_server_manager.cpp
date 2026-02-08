/**
 * @file web_server_manager.cpp
 * @brief Implementation of the WebServerManager class
 * 
 * @copyright Copyright (c) 2024 ESP32 Balise Sonore Project
 * 
 * @license MIT License
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include "web_server_manager.h"
#include <WiFi.h>

#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "development"
#endif

WebServerManager::WebServerManager(Config& config, int port) 
    : server(port), config(config) {
}

/**
 * @brief Generates the common HTML header with navigation menu
 */
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
    html += "<nav><a href='/' class='btn'>Accueil</a> ";
    html += "<a href='/wifi' class='btn'>Config WiFi</a> ";
    html += "<a href='/message' class='btn'>Config Messages</a> ";
    html += "<a href='/esp32' class='btn'>Config ESP32</a></nav><br>";
    return html;
}

/**
 * @brief Generates the common HTML footer
 */
String WebServerManager::getFooter() {
    return "</body></html>";
}

/**
 * @brief Helper to format a configuration item in HTML
 */
String WebServerManager::formatConfigItem(const char* label, const String& value) {
    return "<div class='config-item'><div class='config-label'>" + String(label) + 
           "</div><div class='config-value'>" + value + "</div></div>";
}

void WebServerManager::begin() {
    server.begin();
    setupRoutes();
    Serial.println("Web server started successfully");
}

/**
 * @brief Handles incoming client requests
 * @return true if a web page was served/handled
 */
bool WebServerManager::handleClient() {
    _webPageHandled = false;
    server.handleClient();
    // is true if a web page was handled
    return _webPageHandled;
}

/**
 * @brief Sets up the HTTP routes and their callbacks
 */
void WebServerManager::setupRoutes() {
    Serial.println("Setting up web server routes...");
    server.on("/", [this]() { this->handleRoot(); });
    server.on("/wifi", [this]() { this->handleWifiConfig(); });
    server.on("/wifi-save", HTTP_POST, [this]() { this->handleWifiSave(); });
    server.on("/message", [this]() { this->handleMessageConfig(); });
    server.on("/message-save", HTTP_POST, [this]() { this->handleMessageSave(); });
    server.on("/message-text-save", HTTP_POST, [this]() { this->handleMessageTextSave(); });
    server.on("/volume-save", HTTP_POST, [this]() { this->handleVolumeSave(); });  // Add new route
    server.on("/esp32", [this]() { this->handleEsp32Config(); });
    server.on("/esp32-action", HTTP_POST, [this]() { this->handleEsp32Action(); });
    server.onNotFound([this]() { this->handleNotFound(); });
    Serial.println("Routes configured successfully");
}

/**
 * @brief Handler for the root/home page
 */
void WebServerManager::handleRoot() {
    Serial.println("Handling root page request");
    _webPageHandled = true;

    String html = getHeader("Portail balise sonore");
    
    html += "<div class='config-section'>";
    html += "<h2>Configuration Actuelle</h2>";
    html += formatConfigItem("Mode WiFi", config.isAccessPoint() ? "Point d'Accès" : "Station");
    html += formatConfigItem("SSID", config.getWifiSSID());
    html += formatConfigItem("Message Actif", String(config.getNumeroMessage()));
    html += formatConfigItem("Texte du Message", config.getMessageText(config.getNumeroMessage()));
    html += "</div>";
    
    html += getFooter();
    server.send(200, "text/html", html);
}

/**
 * @brief Handler for the WiFi configuration page
 */
void WebServerManager::handleWifiConfig() {
    Serial.println("Handling WiFi configuration page request");
    _webPageHandled = true;

    String html = getHeader("Configuration WiFi");
    
    html += "<div class='config-section'>";
    html += "<h2>Paramètres WiFi Actuels</h2>";
    html += formatConfigItem("SSID Actuel", config.getWifiSSID());
    html += formatConfigItem("Canal Actuel", String(config.getWifiChannel()));
    html += formatConfigItem("SSID Masqué", config.isHiddenSSID() ? "Oui" : "Non");
    html += "</div>";

    html += "<div class='config-section'>";
    html += "<h2>Mettre à jour les paramètres WiFi</h2>";
    html += "<form action='/wifi-save' method='post'>";
    html += "SSID : <input type='text' name='ssid' value='" + String(config.getWifiSSID()) + "'><br><br>";
    html += "Mot de passe : <input type='text' name='password' value='" + String(config.getWifiPassword()) + "'><br><br>";
    html += "Canal (1-13) : <input type='number' name='channel' min='1' max='13' value='" + 
            String(config.getWifiChannel()) + "'><br><br>";
    html += "Masquer le SSID : <input type='checkbox' name='hidden' " + 
            String(config.isHiddenSSID() ? "checked" : "") + "><br><br>";
    html += "<input type='submit' value='Enregistrer' class='btn'>";
    html += "</form>";
    html += "</div>";
    
    html += getFooter();
    server.send(200, "text/html", html);
}

/**
 * @brief Handler for saving WiFi configuration
 */
void WebServerManager::handleWifiSave() {
    Serial.println("Processing WiFi configuration save");
    _webPageHandled = true;

    if (server.hasArg("ssid")) {
        String ssid = server.arg("ssid");
        String password = server.arg("password");
        bool isAP = true;
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

/**
 * @brief Handler for the Message configuration page
 */
void WebServerManager::handleMessageConfig() {
    Serial.println("Handling message configuration page request");
    _webPageHandled = true;

    String html = getHeader("Configuration des Messages");
    
    html += "<div class='config-section'>";
    html += "<h2>Paramètres du message actuel</h2>";
    html += formatConfigItem("Message actif", String(config.getNumeroMessage()));
    html += formatConfigItem("Texte du Message actif", config.getMessageText(config.getNumeroMessage()));
    html += "</div>";

    // Add CSS for radio button styling and button container
    html += "<style>"
            ".msg-row { display: flex; align-items: center; margin: 5px 0; padding: 5px; background: #f9f9f9; border-radius: 4px; }"
            ".msg-radio { margin-right: 10px; }"
            ".msg-index { font-weight: bold; margin-right: 10px; min-width: 20px; }"
            ".msg-input { flex-grow: 1; padding: 8px; border: 1px solid #ccc; border-radius: 4px; }"
            ".msg-input.changed { background-color: #fff3cd; border-color: #ffc107; }"
            ".button-container { margin: 20px 0; display: flex; flex-wrap: wrap; gap: 10px; }"
            "</style>";

    // Updated JavaScript for message selection handling
    html += "<script>"
            "function markChanged(input) {"
            "  input.classList.add('changed');"
            "}"
            "</script>";

    // Message selection with radio buttons and action buttons
    html += "<div class='config-section'>";
    html += "<h2>Gestion des Messages</h2>";
    html += "<form action='/message-save' method='post'>";
    
    // Message list with radio buttons
    for(int i = 1; i <= config.getMessageCount(); i++) {
        html += "<div class='msg-row'>";
        html += "<input type='radio' class='msg-radio' name='messageNum' value='" + String(i) + "' " + 
                (config.getNumeroMessage() == i ? "checked" : "") + ">";
        html += "<span class='msg-index'>" + String(i) + "</span>";
        html += "<input type='text' class='msg-input' name='text_" + String(i) + "' " +
                "value='" + String(config.getMessageText(i)) + "' " +
                "oninput='markChanged(this)'>";
        html += "</div>";
    }

    // Button container with groups
    html += "<div class='button-container'>";
    html += "<button type='submit' name='action' value='setActive' class='btn'>Définir comme actif</button>";
    html += "<button type='submit' name='action' value='saveTexts' class='btn'>Sauvegarder la description des messages</button>";
    html += "<button type='submit' name='action' value='add' class='btn'>Ajouter un message</button>";
    html += "<button type='submit' name='action' value='remove' class='btn' onclick='return confirm(\"Supprimer ?\")'>Supprimer la sélection</button>";
    html += "</div>";
    html += "</form>";
    html += "</div>";

    // Add volume control section before the footer
    html += "<div class='config-section'>";
    html += "<h2>Contrôle du Volume</h2>";
    html += formatConfigItem("Volume Actuel", String(config.getVolume()));
    html += "<form action='/volume-save' method='post'>";
    html += "<label for='volume'>Volume (0-30) : </label>";
    html += "<input type='number' id='volume' name='volume' min='0' max='30' value='" + 
            String(config.getVolume()) + "'>";
    html += "<input type='submit' value='Enregistrer le Volume' class='btn'>";
    html += "</form>";
    html += "</div>";
    
    html += getFooter();
    server.send(200, "text/html", html);
}

/**
 * @brief Handler for saving general message settings (active message, add/remove)
 */
void WebServerManager::handleMessageSave() {
    Serial.println("Processing message configuration save");
    _webPageHandled = true;

    if (server.hasArg("action")) {
        String action = server.arg("action");
        int messageNum = server.hasArg("messageNum") ? server.arg("messageNum").toInt() : -1;
        Serial.printf("Message action: %s\n", action.c_str());
            
        if (action == "setActive" && messageNum != -1) {
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
        } else if (action == "saveTexts") {
            int count = config.getMessageCount();
            for (int i = 1; i <= count; i++) {
                String argName = "text_" + String(i);
                if (server.hasArg(argName)) {
                    String newText = server.arg(argName);
                    String currentText = String(config.getMessageText(i));
                    if (newText != currentText) {
                        config.setMessageText(i, newText.c_str());
                    }
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

/**
 * @brief Handler for saving the text content of a specific message
 */
void WebServerManager::handleMessageTextSave() {
    Serial.println("Processing message text save");
    _webPageHandled = true;

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

/**
 * @brief Handler for saving volume settings
 */
void WebServerManager::handleVolumeSave() {
    _webPageHandled = true;
    if (server.hasArg("volume")) {
        int volume = server.arg("volume").toInt();
        volume = constrain(volume, 0, 30);  // Ensure volume is within valid range
        
        Serial.printf("Setting volume to: %d\n", volume);
        config.setVolume(volume);
        
        server.sendHeader("Location", "/message");
        server.send(303);
    } else {
        server.send(400, "text/plain", "Missing volume parameter");
    }
}

/**
 * @brief Handler for the ESP32 system configuration page
 */
void WebServerManager::handleEsp32Config() {
    Serial.println("Handling ESP32 configuration page request");
    _webPageHandled = true;

    String html = getHeader("Configuration ESP32");
    
    html += "<div class='config-section'>";
    html += "<h2>Informations Système</h2>";
    html += formatConfigItem("Version du Firmware", FIRMWARE_VERSION);
    html += formatConfigItem("Date de Compilation", __DATE__);
    html += formatConfigItem("Heure de Compilation", __TIME__);
    html += "</div>";

    html += "<div class='config-section'>";
    html += "<h2>Actions Système</h2>";
    html += "<form action='/esp32-action' method='post' style='display:flex; gap:10px;'>";
    html += "<button type='submit' name='action' value='reset' class='btn' "
            "onclick='return confirm(\"Êtes-vous sûr de vouloir redémarrer l'ESP32 ?\")'>Redémarrer ESP32</button>";
    html += "<button type='submit' name='action' value='clear' class='btn' "
            "style='background:#e74c3c;' "
            "onclick='return confirm(\"Êtes-vous sûr de vouloir effacer toute la configuration ?\")'>Effacer la Configuration</button>";
    html += "</form>";
    html += "</div>";
    
    html += getFooter();
    server.send(200, "text/html", html);
}

/**
 * @brief Handler for ESP32 system actions (reboot, clear config)
 */
void WebServerManager::handleEsp32Action() {
    _webPageHandled = true;

    if (server.hasArg("action")) {
        String action = server.arg("action");
        
        if (action == "reset") {
            Serial.println("Rebooting ESP32...");
            server.send(200, "text/plain", "Redémarrage en cours...");
            delay(500);
            ESP.restart();
        }
        else if (action == "clear") {
            Serial.println("Clearing configuration...");
            if (LittleFS.remove("/config.json")) {
                Serial.println("Configuration file removed");
                server.send(200, "text/plain", "Configuration effacée. Redémarrage...");
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

/**
 * @brief Handler for 404 Not Found errors
 */
void WebServerManager::handleNotFound() {
    _webPageHandled = true;
    
    if (config.isAccessPoint()) {
        // Redirect to captive portal root
        server.sendHeader("Location", String("http://") + WiFi.softAPIP().toString() + String("/"), true);
        server.send(302, "text/plain", "");
    } else {
        Serial.println("404 Not Found: " + server.uri());
        server.send(404, "text/plain", "Not found");
    }
}

WebServerManager::~WebServerManager() {
    server.stop();
    Serial.println("Web server stopped successfully");
}
