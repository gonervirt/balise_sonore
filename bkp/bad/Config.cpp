#include "Config.h"
#include "Arduino.h"
#include <Preferences.h>

Preferences preferences;

// Constructeur de la classe Config
Config::Config() : numeroMessage(1) {}

// Initialisation de la configuration
void Config::begin() {
    preferences.begin("config", false);
    loadConfig();
}

// Chargement de la configuration depuis la mémoire non volatile
void Config::loadConfig() {
    numeroMessage = preferences.getInt("numeroMessage", 1);
}

// Sauvegarde de la configuration dans la mémoire non volatile
void Config::saveConfig() {
    preferences.putInt("numeroMessage", numeroMessage);
}

// Retourne le numéro du message
int Config::getNumeroMessage() const {
    return numeroMessage;
}

// Définit le numéro du message
void Config::setNumeroMessage(int numeroMessage) {
    this->numeroMessage = numeroMessage;
}
