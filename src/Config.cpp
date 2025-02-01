#include "Config.h"
#include "Arduino.h"
#include <Preferences.h>

Preferences preferences;

// Constructeur de la classe Config
Config::Config() {
    numeroMessage = 1;
    strcpy(wifi_ssid, "ESP32-AP");
    strcpy(wifi_password, "password123");
    access_point = true;
    wifi_channel = 6;
    hidden_ssid = false;
    message_count = 1;
    for(int i = 0; i < MAX_MESSAGES; i++) {
        message_defined[i] = false;
        strcpy(messages[i], getDefaultMessage(i + 1));
    }
}

// Initialisation de la configuration
void Config::begin() {
    preferences.begin("config", false);
    loadConfig();
}

// Chargement de la configuration depuis la mémoire non volatile
void Config::loadConfig() {
    numeroMessage = preferences.getInt("numeroMessage", 1);
    
    // Load WiFi configuration
    String ssid = preferences.getString("wifi_ssid", "ESP32-AP");
    String password = preferences.getString("wifi_password", "password123");
    access_point = preferences.getBool("access_point", true);
    wifi_channel = preferences.getUChar("wifi_channel", 6);
    hidden_ssid = preferences.getBool("hidden_ssid", false);
    
    strcpy(wifi_ssid, ssid.c_str());
    strcpy(wifi_password, password.c_str());

    message_count = preferences.getInt("message_count", 1);
    for(int i = 0; i < MAX_MESSAGES; i++) {
        char key[16];
        snprintf(key, sizeof(key), "message_%d", i + 1);
        String msg = preferences.getString(key, String(getDefaultMessage(i + 1)));
        strncpy(messages[i], msg.c_str(), MAX_MESSAGE_LENGTH - 1);
        messages[i][MAX_MESSAGE_LENGTH - 1] = '\0';
        message_defined[i] = msg.length() > 0;
    }
}

// Sauvegarde de la configuration dans la mémoire non volatile
void Config::saveConfig() {
    preferences.putInt("numeroMessage", numeroMessage);
    
    // Save WiFi configuration
    preferences.putString("wifi_ssid", wifi_ssid);
    preferences.putString("wifi_password", wifi_password);
    preferences.putBool("access_point", access_point);
    preferences.putUChar("wifi_channel", wifi_channel);
    preferences.putBool("hidden_ssid", hidden_ssid);

    preferences.putInt("message_count", message_count);
    for(int i = 0; i < MAX_MESSAGES; i++) {
        if (message_defined[i]) {
            char key[16];
            snprintf(key, sizeof(key), "message_%d", i + 1);
            preferences.putString(key, messages[i]);
        }
    }
}

// Retourne le numéro du message
int Config::getNumeroMessage() const {
    return numeroMessage;
}

// Définit le numéro du message
void Config::setNumeroMessage(int numeroMessage) {
    this->numeroMessage = numeroMessage;
}

const char* Config::getWifiSSID() const {
    return wifi_ssid;
}

const char* Config::getWifiPassword() const {
    return wifi_password;
}

bool Config::isAccessPoint() const {
    return access_point;
}

void Config::setWifiConfig(const char* ssid, const char* password, bool isAP) {
    strncpy(wifi_ssid, ssid, sizeof(wifi_ssid) - 1);
    strncpy(wifi_password, password, sizeof(wifi_password) - 1);
    access_point = isAP;
    saveConfig();
}

uint8_t Config::getWifiChannel() const {
    return wifi_channel;
}

bool Config::isHiddenSSID() const {
    return hidden_ssid;
}

void Config::setWifiAdvanced(uint8_t channel, bool hidden) {
    wifi_channel = channel;
    hidden_ssid = hidden;
    saveConfig();
}

const char* Config::getMessageText(int number) const {
    if (number < 1 || number > MAX_MESSAGES) return "";
    return messages[number - 1];
}

bool Config::setMessageText(int number, const char* text) {
    if (number < 1 || number > MAX_MESSAGES) return false;
    strncpy(messages[number - 1], text, MAX_MESSAGE_LENGTH - 1);
    message_defined[number - 1] = true;
    if (number > message_count) message_count = number;
    saveConfig();
    return true;
}

int Config::getMessageCount() const {
    return message_count;
}

const char* Config::getDefaultMessage(int number) const {
    static char default_msg[32];
    snprintf(default_msg, sizeof(default_msg), "Message %d - Default Text", number);
    return default_msg;
}
