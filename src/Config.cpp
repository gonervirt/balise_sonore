#include "Config.h"
#include "Arduino.h"
#include <ArduinoJson.h>

Config::Config() : numeroMessage(1) {}

void Config::begin() {
    if (!SPIFFS.begin(true)) {
        Serial.println("An Error has occurred while mounting SPIFFS");
        return;
    }
    loadConfig();
}

void Config::loadConfig() {
    File file = SPIFFS.open("/config.json", "r");
    if (!file) {
        Serial.println("Failed to open config file");
        return;
    }
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, file);
    if (error) {
        Serial.println("Failed to read file, using default configuration");
    } else {
        numeroMessage = doc["numeroMessage"] | 1;
    }
    file.close();
}

void Config::saveConfig() {
    File file = SPIFFS.open("/config.json", "w");
    if (!file) {
        Serial.println("Failed to open config file for writing");
        return;
    }
    StaticJsonDocument<256> doc;
    doc["numeroMessage"] = numeroMessage;
    if (serializeJson(doc, file) == 0) {
        Serial.println("Failed to write to file");
    }
    file.close();
}

int Config::getNumeroMessage() const {
    return numeroMessage;
}

void Config::setNumeroMessage(int numeroMessage) {
    this->numeroMessage = numeroMessage;
}
