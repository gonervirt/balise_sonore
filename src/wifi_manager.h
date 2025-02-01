#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include "Config.h"

/**
 * @brief WiFi connection manager
 * 
 * Handles WiFi configuration and connection in both
 * Access Point and Station modes
 */
class WiFiManager {
private:
    const char* ssid;      // Network name
    const char* password;  // Network password
    bool isAP;            // Mode flag (true = AP, false = Station)
    uint8_t channel;      // WiFi channel (1-13)
    bool hidden_ssid;     // SSID visibility flag

public:
    /**
     * Create WiFi manager in Access Point mode
     * @param ap_ssid Network name
     * @param ap_password Network password
     */
    WiFiManager(const char* ap_ssid, const char* ap_password);
    
    /**
     * Create WiFi manager in Station mode
     * @param sta_ssid Network to connect to
     * @param sta_password Network password
     * @param station Must be true (used to differentiate from AP constructor)
     */
    WiFiManager(const char* sta_ssid, const char* sta_password, bool station);

    /**
     * Create WiFi manager from configuration
     * @param config Configuration object containing WiFi settings
     */
    WiFiManager(Config &config);
    
    /**
     * Initialize WiFi connection
     * @return true if connection successful
     */
    bool begin();
    
    /**
     * Get current IP address
     * @return IP address as string
     */
    String getIP();
};

#endif
