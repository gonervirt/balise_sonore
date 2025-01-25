#include "RadioMessageHandler.h"

volatile int RadioMessageHandler::compteur = 0;
volatile unsigned long RadioMessageHandler::previousMicros = 0;
volatile unsigned long RadioMessageHandler::memoMicros = 0;
volatile int RadioMessageHandler::MyInts[100] = {0};
volatile bool RadioMessageHandler::interruptionActive = true;

RadioMessageHandler::RadioMessageHandler() : currentMessage(0), status(WAITING_MSG) {}

void RadioMessageHandler::begin() {
    pinMode(15, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(15), onInterrupt, CHANGE);
}

void RadioMessageHandler::handleInterrupt() {
    // Handle the interrupt
}

void RadioMessageHandler::processMessages() {
    // Process the received messages
    if (compteur > 60) {
        interruptionActive = false;
        compteur = 0;
        decodeMessage();
    }
}

int RadioMessageHandler::getCurrentMessage() const {
    return currentMessage;
}

void IRAM_ATTR RadioMessageHandler::onInterrupt() {
    if (interruptionActive) {
        compteur++;
        previousMicros = micros();
        int largeur = previousMicros - memoMicros;
        memoMicros = previousMicros;
        MyInts[compteur] = largeur;
    }
}

void RadioMessageHandler::decodeMessage() {
    // Decode the message and set currentMessage
    // Decode the message
    currentMessage = 1;  // decoded message number 
}

void RadioMessageHandler::onToneFinished() {
    status = WAITING_MSG;
}

