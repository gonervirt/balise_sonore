#include "RadioMessageHandler.h"

// Static member initializations
RadioMessageHandler* RadioMessageHandler::instance = nullptr;
volatile unsigned long RadioMessageHandler::intervals[BUFFER_SIZE] = {0};
constexpr float RadioMessageHandler::PATTERN_TIMINGS[];
volatile int RadioMessageHandler::bufferIndex = 0;

/**
 * @brief Constructor for RadioMessageHandler
 * @param pin The pin number to which the radio receiver is connected
 */
RadioMessageHandler::RadioMessageHandler(uint8_t pin) 
    : InputHandler(pin), status(WAITING_MSG), messageReceived(false) {
    instance = this;
    currentMessage.command = INVALID_COMMAND;
    currentMessage.isValid = false;
}

/**
 * @brief Initializes the radio message handler by setting up the pin mode and interrupt
 */
void RadioMessageHandler::begin() {
    pinMode(pin, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(pin), onInterrupt, CHANGE);
}

/**
 * @brief Interrupt Service Routine (ISR) for handling pin change interrupts
 */
void IRAM_ATTR RadioMessageHandler::onInterrupt() {
    if (instance) {
        static unsigned long lastTime = 0;
        unsigned long currentTime = micros();
        unsigned long interval = currentTime - lastTime;
        lastTime = currentTime;
        
        // Filter out unreasonable intervals
        if (interval >= 100 && interval <= 1000) {
            intervals[bufferIndex] = interval;
            bufferIndex = (bufferIndex + 1) % BUFFER_SIZE;
        }
    }
}

/**
 * @brief Matches the given timing with the expected timing within an error range
 * @param timing The actual timing to match
 * @param expected The expected timing
 * @return True if the timing matches within the error range, false otherwise
 */
bool RadioMessageHandler::matchTiming(unsigned long timing, float expected) const {
    float minTime = expected * ERROR_RATE_MIN;
    float maxTime = expected * ERROR_RATE_MAX;
    return timing >= minTime && timing <= maxTime;
}

/**
 * @brief Matches the pattern starting from the given index in the buffer
 * @param startIndex The starting index in the buffer
 * @return True if the complete pattern matches, false otherwise
 */
bool RadioMessageHandler::matchPattern(int startIndex) const {
    // First check for sync pulse
    if (!matchTiming(intervals[startIndex], SYNC_TIME)) {
        return false;
    }

    // Check the rest of the pattern
    const int patternLength = sizeof(PATTERN_TIMINGS)/sizeof(PATTERN_TIMINGS[0]);
    int matchCount = 1; // Start at 1 because we already matched the sync pulse
    
    for (int i = 1; i < patternLength; i++) {
        int idx = (startIndex + i) % BUFFER_SIZE;
        if (!matchTiming(intervals[idx], PATTERN_TIMINGS[i])) {
            return false;
        }
        matchCount++;
    }
    
    return true;
}

/**
 * @brief Updates the state by checking for valid patterns in the buffer
 */
void RadioMessageHandler::update() {
    if (messageReceived) {
        return;
    }
    
    // Look for valid patterns in the buffer
    for (int i = 0; i < BUFFER_SIZE; i++) {
        if (matchTiming(intervals[i], SYNC_TIME)) {  // Using matchTiming for SYNC detection
            if (matchPattern(i)) {
                Serial.println("Complete NFS32-002 pattern found!");
                messageReceived = true;
                currentMessage.command = ACTIVATE_SOUND;
                currentMessage.isValid = true;
                status = MSG_READY;
                return;
            }
        }
    }
}

/**
 * @brief Resets the activation state of the message handler
 */
void RadioMessageHandler::resetActivation() {
    messageReceived = false;
    status = WAITING_MSG;
    currentMessage.isValid = false;
    Serial.println("Message handler reset");
}
