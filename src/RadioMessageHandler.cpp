#include "RadioMessageHandler.h"

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

bool RadioMessageHandler::isGap(unsigned long timing) const {
    return validateTiming(timing, GAP_MIN, GAP_MAX);
}

void RadioMessageHandler::decodeMessage() {
    int bufferSize = getBufferSize();
    if (bufferSize < 60) {
        status = MSG_ERROR;
        return;
    }

    // Look for sync pattern
    int syncIndex = -1;
    int currentIndex = tail;
    
    for (int i = 0; i < bufferSize - 1; i++) {
        if (isSync(MyInts[currentIndex])) {
            Serial.printf("Sync found at index %d with timing value %d us\n", i, MyInts[currentIndex]);
            syncIndex = i;
            break;
        }
        currentIndex = incrementIndex(currentIndex);
    }

    if (syncIndex == -1) {
        status = MSG_ERROR;
        return;
    }

    // Move to sync position
    for (int i = 0; i < syncIndex; i++) {
        tail = incrementIndex(tail);
    }

    // Decode the message after sync
    uint8_t bits[8] = {0};
    int bitCount = 0;
    bool lastBitValid = true;
    currentIndex = incrementIndex(tail);  // Start after sync

    while (bitCount < 8 && currentIndex != head) {
        if (isBit1(MyInts[currentIndex])) {
            bits[bitCount++] = 1;
        } else if (isBit0(MyInts[currentIndex])) {
            bits[bitCount++] = 0;
        } else if (!isGap(MyInts[currentIndex])) {
            lastBitValid = false;
            break;
        }
        currentIndex = incrementIndex(currentIndex);
    }

    if (!lastBitValid || bitCount != 8) {
        status = MSG_ERROR;
        return;
    }

    // Validate message format according to NFS32-002
    // First 4 bits are command, last 4 bits are inverse of first 4
    uint8_t command = (bits[0] << 3) | (bits[1] << 2) | (bits[2] << 1) | bits[3];
    uint8_t inverse = (bits[4] << 3) | (bits[5] << 2) | (bits[6] << 1) | bits[7];

    if ((command ^ inverse) != 0x0F) {
        status = MSG_ERROR;
        return;
    }

    // Update current message
    switch (command) {
        case 0x0A: // Example command for activation
            currentMessage.command = ACTIVATE_SOUND;
            break;
        case 0x05: // Example command for deactivation
            currentMessage.command = DEACTIVATE_SOUND;
            break;
        default:
            currentMessage.command = INVALID_COMMAND;
            break;
    }

    currentMessage.isValid = true;
    currentMessage.repeatCount++;
    status = MSG_READY;
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
