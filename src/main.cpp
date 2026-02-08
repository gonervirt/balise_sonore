/**
 * @file main.cpp
 * ESP32 Balise Sonore
 *
 * Main program file that initializes and manages:
 * - Configuration system
 * - WiFi connection
 * - Web interface
 *
 * The device can operate in either Access Point or Station mode
 * and provides a web interface for configuration.
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

//#define DISABLE_WIFI 0 


#ifndef DISABLE_WIFI
#include "wifi_manager.h"
#include "web_server_manager.h"
#include "esp_wifi.h"  // Include for power management configuration
#include <DNSServer.h>
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
#ifndef DISABLE_WIFI
WiFiManager wifiManager(config);
WebServerManager *webServer;
Timer wifiLiveDurationTimer; // Timer for periodic WiFi checks
DNSServer dnsServer;
const unsigned long WIFI_LIVE_DURATION = 120000; // 2 minutes 
#endif

// Add state machine enum
enum AppState
{
    STARTING,
    HOT_RESTART,
    WELCOME_MESSAGE,
    READY_WAITING,
    PLAYING_TONE,
    INHIBITED,
    DESACTIVATED
};

// Add helper to convert AppState to string
const char* appStateToString(AppState s) {
    switch (s) {
        case STARTING: return "STARTING";
        case HOT_RESTART: return "HOT_RESTART";
        case WELCOME_MESSAGE: return "WELCOME_MESSAGE";
        case READY_WAITING: return "READY_WAITING";
        case PLAYING_TONE: return "PLAYING_TONE";
        case INHIBITED: return "INHIBITED";
        case DESACTIVATED: return "DESACTIVATED";
        default: return "UNKNOWN";
    }
}

// Add state machine variables
AppState currentState = STARTING;
unsigned long stateStartTime = 0;
bool stateInitialized = false;

// Add after other state machine variables
unsigned long lastToneUpdateTime = 0;
const unsigned long TONE_UPDATE_INTERVAL = 1000; // 1 second interval

//AppState targetState; // Initialize next state

// Add state machine variables after other defines
#define STARTING_DURATION 10000 // 30 seconds for starting state
#define MAX_PLAY_DURATION 20000 // 30 seconds for starting state
#define INHIBIT_DURATION 10000  // 10 seconds for inhibit state
#define CHECK_ALIVE_TIMER 120000 //7200000 // 2 hours for check alive state


// Returns "[YYYY-MM-DD HH:MM:SS.mmm] "
String timestamp() {
    struct timeval tv; gettimeofday(&tv, nullptr);
    struct tm t; localtime_r(&tv.tv_sec, &t);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &t);
    int ms = tv.tv_usec / 1000;
    char out[40];
    snprintf(out, sizeof(out), "[%s.%03d] ", buf, ms);
    return String(out);
}

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
        Serial.print(timestamp());
        Serial.printf("Event popped: Transitioning from %s to %s\n",
                      appStateToString(currentEvent),
                      appStateToString(nextEvent));
        return nextEvent;
    }
    return currentEvent;
}





/**
 * @brief Standard Arduino setup function
 * Initializes serial, LEDs, config, input handlers, tone player, and WiFi/Webserver.
 */
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
    


    #ifndef DISABLE_WIFI
    // Initialize WiFi
    wifiManager.begin();
    Serial.println("WiFi initialized");
   
    // Initialize WebServerManager
    Serial.println("Start webServer");
    delay(1000); // Wait a moment before starting the web server
    webServer = new WebServerManager(config);
    webServer->begin();
    Serial.println("webServer started");
    if (config.isAccessPoint()) {
        dnsServer.start(53, "*", WiFi.softAPIP());
        Serial.println("DNS Server started for Captive Portal");
    }
    #endif


    // Input handler initialization
    inputHandler.begin(); // Initialisation du gestionnaire de messages radio
    Serial.println("InputHandler initialized");



    // Switch on the tone player  
    tonePlayer.powerOn(); // Power on the player
    delay(3000); // Wait for the player to power on
    tonePlayer.begin(); // Initialisation du lecteur de tonalité
    Serial.println("TonePlayer powered on");
   
}

/**
 * @brief Standard Arduino loop function
 * Handles WiFi/Webserver tasks and executes the main application state machine.
 * The state machine manages the flow from startup -> welcome -> waiting -> playing -> inhibited.
 */
