/**
 * @file LedManager.cpp
 * @brief Implementation of LedManager for controlling status LEDs
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
#include "LedManager.h"

// Constructeur de la classe LedManager
LedManager::LedManager(uint8_t greenPin, uint8_t yellowPin, uint8_t redPin)
    : greenPin(greenPin), yellowPin(yellowPin), redPin(redPin){}

/**
 * @brief Initializes LED pins and runs a startup sequence
 * Configures pins as outputs and cycles through colors.
 */
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

/**
 * @brief Sets the status to Green (only green LED on)
 */
void LedManager::setGreen() {
    off();
    setLed(greenPin);
    //Serial.println("LED set to green");
}

/**
 * @brief Sets the status to Yellow (only yellow LED on)
 */
void LedManager::setYellow() {
    off();
    setLed(yellowPin);
    //Serial.println("LED set to yellow");
}

/**
 * @brief Sets the status to Red (only red LED on)
 */
void LedManager::setRed() {
    off();
    setLed(redPin);
    //Serial.println("LED set to red");
}

/**
 * @brief Sets the status to Green and Yellow (both LEDs on)
 */
void LedManager::setGreenYellow() {
    off();
    setLed(yellowPin);
    setLed(greenPin);
    //Serial.println("LED set to red");
}

/**
 * @brief Turns off all LEDs
 */
void LedManager::off() {
    digitalWrite(greenPin, LOW);
    digitalWrite(yellowPin, LOW);
    digitalWrite(redPin, LOW);
}

/**
 * @brief Helper to turn on a specific LED pin
 */
void LedManager::setLed(uint8_t pin) {
    digitalWrite(pin, HIGH);
}
