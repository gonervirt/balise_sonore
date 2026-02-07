/**
 * @file TonePlayer.cpp
 * @brief Implementation of the TonePlayer class for managing DFPlayer Mini
 * 
 * @copyright Copyright (c) 2024 ESP32 Balise Sonore Project
 * 
 * @license MIT License
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#include <SoftwareSerial.h>
#include "TonePlayer.h"
#include "Arduino.h"
#include "Config.h"

/**
 * @brief Initialise le gestionnaire de lecture avec les broches spécifiées
 * 
 * Configure les broches RX/TX et crée l'objet de communication série
 */
TonePlayer::TonePlayer(int _rxd2, int _txd2, int _busyPin, int _powerpin, Config& config) 
    : playing(false), config(config), lastConfigVolume(config.getVolume()) {
    rxd2 = _rxd2;
    txd2 = _txd2;
    busyPin = _busyPin;
    powerPin = _powerpin;
    serial2player = new SoftwareSerial(rxd2, txd2);
    pinMode(powerPin, OUTPUT);
    // Configure busy pin as input with pull-up
    pinMode(busyPin, INPUT_PULLUP);
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
    myMP3player.begin(*serial2player, /*isACK = */false, /*doReset = */true);
    readMessage();  // Read initial message from DFPlayer
    //myMP3player.setTimeOut(500);
    //Serial.println(F("Waiting DF player"));
    /*
    delay(5000);
    myMP3player.reset();
    delay(10000);

    readMessage();
    */
    Serial.println(F("DF player begin completed."));
    }

/**
 * @brief Enables the DAC on the DFPlayer
 */
void TonePlayer::enableDAC() {  
    myMP3player.enableDAC();  // Enable DAC output
    Serial.println(F("DAC enabled"));
}

/**
 * @brief Reads the current volume from the DFPlayer
 * @return The current volume level (0-30)
 */
int TonePlayer::readVolume() {  
    return myMP3player.readVolume  ();  // read current volume
}

/**
 * @brief Resets the DFPlayer module
 */
void TonePlayer::reset() {  
    return myMP3player.reset  ();  // reset
}

/**
 * @brief Startup sequence for the player
 * Reads initial messages, sets timeouts, and applies volume configuration.
 */
void TonePlayer::startup() {
    //delay(3000);
    //myMP3player.reset();
    //delay(10000);

    readMessage();
    int vol = myMP3player.readVolume();
    Serial.printf("Volume read from DFPlayer: %d\n", vol);
    readMessage();
    //Serial.printf("waitAvailable");
    //myMP3player.waitAvailable(1000); // Wait for DFPlayer to be ready
    myMP3player.setTimeOut(500); //Set serial communictaion time out 500ms
    //myMP3player.enableDAC();
    adjustVolume(config.getVolume());  // Use volume from config
    Serial.printf("Volume set to %d \n", config.getVolume());
    vol = myMP3player.readVolume();
    Serial.printf("Volume read from DFPlayer: %d\n", vol);
    Serial.println(F("Player initialized"));
    //myMP3player.play(4);

}

/**
 * @brief Reads available messages from the DFPlayer serial interface
 * Prints details of the message if available.
 */
void TonePlayer::readMessage() {
    if (myMP3player.available()) {
        printDetail(myMP3player.readType(), myMP3player.read()); //Print the detail message from DFPlayer to handle different errors and states.
    }
    else {
        Serial.println(F("No message available from DFPlayer"));
    }
}

/**
 * @brief Checks if the player is responsive
 * @return true if the player responds to commands
 */
bool TonePlayer::isAlive() {
    // Try to read current volume as a health check
    int vol = myMP3player.readVolume();
    return myMP3player.available();
}

/**
 * @brief Checks if data is available from the player
 * @return true if data is available
 */
bool TonePlayer::available() {
    return myMP3player.available();
} 

/**
 * @brief Checks if data is available, ignoring TimeOut messages
 * @return true if a non-timeout message is available
 */
bool TonePlayer::availableExceptTimeOut() {
    if (myMP3player.available())
      {
        uint8_t type = myMP3player.readType();
        if (type != TimeOut) {
            return true;  // Only return true if it's not a timeout message
        }
    } 
    return false;  // No available message or it's a timeout message
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
    delay(500);
}

/**
 * @brief Vérifie l'état actuel du lecteur
 * 
 * Interroge le DFPlayer pour connaître son état et détecter
 * la fin de lecture ou les erreurs éventuelles
 */
bool TonePlayer::busy() {
    // HIGH means player is ready/idle, LOW means it's busy playing
    if (digitalRead(busyPin) == LOW) {
      // Player is busy
        return true;
    }
    // player is idle
    Serial.println("Player is idle (hardware detection: busy pin is low)"); 
    Serial.println(F("Current play Finished!"));
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
    // Check volume changes at the start of update
    checkVolumeChange();
    
    // Check if playback finished
    if (busy()) {
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

/**
 * @brief Checks if the volume configuration has changed and updates the player
 */
void TonePlayer::checkVolumeChange()
{
    uint8_t currentConfigVolume = config.getVolume();
    if (currentConfigVolume != lastConfigVolume)
    {
        Serial.printf("Volume changed in config from %d to %d\n", lastConfigVolume, currentConfigVolume);
        adjustVolume(currentConfigVolume);
    }
}

/**
 * @brief Sets the volume on the DFPlayer
 * @param volume The volume level (0-30)
 */
void TonePlayer::adjustVolume(int volume) {
    lastConfigVolume = volume;  // Update the tracking variable
    myMP3player.volume(volume);
}


/**
 * @brief Returns the internal playing state
 * @return true if the class considers itself playing
 */
bool TonePlayer::isPlaying() const {
    return playing;
}

/**
 * @brief Powers on the DFPlayer module via the power pin
 */
void TonePlayer::powerOn() const {
    digitalWrite(powerPin, HIGH);  // Power on the player
    Serial.println("DF mini Power on");
}

/**
 * @brief Powers off the DFPlayer module via the power pin
 */
void TonePlayer::powerOff() const {
    digitalWrite(powerPin, LOW);  // Power off the player
    Serial.println("DF mini Power off");
}

/**
 * @brief Prints detailed debug information based on DFPlayer message types
 */
void TonePlayer::printDetail(uint8_t type, int value){
  switch (type) {
    case TimeOut:
      Serial.println(F("                    ---> Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerUSBInserted:
      Serial.println("USB Inserted!");
      break;
    case DFPlayerUSBRemoved:
      Serial.println("USB Removed!");
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("DFPlayerPlayFinished --> software detection"));
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}
