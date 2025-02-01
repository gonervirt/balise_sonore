#include "wifi_manager.h"

WiFiManager::WiFiManager(const char* ap_ssid, const char* ap_password) {
    this->ssid = ap_ssid;
    this->password = ap_password;
    this->isAP = true;
}

WiFiManager::WiFiManager(const char* sta_ssid, const char* sta_password, bool station) {
    this->ssid = sta_ssid;
    this->password = sta_password;
    this->isAP = false;
}

bool WiFiManager::begin() {
    if (isAP) {
        WiFi.mode(WIFI_AP);
        return WiFi.softAP(ssid, password,6);
    } else {
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);
        
        // Wait for connection with timeout
        int timeout = 20;
        while (WiFi.status() != WL_CONNECTED && timeout > 0) {
            delay(500);
            timeout--;
        }
        return WiFi.status() == WL_CONNECTED;
    }
}

String WiFiManager::getIP() {
    if (isAP) {
        return WiFi.softAPIP().toString();
    }
    return WiFi.localIP().toString();
}
