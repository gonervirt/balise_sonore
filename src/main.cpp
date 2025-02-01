#include <Arduino.h>
#include "wifi_manager.h"
#include "web_server_manager.h"
#include "Config.h"


// Initialisation des objets de gestion
Config config;
// Create as Access Point
WiFiManager wifiManager(config);
// Or create as Station
// WiFiManager wifiManager("YourSSID", "YourPassword", true);

WebServerManager webServer(config);

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
