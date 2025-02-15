#ifndef PUSHBUTTONMANAGER_H
#define PUSHBUTTONMANAGER_H

#include "Arduino.h"


// Classe pour gérer un bouton poussoir
class PushButtonManager  {
public:
    PushButtonManager(uint8_t pin);
    void begin();
    void update();
    bool isButtonPressed() const;
    void releaseButtonPressed();

private:
    uint8_t pin;
    bool buttonPressed;
    bool buttonLocked;
};

#endif // PUSHBUTTONMANAGER_H
