#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>

class WiFiManager {
private:
    const char* ssid;
    const char* password;
    bool isAP;

public:
    // Constructor for AP mode
    WiFiManager(const char* ap_ssid, const char* ap_password);
    
    // Constructor for STA mode
    WiFiManager(const char* sta_ssid, const char* sta_password, bool station);
    
    // Initialize WiFi connection
    bool begin();
    
    // Get IP address as string
    String getIP();
};

#endif
