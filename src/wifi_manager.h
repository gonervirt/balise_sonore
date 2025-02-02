/**
 * @file wifi_manager.h
 * @brief WiFi connection management for ESP32 Balise Sonore
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
