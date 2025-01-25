#include "LedManager.h"

LedManager::LedManager(uint8_t greenPin, uint8_t yellowPin, uint8_t redPin)
    : greenPin(greenPin), yellowPin(yellowPin), redPin(redPin) {}

void LedManager::begin() {
    pinMode(greenPin, OUTPUT);
    pinMode(yellowPin, OUTPUT);
    pinMode(redPin, OUTPUT);
    off();
}

void LedManager::setGreen() {
    setLed(greenPin);
}

void LedManager::setYellow() {
    setLed(yellowPin);
}

void LedManager::setRed() {
    setLed(redPin);
}

void LedManager::off() {
    digitalWrite(greenPin, LOW);
    digitalWrite(yellowPin, LOW);
    digitalWrite(redPin, LOW);
}

void LedManager::setLed(uint8_t pin) {
    off();
    digitalWrite(pin, HIGH);
}
