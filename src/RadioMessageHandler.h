#ifndef RADIOMESSAGEHANDLER_H
#define RADIOMESSAGEHANDLER_H

#include "Arduino.h"

class RadioMessageHandler {
public:
    RadioMessageHandler();
    void begin();
    void handleInterrupt();
    void processMessages();
    int getCurrentMessage() const;

private:
    static void IRAM_ATTR onInterrupt();
    void decodeMessage();
    static volatile int compteur;
    static volatile unsigned long previousMicros;
    static volatile unsigned long memoMicros;
    static volatile int MyInts[100];
    static volatile bool interruptionActive;
    int currentMessage;
};

#endif // RADIOMESSAGEHANDLER_H
