#include "LedManager.h"

// Constructeur de la classe LedManager
LedManager::LedManager(uint8_t greenPin, uint8_t yellowPin, uint8_t redPin)
    : greenPin(greenPin), yellowPin(yellowPin), redPin(redPin) {}

// Initialisation des broches des LEDs
void LedManager::begin() {
    pinMode(greenPin, OUTPUT);
    pinMode(yellowPin, OUTPUT);
    pinMode(redPin, OUTPUT);
    off();
}

// Allume la LED verte
void LedManager::setGreen() {
    setLed(greenPin);
}

// Allume la LED jaune
void LedManager::setYellow() {
    setLed(yellowPin);
}

// Allume la LED rouge
void LedManager::setRed() {
    setLed(redPin);
}

// Éteint toutes les LEDs
void LedManager::off() {
    digitalWrite(greenPin, LOW);
    digitalWrite(yellowPin, LOW);
    digitalWrite(redPin, LOW);
}

// Allume la LED spécifiée et éteint les autres
void LedManager::setLed(uint8_t pin) {
    off();
    digitalWrite(pin, HIGH);
}
