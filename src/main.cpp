#include <Arduino.h>
#include "wifi_manager.h"
#include "web_server_manager.h"

// Create as Access Point
WiFiManager wifiManager("ESP32-AP", "password123");
// Or create as Station
// WiFiManager wifiManager("YourSSID", "YourPassword", true);

WebServerManager webServer;

void setup() {
    Serial.begin(115200);
    
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
    sleep(2);
}
