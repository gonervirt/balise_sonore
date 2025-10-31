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

#include <Arduino.h>
#include <WiFi.h>
#include "Config.h"

class WiFiManager {
public:
    struct WifiStatus {
        String ssid;
        String ip;
        bool isConnected;
        int rssi;
    };

    WiFiManager(const char* ap_ssid, const char* ap_password);
    WiFiManager(const char* sta_ssid, const char* sta_password, bool station);
    WiFiManager(Config &config);

    bool begin();
    String getIP();
    WifiStatus checkStatus();
    void checkAndRestartAP();
    void logAPStatus();
    void loop();

    static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

private:
    // store copies to avoid lifetime issues
    String ssid;
    String password;
    bool isAP = false;
    int channel = 6;
    bool hidden_ssid = false;
    unsigned long lastCheckTime = 0;
    unsigned long lastLogTime = 0;
};

#endif // WIFI_MANAGER_H
