#pragma once
#include <Arduino.h>

class InputHandler {
protected:
    const uint8_t pin;

public:
    InputHandler(uint8_t _pin) : pin(_pin) {}
    virtual ~InputHandler() = default;

    virtual void begin() = 0;
    virtual void update() = 0;
    virtual bool isActivated() const = 0;  // Made pure virtual
    virtual void resetActivation() = 0;    // Made pure virtual
};
