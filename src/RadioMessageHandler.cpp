#include "RadioMessageHandler.h"

volatile int RadioMessageHandler::compteur = 0;
volatile unsigned long RadioMessageHandler::previousMicros = 0;
volatile unsigned long RadioMessageHandler::memoMicros = 0;
volatile int RadioMessageHandler::MyInts[100] = {0};
volatile bool RadioMessageHandler::interruptionActive = true;
volatile uint32_t RadioMessageHandler::lastInterruptTime = 0;
volatile int RadioMessageHandler::lastCompteur = 0;
volatile bool RadioMessageHandler::overflowOccurred = false;

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
    // Keep only essential operations in interrupt
    if (!interruptionActive) return;  // Exit early if not active
    
    if (compteur >= MAX_INTS - 1) {
        interruptionActive = false;
        overflowOccurred = true;
        return;
    }

    unsigned long currentMicros = micros();
    int largeur = currentMicros - memoMicros;
    
    // Store timing only if reasonable
    if (largeur <= 50000) {  // Basic sanity check
        compteur++;
        MyInts[compteur] = largeur;
    }
    
    memoMicros = currentMicros;
    lastInterruptTime = currentMicros;
    lastCompteur = compteur;
}

// Add a new method to check interrupt status periodically
void RadioMessageHandler::processMessages() {
    if (overflowOccurred) {
        Serial.println("Buffer overflow detected!");
        printDebugInfo();
        overflowOccurred = false;  // Reset the flag
    }

    if (compteur > 60) {
        interruptionActive = false;
        Serial.println("Processing message...");
        printDebugInfo();
        decodeMessage();
        compteur = 0;  // Reset counter
        interruptionActive = true;  // Re-enable interrupts
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
    if (compteur < 60) {  // Not enough data
        status = MSG_ERROR;
        return;
    }

    // Look for sync pattern
    int syncIndex = -1;
    for (int i = 1; i < compteur - 1; i++) {
        if (isSync(MyInts[i])) {
            syncIndex = i;
            break;
        }
    }

    if (syncIndex == -1) {
        status = MSG_ERROR;
        return;
    }

    // Decode the message after sync
    uint8_t bits[8] = {0};
    int bitCount = 0;
    bool lastBitValid = true;

    for (int i = syncIndex + 1; i < compteur && bitCount < 8; i++) {
        if (isBit1(MyInts[i])) {
            bits[bitCount++] = 1;
        } else if (isBit0(MyInts[i])) {
            bits[bitCount++] = 0;
        } else if (!isGap(MyInts[i])) {
            lastBitValid = false;
            break;
        }
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
    Serial.printf("Last interrupt time: %lu\n", lastInterruptTime);
    Serial.printf("Last compteur value: %d\n", lastCompteur);
    Serial.printf("Overflow occurred: %s\n", overflowOccurred ? "Yes" : "No");
    
    // Print last few timing values if available
    int start = max(0, lastCompteur - 5);
    for (int i = start; i <= lastCompteur; i++) {
        Serial.printf("MyInts[%d] = %d\n", i, MyInts[i]);
    }
}
