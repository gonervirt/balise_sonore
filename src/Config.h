#ifndef CONFIG_H
#define CONFIG_H

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

private:
    int numeroMessage;
    char wifi_ssid[32];
    char wifi_password[64];
    bool access_point;
    uint8_t wifi_channel;
    bool hidden_ssid;
};

#endif // CONFIG_H
