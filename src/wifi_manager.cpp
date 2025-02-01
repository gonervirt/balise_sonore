#include "wifi_manager.h"

WiFiManager::WiFiManager(const char* ap_ssid, const char* ap_password) {
    this->ssid = ap_ssid;
    this->password = ap_password;
    this->isAP = true;
    this->channel = 6;
    this->hidden_ssid = false;
}

WiFiManager::WiFiManager(const char* sta_ssid, const char* sta_password, bool station) {
    this->ssid = sta_ssid;
    this->password = sta_password;
    this->isAP = false;
    this->channel = 6;
    this->hidden_ssid = false;
}

WiFiManager::WiFiManager(Config &config) {
    this->ssid = config.getWifiSSID();
    this->password = config.getWifiPassword();
    this->isAP = config.isAccessPoint();
    this->channel = config.getWifiChannel();
    this->hidden_ssid = config.isHiddenSSID();
}

bool WiFiManager::begin() {
    if (isAP) {
        WiFi.mode(WIFI_AP);
        return WiFi.softAP(ssid, password, channel, hidden_ssid);
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
