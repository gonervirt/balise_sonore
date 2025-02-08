#include "PushButtonManager.h"

// Constructeur de la classe PushButtonManager
PushButtonManager::PushButtonManager(uint8_t pin)
    : pin(pin), buttonPressed(false), buttonLocked(false) {}

// Initialisation de la broche du bouton poussoir
void PushButtonManager::begin() {
    pinMode(pin, INPUT_PULLUP);
}

// Mise à jour de l'état du bouton poussoir
void PushButtonManager::update() {
    bool buttonState = digitalRead(pin) == LOW;
    if (!buttonState && !buttonLocked) {
        buttonPressed = true;
        buttonLocked = true;
        Serial.println("..........Detected button press");
    } 
}

// Vérifie si le bouton est pressé
bool PushButtonManager::isButtonPressed() const {
    return buttonPressed;
}

// Réinitialise l'état du bouton poussoir lorsque la tonalité est terminée
void PushButtonManager::onToneFinished() {
    buttonPressed = false;
    buttonLocked = false;
    Serial.println("..........onToneFinished called");
}


// Réinitialise l'état du bouton poussoir lorsque la tonalité est terminée
void PushButtonManager::releaseButtonPressed() {
    buttonPressed = false;
    Serial.println("release button pressed");
}
