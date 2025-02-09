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

// Add state machine enum
enum AppState {
    STARTING,
    READY_WAITING,
    PLAYING_TONE,
    INHIBITED
};

// Add state machine variables after other defines
#define STARTING_DURATION 30000    // 30 seconds for starting state
#define INHIBIT_DURATION 10000     // 10 seconds for inhibit state

// Initialize management objects
Config config;
WiFiManager wifiManager(config);
WebServerManager webServer(config);
TonePlayer tonePlayer(RXD2, TXD2);
PushButtonManager pushButtonManager(BUTTON_PIN);
LedManager ledManager(GREEN_LED_PIN, YELLOW_LED_PIN, RED_LED_PIN);

// Add state machine variables
AppState currentState = STARTING;
unsigned long stateStartTime = 0;
bool stateInitialized = false;

// Add after other state machine variables
unsigned long lastToneUpdateTime = 0;
const unsigned long TONE_UPDATE_INTERVAL = 1000; // 1 second interval

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
    //delay(5000);
    Serial.println("TonePlayer initialized");
    //tonePlayer.update();
    //Serial.println("TonePlayer updated");
    //tonePlayer.playTone(1);

    pushButtonManager.begin(); // Initialisation du gestionnaire de bouton poussoir
    Serial.println("PushButtonManager initialized");    


    // register listener
    //tonePlayer.addListener(&pushButtonManager); // Enregistrer le gestionnaire de bouton poussoir comme écouteur
    //Serial.println("PushButtonManager listener added");
    //tonePlayer.addListener(&radioMessageHandler); // Enregistrer le gestionnaire de messages radio comme écouteur
    //Serial.println("RadioMessageHandler listener added"); 

    stateStartTime = millis();  // Initialize state timing
}

void loop() {
    webServer.handleClient();

    // State machine
    switch(currentState) {
        case STARTING:
            /* Entry actions:
             * - Play welcome message (tone 3)
             * 
             * Recurring actions:
             * - None
             * 
             * Exit condition:
             * - After 30 seconds (STARTING_DURATION)
             * - Transitions to READY_WAITING
             */
            if (!stateInitialized) {
                Serial.println("State: STARTING");
                tonePlayer.playTone(4);
                ledManager.setYellow();
                stateInitialized = true;
            }

            // recurring
            //webServer.handleClient();
            // Rate-limited tone player update
            if (millis() - lastToneUpdateTime >= TONE_UPDATE_INTERVAL) {
                tonePlayer.update();
                lastToneUpdateTime = millis();
            }

            if (!tonePlayer.isPlaying()) {
                currentState = READY_WAITING;
                stateStartTime = millis();
                stateInitialized = false;
            }
            
            if (millis() - stateStartTime >= STARTING_DURATION) {
                currentState = READY_WAITING;
                stateStartTime = millis();
                stateInitialized = false;
            }
            break;

        case READY_WAITING:
            /* Entry actions:
             * - Set LED to green
             * 
             * Recurring actions:
             * - Check for button press
             * 
             * Exit condition:
             * - Button is pressed
             * - Transitions to PLAYING_TONE
             */
            if (!stateInitialized) {
                Serial.println("State: READY_WAITING");
                ledManager.setGreen();
                stateInitialized = true;
            }
            // recurring
            pushButtonManager.update();
            //webServer.handleClient();
            // Rate-limited tone player update
            /*
            if (millis() - lastToneUpdateTime >= TONE_UPDATE_INTERVAL) {
                tonePlayer.update();
                lastToneUpdateTime = millis();
            }
            */

            if (pushButtonManager.isButtonPressed()) {
                currentState = PLAYING_TONE;
                stateStartTime = millis();
                stateInitialized = false;
                pushButtonManager.releaseButtonPressed();
            }
            break;

        case PLAYING_TONE:
            /* Entry actions:
             * - Set LED to yellow
             * - Start playing configured message
             * 
             * Recurring actions:
             * - Check if tone has finished playing (rate limited to once per second)
             * 
             * Exit condition:
             * - Tone finishes playing
             * - Transitions to INHIBITED
             */
            if (!stateInitialized) {
                Serial.println("State: PLAYING_TONE");
                ledManager.setYellow();
                tonePlayer.playTone(config.getNumeroMessage());
                stateInitialized = true;
                lastToneUpdateTime = millis();
            }

            // recurring
            //webServer.handleClient();
            // Rate-limited tone player update
            if (millis() - lastToneUpdateTime >= TONE_UPDATE_INTERVAL) {
                tonePlayer.update();
                lastToneUpdateTime = millis();
            }

            if (!tonePlayer.isPlaying()) {
                currentState = INHIBITED;
                tonePlayer.update();
                stateStartTime = millis();
                stateInitialized = false;
            }
            break;

        case INHIBITED:
            /* Entry actions:
             * - Set LED to red
             * 
             * Recurring actions:
             * - None (just waiting)
             * 
             * Exit condition:
             * - After 10 seconds (INHIBIT_DURATION)
             * - Transitions to READY_WAITING
             */
            if (!stateInitialized) {
                Serial.println("State: INHIBITED");
                ledManager.setRed();
                stateInitialized = true;
            }

            // recurring
            //webServer.handleClient();
            // Rate-limited tone player update
            if (millis() - lastToneUpdateTime >= TONE_UPDATE_INTERVAL) {
                tonePlayer.update();
                lastToneUpdateTime = millis();
            }

            if (millis() - stateStartTime >= INHIBIT_DURATION) {
                currentState = READY_WAITING;
                stateStartTime = millis();
                stateInitialized = false;
            }
            break;
    }

    delay(5);  // Prevent watchdog reset
}
