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

const char *ssid = "BALISESONORE";
const char *password = "BaliseSonore_Betton_Mairie";
IPAddress local_ip(192, 168, 10, 1);
IPAddress gateway(192, 168, 10, 1);
IPAddress subnet(255, 255, 255, 0);

#define RXD2 16  // RX2 pour communiquer avec MP3
#define TXD2 17  // TX2 pour communiquer avec MP3
#define BUTTON_PIN 0 // Define the pin for the push button
#define GREEN_LED_PIN 2
#define YELLOW_LED_PIN 4
#define RED_LED_PIN 5

Config config;
TonePlayer tonePlayer(RXD2, TXD2);
WebServerManager webServerManager(tonePlayer, config);
RadioMessageHandler radioMessageHandler;
PushButtonManager pushButtonManager(BUTTON_PIN);
LedManager ledManager(GREEN_LED_PIN, YELLOW_LED_PIN, RED_LED_PIN);

void setup() {
    pinMode(2, OUTPUT);
    Serial.begin(115200);
    config.begin();
    tonePlayer.begin();
    webServerManager.begin();
    radioMessageHandler.begin();
    pushButtonManager.begin();
    ledManager.begin();

    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(local_ip, gateway, subnet);
    WiFi.softAP(ssid, password);
    Serial.println("AP IP address: " + WiFi.softAPIP().toString());
}

void loop() {
    webServerManager.handleClient();
    radioMessageHandler.processMessages();
    pushButtonManager.update();

    if (!tonePlayer.isPlaying() && !tonePlayer.isInhibited()) {
        if (radioMessageHandler.getCurrentMessage() > 0) {
            tonePlayer.playTone(config.getNumeroMessage());
            ledManager.setYellow();
        } else if (pushButtonManager.isButtonPressed()) {
            tonePlayer.playTone(config.getNumeroMessage()); // Play tone when button is pressed
            ledManager.setYellow();
        } else {
            ledManager.setGreen();
        }
    } else {
        ledManager.setRed();
    }

    tonePlayer.update();
}
