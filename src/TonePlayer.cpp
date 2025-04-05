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
    serial2player->begin(9600);

    // Only essential setup in begin()
    pinMode(busyPin, INPUT_PULLUP);
    
    myMP3player.setTimeOut(3000);
    if (!myMP3player.begin(*serial2player, /*isACK = */true, /*doReset = */true))
    {
        Serial.println(F("Unable to begin:"));
        hardwardeInitialized = false;
    }
    else {
        
        Serial.println(F("Hardware initialized"));
        hardwardeInitialized = true;
    }
    myMP3player.setTimeOut(500);
}

/**
 * @brief Initialise le DFPlayer
 * 
 * Tente d'initialiser le DFPlayer, avec une tentative de réinitialisation en cas d'échec
 */
bool TonePlayer::initPlayer() {
    Serial.println(F("Initializing DF player"));
    

    myMP3player.waitAvailable(10000); // Wait for player to be ready
    if (!myMP3player.available()) {
        Serial.println(F("DFPlayer not available"));
        return false;
    }
    hardwardeInitialized = true;
    /*
    // Try initial initialization
    int count = 0;
    while (!checkPlayerState() && count < 10) {
        Serial.print(F("."));
        delay(1000);
        count++;
    }

    // If first attempt fails, try reset
    if (count >= 10) {
        Serial.println(F("\nFirst init attempt failed, trying reset..."));
        myMP3player.reset();
        delay(1000);
        
        count = 0;
        while (!checkPlayerState() && count < 10) {
            Serial.print(F("."));
            delay(1000);
            count++;
        }
        
        if (count >= 10) {
            Serial.println(F("\nPlayer initialization failed"));
            return false;
        }
    }
    */

    update(); // Clear any pending events
    Serial.println(F("\nDFPlayer Mini online."));
    adjustVolume(config.getVolume());
    Serial.println(F("Player initialized"));
    return true;
}

/**
 * @brief Lance la lecture d'un message audio
 * 
 * @param messageNumber Numéro du fichier à lire (correspond au nom du fichier sur la SD)
 * La lecture ne démarre que si aucun autre message n'est en cours
 */
void TonePlayer::playTone(int messageNumber) {
    Serial.println("playTone " + String(messageNumber));
    myMP3player.enableDAC();
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
    
    bool isReady = digitalRead(busyPin) == HIGH;
    //Serial.println("Checking player state " + String(isReady));
    
    if (isReady) {
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
    if (!hardwardeInitialized)  {
        Serial.println(F("Hardware not initialized, cannot update player"));
        return;
    }
    // Check volume changes at the start of update
    checkVolumeChange();


    
    // Check if playback finished
    if (checkPlayerState()) {
        playing = false;
        disableDAC();
        return;
    }

    // Timeout check as fallback
    if (millis() - playStartTime >= PLAY_TIMEOUT) {
        Serial.println("Tone timeout - forcing stop");
        myMP3player.stop();  // Force stop
        playing = false;
        disableDAC();
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

void TonePlayer::enableDAC() {
    myMP3player.enableDAC();
}

void TonePlayer::disableDAC() {
    myMP3player.disableDAC();
}

void TonePlayer::pause() {
    if (playing) {
        myMP3player.pause();
        playing = false;
    }
}

bool TonePlayer::isPlaying() const {
    return playing;
}
