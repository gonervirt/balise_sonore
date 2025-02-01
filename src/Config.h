#ifndef CONFIG_H
#define CONFIG_H

#include <FS.h>
#include <SPIFFS.h>

class Config {
public:
    Config();
    void begin();
    void loadConfig();
    void saveConfig();
    int getNumeroMessage() const;
    void setNumeroMessage(int numeroMessage);

    // New WiFi configuration getters
    const char* getWifiSSID() const;
    const char* getWifiPassword() const;
    bool isAccessPoint() const;
    void setWifiConfig(const char* ssid, const char* password, bool isAP);
    uint8_t getWifiChannel() const;
    bool isHiddenSSID() const;
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
