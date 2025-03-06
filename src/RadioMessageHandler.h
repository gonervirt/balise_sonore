#pragma once
#include <Arduino.h>
#include "InputHandler.h"

enum MessageStatus {
    WAITING_MSG,
    MSG_READY,
    MSG_ERROR
};

enum NFS32_002_Command {
    NO_COMMAND,
    ACTIVATE_SOUND,
    DEACTIVATE_SOUND,
    INVALID_COMMAND
};

struct DecodedMessage {
    NFS32_002_Command command;
    bool isValid;
    uint8_t repeatCount;
};

class RadioMessageHandler : public InputHandler {
private:
    static constexpr float ERROR_RATE = 0.3f;
    static constexpr float ERROR_RATE_MIN = 0.7f;  // 1.0f - ERROR_RATE
    static constexpr float ERROR_RATE_MAX = 1.3f;  // 1.0f + ERROR_RATE
    
    // Base timings (microseconds)
    static constexpr int SYNC_TIME = 625;

    // Complete timing sequence for NFS32-002
    static constexpr float PATTERN_TIMINGS[] = {
        625,    // Sync
        312.5f, 312.5f,  // Manchester bits
        207.5f, 207.5f,  // Manchester bits
        500.0f, 500.0f,  // Manchester bits
        250.0f, 250.0f, 250.0f, 250.0f,  // Manchester bits
        500.0f, 500.0f,  // Manchester bits
        250.0f, 250.0f, 250.0f, 250.0f,  // More Manchester bits
        250.0f, 250.0f, 250.0f, 250.0f,
        250.0f, 250.0f, 500.0f, 250.0f,
        250.0f, 500.0f, 250.0f, 250.0f,
        500.0f, 250.0f, 250.0f, 250.0f,
        250.0f, 250.0f, 250.0f, 250.0f,
        250.0f, 250.0f, 250.0f, 250.0f,
        250.0f, 250.0f, 250.0f, 250.0f
    };

    static const int BUFFER_SIZE = 100;

    MessageStatus status;
    DecodedMessage currentMessage;

    static volatile int compteur;
    static volatile unsigned long previousMicros;
    static volatile unsigned long memoMicros;
    static volatile int MyInts[BUFFER_SIZE];
    static volatile bool interruptionActive;
    static volatile int head;
    static volatile int tail;
    bool messageReceived;

    void decodeMessage();
    bool matchTiming(unsigned long timing, float expected) const;
    bool matchPattern(int startIndex) const;

    static RadioMessageHandler* instance;
    static void IRAM_ATTR onInterrupt();

    // Helper methods for circular buffer
    static int incrementIndex(int index) {
        return (index + 1) % BUFFER_SIZE;
    }
    
    int getBufferSize() const {
        return (head >= tail) ? (head - tail) : (BUFFER_SIZE - tail + head);
    }

public:
    explicit RadioMessageHandler(uint8_t pin);
    void begin() override;
    void update() override;
    void printDebugInfo();
    bool isActivated() const override { return messageReceived; }
    void resetActivation() override { 
        messageReceived = false; 
        status = WAITING_MSG;
        currentMessage.isValid = false;
        currentMessage.repeatCount = 0;
    }
};
