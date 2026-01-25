/**
 * @file TonePlayer.h
 * @brief Header for TonePlayer class
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
#ifndef TONEPLAYER_H
#define TONEPLAYER_H

#include "Arduino.h"
#include <DFRobotDFPlayerMini.h>
#include <vector>
#include <SoftwareSerial.h>
#include "Config.h"

/**
 * @brief Interface pour les écouteurs d'événements de lecture de sons
 */
class TonePlayerListener {
public:
    virtual void onToneFinished() = 0;
};

/**
 * @brief Gestionnaire de lecture de fichiers audio MP3 via DFPlayer Mini
 * 
 * Cette classe gère la lecture des messages sonores stockés sur la carte SD
 * du DFPlayer Mini. Elle permet de:
 * - Initialiser la communication avec le DFPlayer
 * - Contrôler la lecture des messages
 * - Gérer le volume
 * - Notifier les écouteurs de la fin de lecture
 */
class TonePlayer {
public:
    /**
     * @brief Constructeur
     * @param rxd2 Broche RX pour la communication série avec le DFPlayer
     * @param txd2 Broche TX pour la communication série avec le DFPlayer
     * @param busyPin Broche GPIO pour surveiller l'état occupé
     * @param config Référence à la configuration
     */
    TonePlayer(int rxd2, int txd2, int busyPin, int powerpin, Config& config);
    
    /**
     * @brief Destructeur - Libère les ressources
     */
    ~TonePlayer();

    /**
     * @brief Initialise le lecteur MP3
     * Configure la communication série et initialise le DFPlayer
     */
    void begin();

     /**
     * @brief Initialise le lecteur MP3
     * Configure la communication série et initialise le DFPlayer
     */
    void startup();
    void startup1();


    /**
     * @brief Lance la lecture d'un message
     * @param messageNumber Numéro du fichier audio à lire (1-99)
     */
    void playTone(int messageNumber);

    /**
     * @brief Ajuste le volume du lecteur
     * @param volume Niveau de volume (0-30)
     */
    void adjustVolume(int volume);

    /**
     * @brief Vérifie si un message est en cours de lecture
     * @return true si lecture en cours, false sinon
     */
    bool isPlaying() const;

    /**
     * @brief Met à jour l'état du lecteur
     * À appeler régulièrement dans la boucle principale
     */
    void update();

    /**
     * @brief Réinitialise le lecteur
     */
    void reset();

    /**
     * @brief Réinitialise le lecteur
     */
    void powerOn() const;

    /**
     * @brief Réinitialise le lecteur
     */
    void powerOff() const;

    bool isAlive();

    /**
     * @brief Vérifie l'état du lecteur
     * @return true si la lecture est terminée, false sinon
     */
    bool busy();

    void readMessage();

    void enableDAC();
    int readVolume();

    bool available();
    bool availableExceptTimeOut();



    

    void printDetail(uint8_t type, int value);
    
    

    /**
     * @brief Ajoute un écouteur pour les événements de fin de lecture
     * @param listener Pointeur vers l'écouteur à ajouter
     */
    void addListener(TonePlayerListener* listener);

private:
    DFRobotDFPlayerMini myMP3player;
    int rxd2;
    int txd2;
    bool playing;
    int powerPin;
    
    SoftwareSerial* serial2player;
    unsigned long playStartTime;
    static const unsigned long PLAY_TIMEOUT = 10000; // 30 second timeout
    Config& config;  // Add reference to config
    int lastConfigVolume;  // Add this to track config volume changes
    int busyPin;  // GPIO pin for busy state monitoring
    

    
    /** Check and update volume if changed in config */
    void checkVolumeChange();
};

#endif // TONEPLAYER_H
