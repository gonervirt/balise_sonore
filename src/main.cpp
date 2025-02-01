/**
 * ESP32 Balise Sonore
 * 
 * Main program file that initializes and manages:
 * - Configuration system
 * - WiFi connection
 * - Web interface
 * 
 * The device can operate in either Access Point or Station mode
 * and provides a web interface for configuration.
 */

#include <Arduino.h>
#include "wifi_manager.h"
#include "web_server_manager.h"
#include "Config.h"

// Initialize management objects
Config config;
WiFiManager wifiManager(config);
WebServerManager webServer(config);

void setup() {
    Serial.begin(115200);
    Serial.println("Starting ESP32 Balise Sonore...");
    
    // Initialize configuration
    config.begin();
    
    // Initialize WiFi
    if (wifiManager.begin()) {
        Serial.println("WiFi ready");
        Serial.println("IP: " + wifiManager.getIP());
        webServer.begin();
    } else {
        Serial.println("WiFi failed!");
    }
}

void loop() {
    webServer.handleClient();
    sleep(2);  // Prevent watchdog reset
}
