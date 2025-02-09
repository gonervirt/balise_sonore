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
        Serial.println("Starting Access Point mode...");
        WiFi.mode(WIFI_AP);
        bool success = WiFi.softAP(ssid, password, channel, hidden_ssid);
        if (success) {
            Serial.println("Access Point started successfully");
            Serial.printf("SSID: %s\n", ssid);
            Serial.printf("IP Address: %s\n", WiFi.softAPIP().toString().c_str());
        } else {
            Serial.println("Failed to start Access Point");
        }
        return success;
    } else {
        Serial.println("Starting Station mode...");
        Serial.printf("Connecting to SSID: %s\n", ssid);
        
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);
        
        int timeout = 20;
        while (WiFi.status() != WL_CONNECTED && timeout > 0) {
            Serial.print(".");
            delay(500);
            timeout--;
        }
        Serial.println();
        
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("Successfully connected to WiFi");
            Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
            return true;
        }
        Serial.println("Failed to connect to WiFi");
        return false;
    }
}

String WiFiManager::getIP() {
    if (isAP) {
        return WiFi.softAPIP().toString();
    }
    return WiFi.localIP().toString();
}

WiFiManager::WifiStatus WiFiManager::checkStatus() {
    WifiStatus status;
    if (isAP) {
        status.ssid = String(ssid);
        status.ip = WiFi.softAPIP().toString();
        status.isConnected = WiFi.softAPgetStationNum() > 0;
        status.rssi = 0; // Not applicable for AP mode
    } else {
        status.ssid = String(ssid);
        status.ip = WiFi.localIP().toString();
        status.isConnected = WiFi.status() == WL_CONNECTED;
        status.rssi = WiFi.RSSI();
    }
    return status;
}
