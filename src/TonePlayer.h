#ifndef TONEPLAYER_H
#define TONEPLAYER_H

#include "Arduino.h"
#include <DFRobotDFPlayerMini.h>
#include <vector>
#include <SoftwareSerial.h>

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
     */
    TonePlayer(int rxd2, int txd2);
    
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
    bool isPlaying();

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
     * @brief Ajoute un écouteur pour les événements de fin de lecture
     * @param listener Pointeur vers l'écouteur à ajouter
     */
    void addListener(TonePlayerListener* listener);

private:
    DFRobotDFPlayerMini myMP3player;
    int rxd2;
    int txd2;
    int volume;
    bool playing;
    std::vector<TonePlayerListener*> listeners;
    SoftwareSerial* serial2player;
    unsigned long playStartTime;
    static const unsigned long PLAY_TIMEOUT = 30000; // 30 second timeout
    
    /**
     * @brief Vérifie l'état du lecteur
     * @return true si la lecture est terminée, false sinon
     */
    bool checkPlayerState();
    
    /**
     * @brief Notifie tous les écouteurs de la fin de lecture
     */
    void notifyListeners();
};

#endif // TONEPLAYER_H
