/**
 * @file PushButtonManager.cpp
 * @brief Implementation of PushButtonManager for handling physical button inputs
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
#include "PushButtonManager.h"

PushButtonManager::PushButtonManager(uint8_t pin) 
    : InputHandler(pin), buttonLocked(false) {
}

void PushButtonManager::begin() {
    pinMode(pin, INPUT_PULLUP);
}

/**
 * @brief Updates the button state. Locks the button as 'activated' if pressed.
 */
void PushButtonManager::update() {
    if (!buttonLocked && digitalRead(pin)) {
        buttonLocked = true;
    }

}

/**
 * @brief Resets the button lock state, allowing new activations.
 */
void PushButtonManager::resetActivation() {
    buttonLocked = false;  // Reset button lock state
}

bool PushButtonManager::isActivated() const {
    return buttonLocked;
}
