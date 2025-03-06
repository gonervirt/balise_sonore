#include "RadioMessageHandler.h"

// Remove unused static member initializations
constexpr float RadioMessageHandler::PATTERN_TIMINGS[];
volatile int RadioMessageHandler::compteur = 0;
volatile unsigned long RadioMessageHandler::previousMicros = 0;
volatile unsigned long RadioMessageHandler::memoMicros = 0;
volatile int RadioMessageHandler::MyInts[100] = {0};
volatile bool RadioMessageHandler::interruptionActive = true;
volatile int RadioMessageHandler::head = 0;
volatile int RadioMessageHandler::tail = 0;

// Keep constructor, begin, and interrupt handler
RadioMessageHandler::RadioMessageHandler(int pin) : radioPin(pin), status(WAITING_MSG), messageProcessed(false) {
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
        compteur++;
    }
    
    memoMicros = currentMicros;
}

void RadioMessageHandler::decodeMessage() {
    int bufferSize = getBufferSize();
    if (bufferSize < 46) {
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

    status = MSG_READY;
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

void RadioMessageHandler::processMessages() {
    int bufferSize = getBufferSize();
    
    if (bufferSize > 46) {  // Enough data for one complete pattern
        interruptionActive = false;
        decodeMessage();
        if (status == MSG_READY) {
            currentMessage.command = ACTIVATE_SOUND;  // For now, assume any valid pattern is activation
            currentMessage.isValid = true;
            currentMessage.repeatCount++;
            messageProcessed = false;  // New message received, not yet processed
        }
        interruptionActive = true;
    }
}

bool RadioMessageHandler::isMessageReady() const {
    return !messageProcessed &&  // Only return true if message hasn't been processed
           status == MSG_READY && 
           currentMessage.isValid && 
           currentMessage.command == ACTIVATE_SOUND;
}

void RadioMessageHandler::resetMessage() {
    status = WAITING_MSG;
    currentMessage.isValid = false;
    currentMessage.repeatCount = 0;
    messageProcessed = false;
}

// Keep debug methods
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
