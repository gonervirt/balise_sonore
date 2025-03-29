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
#include "RadioMessageHandler.h"
#include "esp_wifi.h"  // Include for power management configuration

#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "development"
#endif


// Pin definitions based on board type
#if defined(BOARD_LOLIN_C3_MINI)
    #define RXD2 20
    #define TXD2 21
//    #define RXD2 6
//    #define TXD2 7
    #define BUSY_PIN 10
    #define BUTTON_PIN 6
//    #define BUTTON_PIN 5
    #define GREEN_LED_PIN 2
    #define YELLOW_LED_PIN 1
    #define RED_LED_PIN 0
#elif defined(BOARD_ESP32)
    #define RXD2 17
    #define TXD2 18
    #define BUSY_PIN 19
    #define GREEN_LED_PIN 47
    #define YELLOW_LED_PIN 42
    #define RED_LED_PIN 37
    #define RADIO_PIN 15
#elif defined(BOARD_ESP32_S3)
    #define RXD2 18
    #define TXD2 17
    #define BUSY_PIN 48
    #define GREEN_LED_PIN 47
    #define YELLOW_LED_PIN 42
    #define RED_LED_PIN 13
    #define RADIO_PIN 15
#else  // Default to LOLIN C3 Mini for any invalid or undefined value
    #define RXD2 20
    #define TXD2 21
    #define BUSY_PIN 19
    #define BUTTON_PIN 6
    #define GREEN_LED_PIN 2
    #define YELLOW_LED_PIN 1
    #define RED_LED_PIN 0
    #define RADIO_PIN 9
#endif

// default wifi
//  wifi_ssid, "BALISESONORE"
//  wifi_password, "BaliseSonore_Betton_Mairie"

// Add state machine enum
enum AppState
{
    STARTUP_BLINK,
    INIT_PLAYER,    // New state
    STARTING,
    READY_WAITING,
    PLAYING_TONE,
    INHIBITED,
    DESACTIVATED
};

// Add state machine variables after other defines
#define STARTING_DURATION 30000 // 30 seconds for starting state
#define INHIBIT_DURATION 10000  // 10 seconds for inhibit state

// Initialize management objects
Config config;
//WiFiManager wifiManager(config);
WebServerManager *webServer;
TonePlayer tonePlayer(RXD2, TXD2, BUSY_PIN, config);  // Updated constructor call
//PushButtonManager pushButtonManager(BUTTON_PIN);
LedManager ledManager(GREEN_LED_PIN, YELLOW_LED_PIN, RED_LED_PIN);
//RadioMessageHandler radioHandler(RADIO_PIN);

#ifdef BOARD_LOLIN_C3_MINI
PushButtonManager inputHandler(BUTTON_PIN);
#elif defined(BOARD_ESP32_S3)
RadioMessageHandler inputHandler(RADIO_PIN);
#elif defined(BOARD_ESP32)
RadioMessageHandler inputHandler(RADIO_PIN);
#else
#error "No input handler defined for this board"
#endif

// Add state machine variables
AppState currentState = STARTUP_BLINK;
unsigned long stateStartTime = 0;
bool stateInitialized = false;

// Add after other state machine variables
unsigned long lastToneUpdateTime = 0;
const unsigned long TONE_UPDATE_INTERVAL = 1000; // 1 second interval

// Add after other global variables
unsigned long lastWifiCheckTime = 0;
const unsigned long WIFI_CHECK_INTERVAL = 5000; // Check every 5 seconds

// Add variables for blinking state
unsigned long blinkStartTime = 0;
unsigned long lastBlinkTime = 0;
const int STARTUP_BLINK_DURATION = 10000; // 10 seconds
const int BLINK_INTERVAL = 500; // 500ms blink rate


void setup()
{
    Serial.begin(115200);

    Serial.println("\n\nStarting ESP32 Balise Sonore...");
    Serial.printf("Version %s Compile time: %s %s\n", FIRMWARE_VERSION, __DATE__, __TIME__);

    ledManager.begin(); // Initialisation du gestionnaire de LEDs
    Serial.println("LedManager initialized");



    // Initialize configuration
    config.begin();
    Serial.println("Config initialized");

    // Initialize WiFi
    WiFi.mode(WIFI_AP);
    bool success = WiFi.softAP(config.getWifiSSID(), config.getWifiPassword());

    // Initialize WebServerManager
    webServer = new WebServerManager(config);
    webServer->begin();
    Serial.println("WebServerManager initialized");

    tonePlayer.begin(); // Initialisation du lecteur de tonalité
    Serial.println("TonePlayer initialized");

    inputHandler.begin(); // Initialisation du gestionnaire de messages radio
    Serial.println("InputHandler initialized");

    stateStartTime = millis(); // Initialize state timing
    stateInitialized = false;

    // Initialize blinking state variables
    currentState = STARTUP_BLINK;
    blinkStartTime = millis();
}

