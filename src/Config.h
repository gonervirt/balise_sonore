/**
 * @file Config.h
 * @brief Configuration management for ESP32 Balise Sonore
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

#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <FS.h>
#include <SPIFFS.h>

/**
 * @brief Configuration management class
 * 
 * Handles persistent storage of configuration parameters including
 * WiFi settings and message numbers using ESP32's Preferences
 */
class Config {
public:
    static const int MAX_MESSAGE_LENGTH = 64;

    Config();
    
    /** Initialize the configuration system */
    void begin();
    
    /** Load configuration from non-volatile storage */
    void loadConfig();
    
    /** Save current configuration to non-volatile storage */
    void saveConfig();
    
    /** @return Current message number */
    int getNumeroMessage() const;
    
    /** @param numeroMessage New message number to set */
    void setNumeroMessage(int numeroMessage);

    // WiFi configuration methods
    /** @return Currently configured SSID */
    const char* getWifiSSID() const;
    
    /** @return Currently configured WiFi password */
    const char* getWifiPassword() const;
    
    /** @return true if configured as Access Point, false for Station mode */
    bool isAccessPoint() const;
    
    /**
     * Set WiFi basic configuration
     * @param ssid Network name
     * @param password Network password
     * @param isAP true for Access Point mode, false for Station mode
     */
    void setWifiConfig(const char* ssid, const char* password, bool isAP);
    
    /** @return WiFi channel (1-13) */
    uint8_t getWifiChannel() const;
    
    /** @return true if SSID should be hidden */
    bool isHiddenSSID() const;
    
    /**
     * Set WiFi advanced configuration
     * @param channel WiFi channel (1-13)
     * @param hidden true to hide SSID
     */
    void setWifiAdvanced(uint8_t channel, bool hidden);

    /** Get message description by number
     * @param number Message number (1-message_count)
     * @return Message description or default text if not set
     */
    const char* getMessageText(int number) const;
    
    /** Set message description
     * @param number Message number (1-message_count)
     * @param text New message description
     * @return true if successful
     */
    bool setMessageText(int number, const char* text);
    
    /** Get number of defined messages */
    int getMessageCount() const;

    /** Set number of defined messages */
    void setMessageCount(int number) ;

    /** Remove the latest message and decrement message count */
    bool removeLatestMessage();

    uint8_t getVolume() const;
    void setVolume(uint8_t vol);

private:
    int numeroMessage;
    char wifi_ssid[32];
    char wifi_password[64];
    bool access_point;
    uint8_t wifi_channel;
    bool hidden_ssid;
    static const int ABSOLUTE_MAX_MESSAGES = 32;  // Safety limit
    char messages[ABSOLUTE_MAX_MESSAGES][MAX_MESSAGE_LENGTH];
    bool message_defined[ABSOLUTE_MAX_MESSAGES];
    int message_count;
    uint8_t volume;  // Add volume parameter
    
    const char* getDefaultMessage(int number) const;
    static const char* CONFIG_FILE;  // Add this line
    bool saveJsonToFile(const JsonDocument& doc);
    bool loadJsonFromFile(JsonDocument& doc);
};

#endif // CONFIG_H
