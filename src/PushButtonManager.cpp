#include "PushButtonManager.h"

PushButtonManager::PushButtonManager(uint8_t pin)
    : pin(pin), lastButtonState(false), buttonPressed(false) {}

void PushButtonManager::begin() {
    pinMode(pin, INPUT_PULLUP);
}

void PushButtonManager::update() {
    bool buttonState = digitalRead(pin) == LOW;
    if (buttonState && !lastButtonState) {
        buttonPressed = true;
    } else {
        buttonPressed = false;
    }
    lastButtonState = buttonState;
}

bool PushButtonManager::isButtonPressed() const {
    return buttonPressed;
}
