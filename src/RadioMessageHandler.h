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
    static constexpr float ERROR_RATE = 0.3f;
    static constexpr float ERROR_RATE_MIN = 0.7f;  // 1.0f - ERROR_RATE
    static constexpr float ERROR_RATE_MAX = 1.3f;  // 1.0f + ERROR_RATE
    
    // Base timings (microseconds)
    static constexpr int SYNC_TIME = 625;
    static constexpr int SYNC_MIN = 575;   // SYNC_TIME - 50
    static constexpr int SYNC_MAX = 675;   // SYNC_TIME + 50
    
    static constexpr int BIT_1_TIME = 500;
    static constexpr int BIT_1_MIN = 450;  // BIT_1_TIME - 50
    static constexpr int BIT_1_MAX = 550;  // BIT_1_TIME + 50
    
    static constexpr int BIT_0_TIME = 250;
    static constexpr int BIT_0_MIN = 200;  // BIT_0_TIME - 50
    static constexpr int BIT_0_MAX = 300;  // BIT_0_TIME + 50
    
    static constexpr int SHORT_TIME = 207;
    static constexpr int SHORT_MIN = 157;  // SHORT_TIME - 50
    static constexpr int SHORT_MAX = 257;  // SHORT_TIME + 50

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
    bool isShortPulse(unsigned long timing) const;  // Added declaration
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

    bool matchTiming(unsigned long timing, float expected) const;
    bool matchPattern(int startIndex) const;

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
