#include "LedManager.h"

// Constructeur de la classe LedManager
LedManager::LedManager(uint8_t greenPin, uint8_t yellowPin, uint8_t redPin)
    : greenPin(greenPin), yellowPin(yellowPin), redPin(redPin){}

// Initialisation des broches des LEDs
void LedManager::begin() {
    pinMode(greenPin, OUTPUT);
    pinMode(yellowPin, OUTPUT);
    pinMode(redPin, OUTPUT);
    off();
    //test leds, start up sequence
    setGreen(); // LED verte pour indiquer le démarrage
    delay(100);
    setYellow(); // LED verte pour indiquer le démarrage
    delay(100);
    setRed(); // LED verte pour indiquer le démarrage
    delay(100);
    setYellow(); // LED verte pour indiquer le démarrage
    delay(100);
    setGreen(); // LED verte pour indiquer le démarrage
    delay(100);
}

// Allume la LED verte
void LedManager::setGreen() {
    off();
    setLed(greenPin);
    //Serial.println("LED set to green");
}

// Allume la LED jaune
void LedManager::setYellow() {
    off();
    setLed(yellowPin);
    //Serial.println("LED set to yellow");
}

// Allume la LED rouge
void LedManager::setRed() {
    off();
    setLed(redPin);
    //Serial.println("LED set to red");
}

// Allume la LED rouge
void LedManager::setGreenYellow() {
    off();
    setLed(yellowPin);
    setLed(greenPin);
    //Serial.println("LED set to red");
}

// Éteint toutes les LEDs
void LedManager::off() {
    digitalWrite(greenPin, LOW);
    digitalWrite(yellowPin, LOW);
    digitalWrite(redPin, LOW);
}

// Allume la LED spécifiée 
void LedManager::setLed(uint8_t pin) {
    digitalWrite(pin, HIGH);
}


