#ifndef RADIOMESSAGEHANDLER_H
#define RADIOMESSAGEHANDLER_H

#include "Arduino.h"
#include "TonePlayer.h"
//#include <RCSwitch.h>
//https://github.com/sui77/rc-switch/blob/master/examples/ReceiveDemo_Simple/ReceiveDemo_Simple.ino

class RadioMessageHandler : public TonePlayerListener {
public:
    RadioMessageHandler();
    void begin();
    void handleInterrupt();
    void processMessages();
    int getCurrentMessage() const;
    bool isMessageReady() const;
    void onToneFinished() override;

    static const int WAITING_MSG = 0;
    static const int RECEIVING_MSG = 1;
    static const int MSG_READY = 2;

private:
    static void IRAM_ATTR onInterrupt();
    void decodeMessage();
    static volatile int compteur;
    static volatile unsigned long previousMicros;
    static volatile unsigned long memoMicros;
    static volatile int MyInts[100];
    static volatile bool interruptionActive;
    int currentMessage;
    int status;
};

#endif // RADIOMESSAGEHANDLER_H
