#ifndef PUSHBUTTONMANAGER_H
#define PUSHBUTTONMANAGER_H

#include "Arduino.h"

class PushButtonManager {
public:
    PushButtonManager(uint8_t pin);
    void begin();
    void update();
    bool isButtonPressed() const;

private:
    uint8_t pin;
    bool lastButtonState;
    bool buttonPressed;
};

#endif // PUSHBUTTONMANAGER_H
