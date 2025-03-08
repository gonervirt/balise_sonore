#pragma once
#include "InputHandler.h"

class PushButtonManager : public InputHandler {
private:
    bool buttonLocked;

public:
    explicit PushButtonManager(uint8_t pin);
    void begin() override;
    void update() override;
    void resetActivation() override;  // Add override
    bool isActivated() const override { return buttonLocked; }
};
