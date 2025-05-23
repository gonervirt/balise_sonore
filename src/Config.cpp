#include "Config.h"
#include "Arduino.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

const char* Config::CONFIG_FILE = "/config.json";

// Constructeur de la classe Config
Config::Config(int defaultVolume) : _defaultVolume(defaultVolume) {
    numeroMessage = 1;
    strcpy(wifi_ssid, "BALISESONORE");
    strcpy(wifi_password, "BaliseSonore_Betton_Mairie");
    access_point = true;
    wifi_channel = 6;
    hidden_ssid = false;
    message_count = 3;  // Initial number of messages
    for(int i = 0; i < message_count; i++) {
        message_defined[i] = false;
        strcpy(messages[i], getDefaultMessage(i + 1));
    }
    volume = _defaultVolume;  // Default volume
}

// Initialisation de la configuration
void Config::begin() {
    Serial.println("Initializing configuration system...");
    
    if (!LittleFS.begin(false)) {
        Serial.println("Failed to mount LittleFS, attempting to format...");
        if (!LittleFS.begin(true)) {
            Serial.println("Fatal: Failed to mount and format LittleFS");
            return;
        }
        Serial.println("LittleFS formatted successfully");
    } else {
        Serial.println("LittleFS mounted successfully");
    }
    
    // Create config file if it doesn't exist
    if (!LittleFS.exists(CONFIG_FILE)) {
        Serial.println("No configuration file found, creating default...");
        saveConfig();  // Save default values from constructor
    } else {
        Serial.println("Loading existing configuration...");
        loadConfig();
    }
}

// Chargement de la configuration depuis la mémoire non volatile
void Config::loadConfig() {
    JsonDocument doc;
    
    if (!loadJsonFromFile(doc)) {
        Serial.println("Using default configuration");
        return;
    }

    // Load WiFi settings
    strlcpy(wifi_ssid, doc["wifi_ssid"] | "ESP32-AP", sizeof(wifi_ssid));
    strlcpy(wifi_password, doc["wifi_password"] | "password123", sizeof(wifi_password));
    access_point = doc["access_point"] | true;
    wifi_channel = doc["wifi_channel"] | 6;
    hidden_ssid = doc["hidden_ssid"] | false;

    // Load message settings
    numeroMessage = doc["numeroMessage"] | 1;
    message_count = doc["message_count"] | 1;

    // Load messages
    JsonArray messages_array = doc["messages"].as<JsonArray>();
    int i = 0;
    for (JsonVariant v : messages_array) {
        if (i >= message_count) break;
        const char* msg = v["text"] | getDefaultMessage(i + 1);
        strlcpy(messages[i], msg, MAX_MESSAGE_LENGTH);
        message_defined[i] = strlen(messages[i]) > 0;
        i++;
    }

    volume = doc["volume"] | _defaultVolume;  // Load volume setting
}

// Sauvegarde de la configuration dans la mémoire non volatile
void Config::saveConfig() {
    JsonDocument doc;

    // Save WiFi settings
    doc["wifi_ssid"] = wifi_ssid;
    doc["wifi_password"] = wifi_password;
    doc["access_point"] = access_point;
    doc["wifi_channel"] = wifi_channel;
    doc["hidden_ssid"] = hidden_ssid;

    // Save message settings
    doc["numeroMessage"] = numeroMessage;
    doc["message_count"] = message_count;

    // Save messages using newer API
    JsonArray messages_array = doc["messages"].to<JsonArray>();
    for (int i = 0; i < message_count; i++) {
        if (message_defined[i]) {
            JsonObject msg = messages_array.add<JsonObject>();
            msg["id"] = i + 1;
            msg["text"] = messages[i];
        }
    }

    doc["volume"] = volume;

    if (!saveJsonToFile(doc)) {
        Serial.println("Failed to save configuration");
    }
}

bool Config::loadJsonFromFile(JsonDocument& doc) {
    if (!LittleFS.exists(CONFIG_FILE)) {
        Serial.println("Configuration file not found");
        return false;
    }

    File file = LittleFS.open(CONFIG_FILE, "r");
    if (!file) {
        Serial.println("Failed to open config file");
        return false;
    }

    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        Serial.print("Failed to parse config file: ");
        Serial.println(error.c_str());
        return false;
    }

    return true;
}

bool Config::saveJsonToFile(const JsonDocument& doc) {
    File file = LittleFS.open(CONFIG_FILE, "w");
    if (!file) {
        Serial.println("Failed to open config file for writing");
        return false;
    }

    if (serializeJson(doc, file) == 0) {
        Serial.println("Failed to write config file");
        file.close();
        return false;
    }

    file.close();
    return true;
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
    if (number < 1 || number > message_count) return "";
    return messages[number - 1];
}

bool Config::setMessageText(int number, const char* text) {
    if (number < 1 || number > message_count) return false;
    strncpy(messages[number - 1], text, MAX_MESSAGE_LENGTH - 1);
    message_defined[number - 1] = true;
    if (number > message_count) message_count = number;
    saveConfig();
    return true;
}

int Config::getMessageCount() const {
    return message_count;
}

void Config::setMessageCount(int number)  {
    message_count = number;
}

const char* Config::getDefaultMessage(int number) const {
    static char default_msg[32];
    snprintf(default_msg, sizeof(default_msg), "Message %d - Default Text", number);
    return default_msg;
}

bool Config::removeLatestMessage() {
    if (message_count <= 1) {
        Serial.println("Cannot remove last message, minimum one message required");
        return false;
    }

    // Clear the last message
    messages[message_count - 1][0] = '\0';
    message_defined[message_count - 1] = false;
    
    // Decrement message count
    message_count--;
    
    // If active message was the removed one, update it
    if (numeroMessage > message_count) {
        numeroMessage = message_count;
    }
    
    Serial.printf("Removed message %d, new message count: %d\n", message_count + 1, message_count);
    saveConfig();
    return true;
}

uint8_t Config::getVolume() const {
    return volume;
}

void Config::setVolume(uint8_t vol) {
    volume = constrain(vol, 0, 30);
    saveConfig();
}
