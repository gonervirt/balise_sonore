#include <SoftwareSerial.h>
#include "TonePlayer.h"
#include "Arduino.h"
#include "Config.h"

/**
 * @brief Initialise le gestionnaire de lecture avec les broches spécifiées
 * 
 * Configure les broches RX/TX et crée l'objet de communication série
 */
TonePlayer::TonePlayer(int _rxd2, int _txd2, Config& config) 
    : volume(config.getVolume()), playing(false), config(config) {
    rxd2 = _rxd2;
    txd2 = _txd2;
    serial2player = new SoftwareSerial(rxd2, txd2);
}

/**
 * @brief Nettoie les ressources lors de la destruction
 */
TonePlayer::~TonePlayer() {
    if (serial2player) {
        delete serial2player;
        serial2player = nullptr;
    }
}

/**
 * @brief Configure la communication avec le DFPlayer
 * 
 * Initialise la communication série, configure le DFPlayer
 * et attend que le module soit prêt
 */
void TonePlayer::begin() {
    if (!serial2player) {
        Serial.println("Error: serial2player not initialized");
        return;
    }

    serial2player->begin(9600);
    myMP3player.setTimeOut(1000);
    myMP3player.begin(*serial2player, /*isACK = */true, /*doReset = */true);
    Serial.println(F("Waiting DF player"));
    delay(1000);
    myMP3player.reset();
    
    int count = 0;
    while (!myMP3player.available() && count < 30) {
        Serial.print(F("."));
        delay(1000);
        count++;
    }
    update(); // Clear any pending events
    Serial.println(F(""));

    Serial.println(F("DFPlayer Mini online."));
    myMP3player.enableDAC();
    myMP3player.volume(config.getVolume());  // Use volume from config
    Serial.println(F("Player initialized"));
}

/**
 * @brief Lance la lecture d'un message audio
 * 
 * @param messageNumber Numéro du fichier à lire (correspond au nom du fichier sur la SD)
 * La lecture ne démarre que si aucun autre message n'est en cours
 */
void TonePlayer::playTone(int messageNumber) {
    Serial.println("playTone " + String(messageNumber));
    myMP3player.play(messageNumber);
    playing = true;
    playStartTime = millis();
}

/**
 * @brief Vérifie l'état actuel du lecteur
 * 
 * Interroge le DFPlayer pour connaître son état et détecter
 * la fin de lecture ou les erreurs éventuelles
 */
bool TonePlayer::checkPlayerState() {
    if (!myMP3player.available()) {
        Serial.println("DF player is *not* available");
        return false;
    } else {
        Serial.println("DF player is available");
    }

    uint8_t type = myMP3player.readType();
    int value = myMP3player.read();
    
    Serial.printf("Player state - Type: %d, Value: %d\n", type, value);
    
    switch (type) {
        case DFPlayerPlayFinished:
            Serial.println("Tone finished (DFPlayerPlayFinished event detected)");
            return true;
        case DFPlayerError:
            Serial.println("Player error detected");
            return true;
        case TimeOut:
            Serial.println("Player timeout detected");
            return true;
        default:
            return false;
    }
}

/**
 * @brief Met à jour l'état du lecteur
 * 
 * Vérifie si la lecture est terminée par plusieurs méthodes:
 * 1. Événements du DFPlayer
 * 2. État direct du lecteur
 * 3. Timeout de sécurité
 */
void TonePlayer::update() {
    Serial.println("Tone update called");
    // Method 1: Check for completion events
    if (checkPlayerState()) {
        Serial.println("Tone finished (event detected)");
        playing = false;
        return;
    }

    // Method 2: Check player status directly
    if (myMP3player.readState() == 512) { // 512 typically means stopped
        Serial.println("Tone finished (status check)");
        playing = false;
        return;
    }

    // Method 3: Timeout check
    if (millis() - playStartTime >= PLAY_TIMEOUT) {
        Serial.println("Tone timeout - forcing stop");
        myMP3player.stop();  // Force stop
        playing = false;
        return;
    }
}

void TonePlayer::adjustVolume(int volume) {
    config.setVolume(volume);  // Save volume to config
    this->volume = config.getVolume();
    myMP3player.volume(this->volume);
}


bool TonePlayer::isPlaying() {
    return playing;
}


void TonePlayer::addListener(TonePlayerListener* listener) {
    listeners.push_back(listener);
}

void TonePlayer::notifyListeners() {
    for (TonePlayerListener* listener : listeners) {
        listener->onToneFinished();
    }
}
