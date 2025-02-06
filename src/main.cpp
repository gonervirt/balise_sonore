/**
 * ESP32 Balise Sonore
 * 
 * Main program file that initializes and manages:
 * - Configuration system
 * - WiFi connection
 * - Web interface
 * 
 * The device can operate in either Access Point or Station mode
 * and provides a web interface for configuration.
 */

#include <Arduino.h>
#include "wifi_manager.h"
#include "web_server_manager.h"
#include "PushButtonManager.h"
#include "LedManager.h"
#include "TonePlayer.h"
#include "Config.h"

#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "development"
#endif

// Définition des broches pour la communication et les LEDs
#define RXD2 20  // RX2 pour communiquer avec MP3
#define TXD2 21  // TX2 pour communiquer avec MP3
#define BUTTON_PIN 6 // Définir la broche pour le bouton poussoir
#define GREEN_LED_PIN 2
#define YELLOW_LED_PIN 1
#define RED_LED_PIN 0

// Initialize management objects
Config config;
WiFiManager wifiManager(config);
WebServerManager webServer(config);
TonePlayer tonePlayer(RXD2, TXD2);
PushButtonManager pushButtonManager(BUTTON_PIN);
LedManager ledManager(GREEN_LED_PIN, YELLOW_LED_PIN, RED_LED_PIN);

// Add debug timing variables
unsigned long lastLoopTime = 0;
unsigned long currentTime = 0;
unsigned long webServerTime = 0;
unsigned long buttonTime = 0;
unsigned long playerTime = 0;
unsigned long totalLoopTime = 0;

// Wait for Serial with timeout
void waitForSerial(unsigned long timeout_ms = 10000) {
    unsigned long start = millis();
    while (!Serial && (millis() - start) < timeout_ms) {
        delay(100);
    }
}

void setup() {
    Serial.begin(115200);
    waitForSerial();  // Wait up to 10 seconds for Serial
    
    Serial.println("\n\nStarting ESP32 Balise Sonore...");
    Serial.printf("Version %s Compile time: %s %s\n", FIRMWARE_VERSION, __DATE__, __TIME__);
    
    // Initialize configuration
    config.begin();

    ledManager.begin(); // Initialisation du gestionnaire de LEDs
    Serial.println("LedManager initialized"); 
    
    // Initialize WiFi
    if (wifiManager.begin()) {
        Serial.println("WiFi ready");
        Serial.println("IP: " + wifiManager.getIP());
        webServer.begin();
    } else {
        Serial.println("WiFi failed!");
    }

    tonePlayer.begin(); // Initialisation du lecteur de tonalité
    Serial.println("TonePlayer initialized");
    tonePlayer.update();
    Serial.println("TonePlayer updated");

    pushButtonManager.begin(); // Initialisation du gestionnaire de bouton poussoir
    Serial.println("PushButtonManager initialized");    


    // register listener
    //tonePlayer.addListener(&pushButtonManager); // Enregistrer le gestionnaire de bouton poussoir comme écouteur
    //Serial.println("PushButtonManager listener added");
    //tonePlayer.addListener(&radioMessageHandler); // Enregistrer le gestionnaire de messages radio comme écouteur
    //Serial.println("RadioMessageHandler listener added"); 
}

void loop() {
    currentTime = millis();
    
    // Web Server handling
    unsigned long startTime = micros();
    webServer.handleClient();
    webServerTime = micros() - startTime;
    //Serial.printf("[%lu] Web server time: %lu us\n", currentTime, webServerTime);

    // Button handling
    startTime = micros();
    pushButtonManager.update();
    buttonTime = micros() - startTime;
    //Serial.printf("[%lu] Button handling time: %lu us\n", currentTime, buttonTime);

    // Player handling
    startTime = micros();
    //Serial.println("TonePlayer update called");
    tonePlayer.update();
    //Serial.println("TonePlayer update finished");
    playerTime = micros() - startTime;
    //Serial.printf("[%lu] Player handling time: %lu us\n", currentTime, playerTime);

    // Tone and LED management
    if (pushButtonManager.isButtonPressed()) {
        Serial.println("Playing tone for button press");
        Serial.println(String (config.getNumeroMessage()));
        ledManager.setYellow();
        tonePlayer.playTone(config.getNumeroMessage());
        
    }
    //Serial.printf("[%lu] Button pressed: %d\n", currentTime, pushButtonManager.isButtonPressed());

    // Calculate total loop time
    totalLoopTime = millis() - currentTime;

    delay(100);  // Prevent watchdog reset 100ms

    // Print timing information every second
    /*
    if (millis() - lastLoopTime >= 1000) {
        Serial.printf("[%lu] Timing (us) - Web: %lu, Button: %lu, Player: %lu, Total: %lu ms\n",
                     currentTime,
                     webServerTime,
                     buttonTime,
                     playerTime,
                     totalLoopTime);
        lastLoopTime = millis();
    }
*/
   
}
