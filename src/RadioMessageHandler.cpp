#include "RadioMessageHandler.h"

// Define static class members
constexpr float RadioMessageHandler::PATTERN_TIMINGS[];

volatile int RadioMessageHandler::compteur = 0;
volatile unsigned long RadioMessageHandler::previousMicros = 0;
volatile unsigned long RadioMessageHandler::memoMicros = 0;
volatile int RadioMessageHandler::MyInts[100] = {0};
volatile bool RadioMessageHandler::interruptionActive = true;
volatile uint32_t RadioMessageHandler::lastInterruptTime = 0;
volatile int RadioMessageHandler::lastCompteur = 0;
volatile bool RadioMessageHandler::overflowOccurred = false;
volatile int RadioMessageHandler::head = 0;
volatile int RadioMessageHandler::tail = 0;

RadioMessageHandler::RadioMessageHandler(int pin) : radioPin(pin), status(WAITING_MSG) {
    currentMessage.command = INVALID_COMMAND;
    currentMessage.isValid = false;
    currentMessage.repeatCount = 0;
}

void RadioMessageHandler::begin() {
    pinMode(radioPin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(radioPin), onInterrupt, CHANGE);
}

void IRAM_ATTR RadioMessageHandler::onInterrupt() {
    if (!interruptionActive) return;

    unsigned long currentMicros = micros();
    int largeur = currentMicros - memoMicros;
    
    // Store timing only if reasonable
    if (largeur <= 50000) {  // Basic sanity check
        MyInts[head] = largeur;
        head = incrementIndex(head);
        // No need to check for overflow anymore
        compteur++;
    }
    
    memoMicros = currentMicros;
    lastInterruptTime = currentMicros;
    lastCompteur = compteur;
}

void RadioMessageHandler::processMessages() {
    int bufferSize = getBufferSize();
    
    if (bufferSize > 60) {  // Enough data to process
        interruptionActive = false;
        decodeMessage();
        if (status == MSG_READY) {
            Serial.println("Message successfully decoded:");
            printDebugInfo();
        }
        // Clear processed data by moving tail
        tail = head;  // Reset buffer after processing
        compteur = 0;
        interruptionActive = true;
    }
}

bool RadioMessageHandler::isMessageReady() const {
    return status == MSG_READY;
}

bool RadioMessageHandler::validateTiming(unsigned long timing, unsigned long min, unsigned long max) const {
    return timing >= min && timing <= max;
}

bool RadioMessageHandler::isSync(unsigned long timing) const {
    return validateTiming(timing, SYNC_MIN, SYNC_MAX);
}

bool RadioMessageHandler::isBit1(unsigned long timing) const {
    return validateTiming(timing, BIT_1_MIN, BIT_1_MAX);
}

bool RadioMessageHandler::isBit0(unsigned long timing) const {
    return validateTiming(timing, BIT_0_MIN, BIT_0_MAX);
}

bool RadioMessageHandler::isShortPulse(unsigned long timing) const {
    return validateTiming(timing, SHORT_MIN, SHORT_MAX);
}

bool RadioMessageHandler::matchTiming(unsigned long timing, float expected) const {
    return timing >= (expected * ERROR_RATE_MIN) && 
           timing <= (expected * ERROR_RATE_MAX);
}

bool RadioMessageHandler::matchPattern(int startIndex) const {
    int currentIndex = startIndex;
    
    for (float expectedTiming : PATTERN_TIMINGS) {
        if (currentIndex == head) return false;
        
        if (!matchTiming(MyInts[currentIndex], expectedTiming)) {
            return false;
        }
        currentIndex = incrementIndex(currentIndex);
    }
    return true;
}

void RadioMessageHandler::decodeMessage() {
    int bufferSize = getBufferSize();
    if (bufferSize < 46) {  // Need at least one complete pattern
        status = MSG_ERROR;
        return;
    }

    // Look for complete pattern
    int currentIndex = tail;
    bool patternFound = false;
    
    for (int i = 0; i < bufferSize - 46; i++) {
        if (matchTiming(MyInts[currentIndex], SYNC_TIME)) {
            if (matchPattern(currentIndex)) {
                patternFound = true;
                Serial.println("Complete NFS32-002 pattern found!");
                break;
            }
        }
        currentIndex = incrementIndex(currentIndex);
    }

    if (!patternFound) {
        status = MSG_ERROR;
        return;
    }

    // ... rest of decoding logic ...
}

void RadioMessageHandler::printDebugInfo() {
    Serial.println("Debug Info:");
    Serial.printf("Buffer: head=%d, tail=%d, size=%d\n", head, tail, getBufferSize());
    
    // Print last few timing values
    int current = tail;
    int count = 0;
    while (current != head && count < 5) {
        Serial.printf("MyInts[%d] = %d\n", current, MyInts[current]);
        current = incrementIndex(current);
        count++;
    }
}
