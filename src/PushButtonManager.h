#ifndef PUSHBUTTONMANAGER_H
#define PUSHBUTTONMANAGER_H

#include "Arduino.h"
#include "TonePlayer.h"

// Classe pour gérer un bouton poussoir
class PushButtonManager : public TonePlayerListener {
public:
    PushButtonManager(uint8_t pin);
    void begin();
    void update();
    bool isButtonPressed() const;
    void releaseButtonPressed();
    void onToneFinished() override;

private:
    uint8_t pin;
    bool buttonPressed;
    bool buttonLocked;
};

#endif // PUSHBUTTONMANAGER_H
