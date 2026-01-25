/**
 * @file RadioMessageHandler.h
 * @brief Header file for RadioMessageHandler class
 * 
 * @copyright Copyright (c) 2024 ESP32 Balise Sonore Project
 * 
 * @license MIT License
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
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
    static constexpr float ERROR_RATE_MIN = 0.7f;
    static constexpr float ERROR_RATE_MAX = 1.3f;

    // Base timings (microseconds)
    static constexpr int SYNC_TIME = 625;
    
    // Define pattern timings directly in header
    static constexpr float PATTERN_TIMINGS[41] = {
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
        250.0f, 250.0f, 250.0f, 250.0f
    };

    static const int BUFFER_SIZE = 100;
    static volatile unsigned long intervals[BUFFER_SIZE];
    static volatile int bufferIndex;
    static RadioMessageHandler* instance;

    bool messageReceived;
    MessageStatus status;
    DecodedMessage currentMessage;

    static void IRAM_ATTR onInterrupt();
    bool matchTiming(unsigned long timing, float expected) const;
    bool matchPattern(int startIndex) const;

public:
    /**
     * @brief Constructor for RadioMessageHandler
     * @param pin The pin number to which the radio receiver is connected
     */
    explicit RadioMessageHandler(uint8_t pin);

    /**
     * @brief Initializes the radio message handler by setting up the pin mode and interrupt
     */
    void begin() override;

    /**
     * @brief Updates the state by checking for valid patterns in the buffer
     */
    void update() override;

    /**
     * @brief Checks if a message has been received and is valid
     * @return True if a valid message has been received, false otherwise
     */
    bool isActivated() const override { return messageReceived; }

    /**
     * @brief Resets the activation state of the message handler
     */
    void resetActivation() override;
};
