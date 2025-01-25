#include "Arduino.h"
#include "WebServerManager.h"
#include "RadioMessageHandler.h"
#include "TonePlayer.h"
#include "Config.h"
#include "PushButtonManager.h"
#include "LedManager.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <WiFiAP.h>
#include <FS.h>
#include <SPIFFS.h>

// Configuration du réseau WiFi
const char *ssid = "BALISESONORE";
const char *password = "BaliseSonore_Betton_Mairie";
IPAddress local_ip(192, 168, 10, 1);
IPAddress gateway(192, 168, 10, 1);
IPAddress subnet(255, 255, 255, 0);

// Définition des broches pour la communication et les LEDs
#define RXD2 20  // RX2 pour communiquer avec MP3
#define TXD2 21  // TX2 pour communiquer avec MP3
#define BUTTON_PIN 6 // Définir la broche pour le bouton poussoir
#define GREEN_LED_PIN 0
#define YELLOW_LED_PIN 1
#define RED_LED_PIN 2

// Initialisation des objets de gestion
Config config;
TonePlayer tonePlayer(RXD2, TXD2);
WebServerManager webServerManager(tonePlayer, config);
RadioMessageHandler radioMessageHandler;
PushButtonManager pushButtonManager(BUTTON_PIN);
LedManager ledManager(GREEN_LED_PIN, YELLOW_LED_PIN, RED_LED_PIN);

void setup() {
    pinMode(2, OUTPUT);
    Serial.begin(115200);
    config.begin(); // Initialisation de la configuration
    tonePlayer.begin(); // Initialisation du lecteur de tonalité
    webServerManager.begin(); // Initialisation du gestionnaire de serveur web
    radioMessageHandler.begin(); // Initialisation du gestionnaire de messages radio
    pushButtonManager.begin(); // Initialisation du gestionnaire de bouton poussoir
    ledManager.begin(); // Initialisation du gestionnaire de LEDs

    tonePlayer.addListener(&pushButtonManager); // Enregistrer le gestionnaire de bouton poussoir comme écouteur
    tonePlayer.addListener(&radioMessageHandler); // Enregistrer le gestionnaire de messages radio comme écouteur

    // Configuration du point d'accès WiFi
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(local_ip, gateway, subnet);
    WiFi.softAP(ssid, password);
    Serial.println("AP IP address: " + WiFi.softAPIP().toString());
}

void loop() {
    webServerManager.handleClient(); // Gestion des clients du serveur web
    radioMessageHandler.processMessages(); // Traitement des messages radio
    pushButtonManager.update(); // Mise à jour de l'état du bouton poussoir

    // Gestion de la lecture des tonalités et des LEDs
    if (!tonePlayer.isPlaying() && !tonePlayer.isInhibited()) {
        if (radioMessageHandler.getCurrentMessage() > 0) {
            tonePlayer.playTone(config.getNumeroMessage());
            ledManager.setYellow(); // LED jaune pendant la lecture
        } else if (pushButtonManager.isButtonPressed()) {
            tonePlayer.playTone(config.getNumeroMessage()); // Lecture de la tonalité lorsque le bouton est pressé
            ledManager.setYellow(); // LED jaune pendant la lecture
        } else {
            ledManager.setGreen(); // LED verte en attente de déclenchement
        }
    } else {
        ledManager.setRed(); // LED rouge pendant l'interaction avec le serveur web
    }

    tonePlayer.update(); // Mise à jour du lecteur de tonalité
}
