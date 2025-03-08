#include "PushButtonManager.h"

PushButtonManager::PushButtonManager(uint8_t pin) 
    : InputHandler(pin), buttonLocked(false) {
}

void PushButtonManager::begin() {
    pinMode(pin, INPUT_PULLUP);
}

void PushButtonManager::update() {
    if (!buttonLocked && digitalRead(pin)) {
        buttonLocked = true;
    }

}

void PushButtonManager::resetActivation() {
    buttonLocked = false;  // Reset button lock state
}

bool PushButtonManager::isActivated() const {
    return buttonLocked;
}
