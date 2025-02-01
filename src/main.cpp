#include "Arduino.h"
#include "WebServerManager.h"
#include "RadioMessageHandler.h"
#include "TonePlayer.h"
#include "Config.h"
#include "PushButtonManager.h"
#include "LedManager.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
//#include <WiFiAP.h>
#include <FS.h>
//#include <SPIFFS.h>

// Configuration du réseau WiFi
const char *ssid = "testAP";
const char *password = "123456ABC";


// Définition des broches pour la communication et les LEDs
#define RXD2 20  // RX2 pour communiquer avec MP3
#define TXD2 21  // TX2 pour communiquer avec MP3
#define BUTTON_PIN 6 // Définir la broche pour le bouton poussoir
#define GREEN_LED_PIN 2
#define YELLOW_LED_PIN 1
#define RED_LED_PIN 0

// Initialisation des objets de gestion
Config config;
TonePlayer tonePlayer(RXD2, TXD2);
WebServerManager webServerManager(tonePlayer, config);
RadioMessageHandler radioMessageHandler;
PushButtonManager pushButtonManager(BUTTON_PIN);
LedManager ledManager(GREEN_LED_PIN, YELLOW_LED_PIN, RED_LED_PIN);




void setup() {
    //pinMode(2, OUTPUT);
    Serial.begin(115200);
    Serial.println("Setup started");
    
    ledManager.begin(); // Initialisation du gestionnaire de LEDs
    Serial.println("LedManager initialized");   
    // Configuration du point d'accès WiFi
    WiFi.begin();
    WiFi.softAP(ssid, password);
    Serial.println("WiFi AP configured");
    Serial.println("AP IP address: " + WiFi.softAPIP().toString());

    config.begin(); // Initialisation de la configuration
    Serial.println("Config initialized");
    tonePlayer.begin(); // Initialisation du lecteur de tonalité
    Serial.println("TonePlayer initialized");
    webServerManager.begin(); // Initialisation du gestionnaire de serveur web
    Serial.println("WebServerManager initialized");
    //radioMessageHandler.begin(); // Initialisation du gestionnaire de messages radio
    //Serial.println("RadioMessageHandler initialized");
    pushButtonManager.begin(); // Initialisation du gestionnaire de bouton poussoir
    Serial.println("PushButtonManager initialized");
    

    tonePlayer.addListener(&pushButtonManager); // Enregistrer le gestionnaire de bouton poussoir comme écouteur
    Serial.println("PushButtonManager listener added");
    tonePlayer.addListener(&radioMessageHandler); // Enregistrer le gestionnaire de messages radio comme écouteur
    Serial.println("RadioMessageHandler listener added");

    
}

void loop() {
    //Serial.println("Loop start");
    webServerManager.handleClient(); // Gestion des clients du serveur web
    //Serial.println("Handled web server client");
    radioMessageHandler.processMessages(); // Traitement des messages radio
    //Serial.println("Processed radio messages");
    pushButtonManager.update(); // Mise à jour de l'état du bouton poussoir
    //Serial.println("Updated push button manager");

    // Gestion de la lecture des tonalités et des LEDs
     if (pushButtonManager.isButtonPressed()) {
            Serial.println("Playing tone for button press");
            tonePlayer.playTone(config.getNumeroMessage()); // Lecture de la tonalité lorsque le bouton est pressé
            ledManager.setYellow(); // LED jaune pendant la lecture
    } else
        if (radioMessageHandler.isMessageReady()) {
            Serial.println("Playing tone for radio message");
            tonePlayer.playTone(config.getNumeroMessage());
            ledManager.setYellow(); // LED jaune pendant la lecture
        } else {
            //Serial.println("Setting LED to green");
            ledManager.setGreen(); // LED verte en attente de déclenchement
        }


    tonePlayer.update(); // Mise à jour du lecteur de tonalité
    //Serial.println("Updated tone player");
    delay(500);
    //Serial.println("Loop end\n");
}
