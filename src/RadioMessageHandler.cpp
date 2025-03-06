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
RadioMessageHandler::RadioMessageHandler(uint8_t pin) 
    : InputHandler(pin), status(WAITING_MSG), messageReceived(false) {
    currentMessage.command = INVALID_COMMAND;
    currentMessage.isValid = false;
    currentMessage.repeatCount = 0;
}

void RadioMessageHandler::begin() {
    pinMode(pin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(pin), onInterrupt, CHANGE);
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

void RadioMessageHandler::update() {  // Renamed from processMessages
    int bufferSize = getBufferSize();
    
    if (bufferSize > 46) {
        interruptionActive = false;
        decodeMessage();
        if (status == MSG_READY) {
            currentMessage.command = ACTIVATE_SOUND;
            currentMessage.isValid = true;
            currentMessage.repeatCount++;
            messageReceived = true;
        }
        interruptionActive = true;
    }
}

// Remove isMessageReady() and resetMessage() implementations
// They are replaced by base class methods isActivated() and resetActivation()

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
