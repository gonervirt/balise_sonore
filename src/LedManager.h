#ifndef LEDMANAGER_H
#define LEDMANAGER_H

#include "Arduino.h"

// Classe pour gérer les LEDs de statut
class LedManager {
public:
    LedManager(uint8_t greenPin, uint8_t yellowPin, uint8_t redPin);
    void begin();
    void setGreen();
    void setYellow();
    void setYellowRed();
    void setRed();
    void off();

private:
    uint8_t greenPin;
    uint8_t yellowPin;
    uint8_t redPin;
    void setLed(uint8_t pin);
};

#endif // LEDMANAGER_H
