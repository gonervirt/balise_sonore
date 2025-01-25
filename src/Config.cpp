#include "Config.h"
#include "Arduino.h"
#include <Preferences.h>

Preferences preferences;

Config::Config() : numeroMessage(1) {}

void Config::begin() {
    preferences.begin("config", false);
    loadConfig();
}

void Config::loadConfig() {
    numeroMessage = preferences.getInt("numeroMessage", 1);
}

void Config::saveConfig() {
    preferences.putInt("numeroMessage", numeroMessage);
}

int Config::getNumeroMessage() const {
    return numeroMessage;
}

void Config::setNumeroMessage(int numeroMessage) {
    this->numeroMessage = numeroMessage;
}