void loop()
{
    #ifndef DISABLE_WIFI
    // Add at the beginning of the loop function
    if (wifiManager.isAlive()) {
        if (config.isAccessPoint()) {
            dnsServer.processNextRequest();
        }
        if (webServer->handleClient()){
            // a web client was handled, reset timer before switch off wifi
            // reset WiFi live duration timer
            Serial.print("Web page handled, extending WiFi live duration\n");
            wifiLiveDurationTimer.armTimer(WIFI_LIVE_DURATION);
        };
    }
    #endif


    // State machine
    switch (currentState)
    {

        // STARTING State:
        // Initial state after boot.
        // Transitions to WELCOME_MESSAGE after a delay and player initialization.
        case STARTING:
            Serial.print(timestamp());
            Serial.println("State: STARTING");
            currentState = WELCOME_MESSAGE; // Transition to START_TONE_PLAYER state
            #ifdef BOARD_LOLIN_C3_MINI
            tonePlayer.enableDAC();
            #endif
            stateInitialized = false; // Reset state initialization flag
            #ifndef DISABLE_WIFI
            // reset WiFi live duration timer
            wifiLiveDurationTimer.armTimer(WIFI_LIVE_DURATION);
            #endif
            // next state
            currentState = WELCOME_MESSAGE; // Transition to WELCOME_MESSAGE state
            break;

     

    // HOT_RESTART State:
    // Re-initializes the player and WiFi if needed without a full system reboot.
    // Waits for the player to be available or a timeout before transitioning.
    case HOT_RESTART:
        /* Entry actions:
         * - Play WELCOME_MESSAGE message (tone 3)
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
            Serial.print(timestamp());
            Serial.println("State: HOT_RESTART");
            tonePlayer.powerOn(); // Reset the tone player
            tonePlayer.readMessage(); // Reset the tone player
            stateInitialized = true;
            timer.armTimer(STARTING_DURATION); // Set timer for 30 seconds
            #ifndef DISABLE_WIFI
            // reset WiFi live duration timer
            wifiLiveDurationTimer.armTimer(WIFI_LIVE_DURATION);
            if (!wifiManager.isAlive()) {
                Serial.println("Wifi restarting Access Point...");
                wifiManager.startAP();
                if (config.isAccessPoint()) {
                    dnsServer.start(53, "*", WiFi.softAPIP());
                }
            } else {
                Serial.println("Wifi is alive, skipping start AP");
            }
            #endif


        }
        
        
        currentState = waitEvent(currentState, [&]() { return  tonePlayer.available(); }, PLAYING_TONE);

        // wait for tone player not to be busy
        currentState = waitEvent(currentState, [&]() { return timer.checkTimer(); }, PLAYING_TONE);

        // wait for tone player not to be busy
        //tonePlayer.reset(); // Reset the tone player
        //delay(500); // Wait for the player to power on

        if (currentState != HOT_RESTART) {stateInitialized = false;}
        
    break;

   
    // WELCOME_MESSAGE State:
    // Plays the welcome tone (message 4) to indicate the system is up.
    // Transitions to INHIBITED when playback finishes or times out.
    case WELCOME_MESSAGE:
        /* Entry actions:
         * - Play WELCOME_MESSAGE_MESSAGE message (tone 3)
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
            Serial.print(timestamp());
            Serial.println("State: WELCOME_MESSAGE");
            tonePlayer.playTone(4);
            ledManager.setYellow();
            stateInitialized = true;
            timer.armTimer(MAX_PLAY_DURATION); // Set timer for 30 seconds
        }

        // manage events
        // wait for tone player to finish
        currentState = waitEvent(currentState, [&]() { return ! tonePlayer.busy(); }, INHIBITED);
        // watch dog timer
        currentState = waitEvent(currentState, [&]() { return timer.checkTimer(); }, INHIBITED);
        
        if (currentState != WELCOME_MESSAGE) {stateInitialized = false;}
        break;

    // READY_WAITING State:
    // Idle state waiting for user input (button press or radio signal).
    // Transitions to HOT_RESTART (which leads to PLAYING_TONE) upon activation.
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
            Serial.print(timestamp());
            Serial.println("State: READY_WAITING");
            ledManager.setGreen();
            tonePlayer.readMessage(); // check if message still available
            stateInitialized = true;
        }
        // check if a input hanlder is ativated
        inputHandler.update();
        tonePlayer.update(); // update sound player volume if needed
        // if event is activated, go to state HOT_RESTART
        currentState = waitEvent(currentState, [&]() { return inputHandler.isActivated(); }, HOT_RESTART);
        // stop the wifi after WIFI_LIVE_DURATION
        if (wifiLiveDurationTimer.checkTimer()) {
            wifiManager.stopAP();
        }
        
        if (currentState != READY_WAITING) {stateInitialized = false;}
        
        break;


    // PLAYING_TONE State:
    // Plays the configured message number.
    // Transitions to INHIBITED when playback finishes or times out.
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
            Serial.print(timestamp());
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


    // INHIBITED State:
    // A refractory period after playing a tone where inputs are ignored.
    // Transitions back to READY_WAITING after the duration expires.
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
            Serial.print(timestamp());
            Serial.println("State: INHIBITED");
            ledManager.setGreenYellow();
            stateInitialized = true;
            timer.armTimer(INHIBIT_DURATION); // Set timer for 10 seconds
            //shutdown the tone player
            tonePlayer.powerOff(); // Power off the player
            tonePlayer.readMessage(); // Read the message from the player
            tonePlayer.readMessage(); // Read the message from the player
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
