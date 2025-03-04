#include <SoftwareSerial.h>
#include "TonePlayer.h"
#include "Arduino.h"
#include "Config.h"

/**
 * @brief Initialise le gestionnaire de lecture avec les broches spécifiées
 * 
 * Configure les broches RX/TX et crée l'objet de communication série
 */
TonePlayer::TonePlayer(int _rxd2, int _txd2, int _busyPin, Config& config) 
    : playing(false), config(config), lastConfigVolume(config.getVolume()) {
    rxd2 = _rxd2;
    txd2 = _txd2;
    busyPin = _busyPin;
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

    // Configure busy pin as input with pull-up
    pinMode(busyPin, INPUT_PULLUP);

    serial2player->begin(9600);
    myMP3player.setTimeOut(1000);
    myMP3player.begin(*serial2player, /*isACK = */true, /*doReset = */true);
    Serial.println(F("Waiting DF player"));
    delay(4000);
    //myMP3player.reset();
    //delay(4000);
    
    
    int count = 0;
    while (digitalRead(busyPin) == LOW && count < 10) {
        Serial.print(F("."));
        delay(1000);
        update(); // Clear any pending events
        count++;
    }
    //update(); // Clear any pending events
    Serial.println(F(""));
    

    Serial.println(F("DFPlayer Mini online."));
    myMP3player.enableDAC();
    adjustVolume(config.getVolume());  // Use volume from config
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
    // HIGH means player is ready/idle, LOW means it's busy playing
    
    //bool isReady = digitalRead(busyPin) == HIGH;
    //Serial.println("Checking player state " + String(isReady));
    playing = false;
    return false;

    bool isReady = true;
    
    if (isReady && playing) {
        Serial.println("Tone finished (busy pin is HIGH)");
        return true;
    }
    
    return false;
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
    
    // Check volume changes at the start of update
    checkVolumeChange();
    
    // Check if playback finished
    if (checkPlayerState()) {
        playing = false;
        return;
    }

    // Timeout check as fallback
    if (millis() - playStartTime >= PLAY_TIMEOUT) {
        Serial.println("Tone timeout - forcing stop");
        myMP3player.stop();  // Force stop
        playing = false;
        return;
    }
}

void TonePlayer::checkVolumeChange()
{
    uint8_t currentConfigVolume = config.getVolume();
    if (currentConfigVolume != lastConfigVolume)
    {
        Serial.printf("Volume changed in config from %d to %d\n", lastConfigVolume, currentConfigVolume);
        adjustVolume(currentConfigVolume);
    }
}

void TonePlayer::adjustVolume(int volume) {
    lastConfigVolume = volume;  // Update the tracking variable
    myMP3player.volume(volume);
}


bool TonePlayer::isPlaying() const {
    return playing;
}
