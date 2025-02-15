#pragma once
#include <Arduino.h>

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

class RadioMessageHandler {
private:
    // Timing constants (microseconds) according to NFS32-002
    static const unsigned long SYNC_MIN = 4800;
    static const unsigned long SYNC_MAX = 5200;
    static const unsigned long BIT_1_MIN = 1800;
    static const unsigned long BIT_1_MAX = 2200;
    static const unsigned long BIT_0_MIN = 800;
    static const unsigned long BIT_0_MAX = 1200;
    static const unsigned long GAP_MIN = 500;
    static const unsigned long GAP_MAX = 700;
    
    const int radioPin;
    MessageStatus status;
    DecodedMessage currentMessage;

    static volatile int compteur;
    static volatile unsigned long previousMicros;
    static volatile unsigned long memoMicros;
    static volatile int MyInts[100];
    static volatile bool interruptionActive;

    bool validateTiming(unsigned long timing, unsigned long min, unsigned long max) const;
    bool isSync(unsigned long timing) const;
    bool isBit1(unsigned long timing) const;
    bool isBit0(unsigned long timing) const;
    bool isGap(unsigned long timing) const;
    void decodeMessage();

public:
    RadioMessageHandler(int pin);
    void begin();
    void processMessages();
    bool isMessageReady() const;
    DecodedMessage getLastMessage() const { return currentMessage; }
    static void IRAM_ATTR onInterrupt();
};