void loop()
{
    // Add at the beginning of the loop function
    webServer->handleClient();
    //wifiManager.loop();

    // Monitor heap memory
    //Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());

    // State machine
    switch (currentState)
    {
    case STARTUP_BLINK:
        /* Entry actions:
         * - Blink green and yellow LEDs alternately
         *
         * Recurring actions:
         * - Blink LEDs every BLINK_INTERVAL ms
         *
         * Exit condition:
         * - After 10 seconds (STARTUP_BLINK_DURATION)
         * - Transitions to INIT_PLAYER
         */
        if (!stateInitialized)
        {
            Serial.println("State: STARTUP_BLINK");
            stateInitialized = true;
        }

        if (millis() - blinkStartTime < STARTUP_BLINK_DURATION)
        {
            if (millis() - lastBlinkTime >= BLINK_INTERVAL)
            {
                ledManager.setGreen();
                delay(BLINK_INTERVAL / 2);
                ledManager.setYellow();
                delay(BLINK_INTERVAL / 2);
                lastBlinkTime = millis();
            }
        }
        else
        {
            ledManager.setGreenYellow(); // Turn off LEDs after the wait period
            currentState = INIT_PLAYER;  // Changed transition to new state
            stateStartTime = millis();
            stateInitialized = false;
        }
        break;

    case INIT_PLAYER:
        /* Entry actions:
         * - Call initPlayer()
         *
         * Recurring actions:
         * - None
         *
         * Exit condition:
         * - Player initialization succeeds or fails
         * - Transitions to STARTING on success
         */
        if (!stateInitialized)
        {
            Serial.println("State: INIT_PLAYER");
            ledManager.setYellow();
            stateInitialized = true;
        }

        if (tonePlayer.initPlayer())
        {
            currentState = STARTING;
            stateStartTime = millis();
            stateInitialized = false;
        }
        else
        {
            // If initialization fails, retry after a delay
            delay(1000);
            stateInitialized = false;
        }
        break;

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
        if (!stateInitialized)
        {
            Serial.println("State: STARTING");
            tonePlayer.playTone(4);
            ledManager.setYellow();
            stateInitialized = true;
        }

        // recurring
        // webServer->handleClient();
        // Rate-limited tone player update
        if (millis() - lastToneUpdateTime >= TONE_UPDATE_INTERVAL)
        {
            tonePlayer.update();
            lastToneUpdateTime = millis();

            if (!tonePlayer.isPlaying())
            {
                currentState = READY_WAITING;
                stateStartTime = millis();
                stateInitialized = false;

                if (millis() - stateStartTime >= STARTING_DURATION)
                {
                    currentState = READY_WAITING;
                    stateStartTime = millis();
                    stateInitialized = false;
                }
            }
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
        if (!stateInitialized)
        {
            Serial.println("State: READY_WAITING");
            ledManager.setGreen();
            stateInitialized = true;
        }
        // recurring
        inputHandler.update();

        if (inputHandler.isActivated())
        {
            currentState = PLAYING_TONE;
            stateStartTime = millis();
            stateInitialized = false;

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
        if (!stateInitialized)
        {
            Serial.println("State: PLAYING_TONE");
            ledManager.setYellow();
            tonePlayer.playTone(config.getNumeroMessage());
            stateInitialized = true;
            lastToneUpdateTime = millis();
        }

        // recurring
        // webServer->handleClient();
        // Rate-limited tone player update
        if (millis() - lastToneUpdateTime >= TONE_UPDATE_INTERVAL)
        {
            tonePlayer.update();
            lastToneUpdateTime = millis();

            if (!tonePlayer.isPlaying())
            {
                currentState = INHIBITED;
                tonePlayer.update();
                stateStartTime = millis();
                stateInitialized = false;
            }
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
        if (!stateInitialized)
        {
            Serial.println("State: INHIBITED");
            ledManager.setGreenYellow();
            stateInitialized = true;
        }

        // recurring
        // webServer->handleClient();
        // Rate-limited tone player update
        if (millis() - lastToneUpdateTime >= TONE_UPDATE_INTERVAL)
        {
            tonePlayer.update();
            lastToneUpdateTime = millis();

            if (millis() - stateStartTime >= INHIBIT_DURATION)
            {
                currentState = READY_WAITING;
                inputHandler.resetActivation();
                stateStartTime = millis();
                stateInitialized = false;
            }
        }
        break;

    case DESACTIVATED:
        break;
    }

    delay(5); // Prevent watchdog reset
}
