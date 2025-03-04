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
    
    static const int BUFFER_SIZE = 100;  // Renamed from MAX_INTS

    const int radioPin;
    MessageStatus status;
    DecodedMessage currentMessage;

    static volatile int compteur;
    static volatile unsigned long previousMicros;
    static volatile unsigned long memoMicros;
    static volatile int MyInts[BUFFER_SIZE];  // Buffer stays the same size
    static volatile bool interruptionActive;

    // Add debug variables
    static volatile uint32_t lastInterruptTime;
    static volatile int lastCompteur;
    static volatile bool overflowOccurred;

    // Add head and tail index for circular buffer
    static volatile int head;
    static volatile int tail;

    bool validateTiming(unsigned long timing, unsigned long min, unsigned long max) const;
    bool isSync(unsigned long timing) const;
    bool isBit1(unsigned long timing) const;
    bool isBit0(unsigned long timing) const;
    bool isGap(unsigned long timing) const;
    void decodeMessage();

    static RadioMessageHandler* instance;
    static void IRAM_ATTR onInterrupt();

    // Helper methods for circular buffer
    static int incrementIndex(int index) {
        return (index + 1) % BUFFER_SIZE;
    }
    
    int getBufferSize() const {
        if (head == tail) {
            return 0;  // Buffer empty
        }
        return ((head + BUFFER_SIZE - tail) % BUFFER_SIZE);
    }

public:
    RadioMessageHandler(int pin);
    void begin();
    void processMessages();
    bool isMessageReady() const;
    DecodedMessage getLastMessage() const { return currentMessage; }
    void resetMessage() { 
        status = WAITING_MSG; 
        currentMessage.isValid = false;
        currentMessage.repeatCount = 0;
    }
    const DecodedMessage& getCurrentMessage() const { return currentMessage; }

    // Add debug accessor
    static bool getOverflowStatus() { return overflowOccurred; }
    static int getLastCompteur() { return lastCompteur; }
    static uint32_t getLastInterruptTime() { return lastInterruptTime; }

    // Add debug method declaration
    void printDebugInfo();
};
