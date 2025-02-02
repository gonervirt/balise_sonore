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

// Wait for Serial with timeout
void waitForSerial(unsigned long timeout_ms = 10000) {
    unsigned long start = millis();
    while (!Serial && (millis() - start) < timeout_ms) {
        delay(100);
    }
}

void setup() {
    Serial.begin(115200);
    waitForSerial();  // Wait up to 10 seconds for Serial
    
    Serial.println("\n\nStarting ESP32 Balise Sonore...");
    Serial.printf("Compile time: %s %s\n", __DATE__, __TIME__);
    
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
