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

#include "PushButtonManager.h"
#include "LedManager.h"
#include "TonePlayer.h"
#include "Config.h"
#include "RadioMessageHandler.h"
#include "Timer.h"


#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "development"
#endif

#define DISABLE_WIFI 1 

#ifndef DISABLE_WIFI
#include "wifi_manager.h"
#include "web_server_manager.h"
#include "esp_wifi.h"  // Include for power management configuration
#endif

// Pin definitions based on board type
#if defined(BOARD_LOLIN_C3_MINI)
    #define RXD2 20
    #define TXD2 21
    #define BUSY_PIN 10
    #define BUTTON_PIN 6
    #define GREEN_LED_PIN 2
    #define YELLOW_LED_PIN 1
    #define RED_LED_PIN 0
    #define TONE_PLAYER_POWER_PIN 9
    #define DEFAULT_VOLUME 5
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
    #define TONE_PLAYER_POWER_PIN 41
    #define DEFAULT_VOLUME 30
#else  // Default to LOLIN C3 Mini for any invalid or undefined value
    #define RXD2 20
    #define TXD2 21
    #define BUSY_PIN 19
    #define BUTTON_PIN 6
    #define GREEN_LED_PIN 2
    #define YELLOW_LED_PIN 1
    #define RED_LED_PIN 0
    #define TONE_PLAYER_POWER_PIN 9
#endif

// default wifi
//  wifi_ssid, "BALISESONORE"
//  wifi_password, "BaliseSonore_Betton_Mairie"







#ifndef DISABLE_WIFI
//WiFiManager wifiManager(config);
WebServerManager *webServer;
#endif

// Initialize management objects
Config config(DEFAULT_VOLUME); // Pass default volume to Config constructor
TonePlayer tonePlayer(RXD2, TXD2, BUSY_PIN, TONE_PLAYER_POWER_PIN, config);  // Updated constructor call
LedManager ledManager(GREEN_LED_PIN, YELLOW_LED_PIN, RED_LED_PIN);
Timer timer; // Timer for managing timeouts
// Initialize input handler based on board type
#ifdef BOARD_LOLIN_C3_MINI
PushButtonManager inputHandler(BUTTON_PIN);
#elif defined(BOARD_ESP32_S3)
RadioMessageHandler inputHandler(RADIO_PIN);
#elif defined(BOARD_ESP32)
RadioMessageHandler inputHandler(RADIO_PIN);
#else
#error "No input handler defined for this board"
#endif

// Add state machine enum
enum AppState
{
    STARTING,
    START_TONE_PLAYER_1,
    START_TONE_PLAYER_2,
    WELCOME,
    READY_WAITING,
    START_TONE_PLAYER,
    PLAYING_TONE,
    INHIBITED,
    DESACTIVATED
};

// Add state machine variables
AppState currentState = STARTING;
unsigned long stateStartTime = 0;
bool stateInitialized = false;

// Add after other state machine variables
unsigned long lastToneUpdateTime = 0;
const unsigned long TONE_UPDATE_INTERVAL = 1000; // 1 second interval

// Add state machine variables after other defines
#define STARTING_DURATION 10000 // 30 seconds for starting state
#define MAX_PLAY_DURATION 20000 // 30 seconds for starting state
#define INHIBIT_DURATION 10000  // 10 seconds for inhibit state

// Add after other global variables
//unsigned long lastWifiCheckTime = 0;
//const unsigned long WIFI_CHECK_INTERVAL = 5000; // Check every 5 seconds



/**
 * @brief Waits for an event to occur before transitioning to the next state.
 *
 * @param currentEvent The current application state.
 * @param condition Function pointer to a function that returns true when the transition should occur.
 * @param nextEvent The next application state to transition to.
 */
AppState waitEvent(AppState currentEvent, std::function<bool()> condition, AppState nextEvent) {
    if (condition()) {
        // wait event is met, transition to next state
        Serial.printf("Event poped: Transitioning from %d to %d\n", currentEvent, nextEvent);
        return nextEvent;
    }
    return currentEvent;
}



void setup()
{
    Serial.begin(115200);
    Serial.println("Starting ESP32 Balise Sonore...");
    Serial.printf("Version %s Compile time: %s %s\n", FIRMWARE_VERSION, __DATE__, __TIME__);
    Serial.println("Initializing components...");

     // Initialize the led Manager
    ledManager.begin(); // Initialisation du gestionnaire de LEDs
    ledManager.setGreen();
    Serial.println("LedManager initialized");
    // Initialize configuration
    config.begin(); 
    // Input handler initialization
    inputHandler.begin(); // Initialisation du gestionnaire de messages radio
    Serial.println("InputHandler initialized");
    // Switch on the tone player  
    tonePlayer.powerOn(); // Power on the player
    delay(2000); // Wait for the player to power on
    tonePlayer.begin(); // Initialisation du lecteur de tonalité
    Serial.println("TonePlayer powered on");

    #ifndef DISABLE_WIFI
    // Initialize WiFi
    WiFi.mode(WIFI_AP);
    bool success = WiFi.softAP(config.getWifiSSID(), config.getWifiPassword());

    // Initialize WebServerManager
    webServer = new WebServerManager(config);
    webServer->begin();
    #endif


    

   
}

void loop()
{
    #ifndef DISABLE_WIFI
    // Add at the beginning of the loop function
    webServer->handleClient();
    #endif


    // Monitor heap memory
    //Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());

    // State machine
    switch (currentState)
    {

        case STARTING:
            Serial.println("State: STARTING");
            currentState = START_TONE_PLAYER_1; // Transition to START_TONE_PLAYER state
            delay(15000); // Wait for 1.5 seconds before transitioning
            stateInitialized = false; // Reset state initialization flag
            break;

        case START_TONE_PLAYER_1:
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
            Serial.println("State: START_TONE_PLAYER_1");
            Serial.println("reset...");
            tonePlayer.reset(); // Reset the tone player
            Serial.println("Wait...");
            Serial.println("Expecting player not busy...");
            stateInitialized = true;
            timer.armTimer(STARTING_DURATION); // Set timer for 30 seconds
        }
        
        // wait for tone player not to be busy
        //currentState = waitEvent(currentState, [&]() { return ! tonePlayer.busy(); }, START_TONE_PLAYER_2);
        // wait for tone player not to be busy
        currentState = waitEvent(currentState, [&]() { return timer.checkTimer(); }, START_TONE_PLAYER_2);

        if (currentState != START_TONE_PLAYER_1) {stateInitialized = false;}
        /*
            //starting the tone player
            //tonePlayer.powerOn(); // Power on the player
            //delay(2000); // Wait for the player to power on
            //tonePlayer.begin(); // Initialisation du lecteur de tonalité
            tonePlayer.startup(); // Initialisation du lecteur de tonalité

            Serial.println("TonePlayer initialized completed");
            currentState = WELCOME;
            */
        
    break;

    case START_TONE_PLAYER_2:
        if (!stateInitialized)
        {
            Serial.println("State: START_TONE_PLAYER_2");
            tonePlayer.readMessage();
            tonePlayer.adjustVolume(config.getVolume());  // Use volume from config
            Serial.printf("Volume set to %d \n", config.getVolume());
            //Serial.printf("Volume read from DFPlayer: %d\n", tonePlayer.readVolume());
            //tonePlayer.enableDAC();
            Serial.println(F("Player initialized"));
        }

        currentState = WELCOME;
        stateInitialized = false;

        break;

    case WELCOME:
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
            Serial.println("State: WELCOME");
            tonePlayer.playTone(4);
            ledManager.setYellow();
            stateInitialized = true;
            timer.armTimer(MAX_PLAY_DURATION); // Set timer for 30 seconds
        }

        // manage events
        // wait for tone player to finish
        currentState = waitEvent(currentState, [&]() { return ! tonePlayer.busy(); }, READY_WAITING);
        // watch dog timer
        currentState = waitEvent(currentState, [&]() { return timer.checkTimer(); }, READY_WAITING);
        
        if (currentState != WELCOME) {stateInitialized = false;}
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
        // manage events
        currentState = waitEvent(currentState, [&]() { return inputHandler.isActivated(); }, PLAYING_TONE);

        if (currentState != READY_WAITING) {stateInitialized = false;}
        break;

    case START_TONE_PLAYER:
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
            Serial.println("State: START_TONE_PLAYER");
            
            ledManager.setGreenYellow(); // Turn off LEDs after the wait period
            // tonePlayer.begin(); // Initialisation du lecteur de tonalité
            Serial.println("TonePlayer initialized");
            stateInitialized = true;
        }

        // wait for tone player not to be busy
        currentState = waitEvent(currentState, [&]() { return ! tonePlayer.busy(); }, PLAYING_TONE);
        // wait for tone player not to be busy
        currentState = waitEvent(currentState, [&]() { return  tonePlayer.busy(); }, PLAYING_TONE);

        if (currentState != START_TONE_PLAYER) {stateInitialized = false;}
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
            
            timer.armTimer(MAX_PLAY_DURATION); // Set timer for 20 seconds
            stateInitialized = true;
        }

        // manage events
        // wait for tone player to finish
        currentState = waitEvent(currentState, [&]() { return ! tonePlayer.busy(); }, INHIBITED);
        // watch dog timer
        currentState = waitEvent(currentState, [&]() { return timer.checkTimer(); }, INHIBITED);
        
        if (currentState != PLAYING_TONE) {stateInitialized = false;}
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
            //shutdown the tone player
            //tonePlayer.powerOff(); // Power off the player
            stateInitialized = true;
            timer.armTimer(INHIBIT_DURATION); // Set timer for 10 seconds
        }
        
        // wait timer
        currentState = waitEvent(currentState, [&]() { return timer.checkTimer(); }, READY_WAITING);

        if (currentState != INHIBITED) {inputHandler.resetActivation();stateInitialized = false;}
        break;

    case DESACTIVATED:
        break;
    }

    delay(5); // Prevent watchdog reset
}
