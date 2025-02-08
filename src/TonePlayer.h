#ifndef TONEPLAYER_H
#define TONEPLAYER_H

#include "Arduino.h"
#include <DFRobotDFPlayerMini.h>
#include <vector>
#include <SoftwareSerial.h>

class TonePlayerListener {
public:
    virtual void onToneFinished() = 0;
};

class TonePlayer {
public:
    TonePlayer(int rxd2, int txd2);
    ~TonePlayer();  // Add destructor declaration
    void begin();
    void playTone(int messageNumber);
    void adjustVolume(int volume);
    void adjustInhibitDuration(double duration);
    bool isPlaying();
    bool isInhibited();
    void update();
    void reset();
    void addListener(TonePlayerListener* listener);

private:
    DFRobotDFPlayerMini myMP3player;
    int rxd2;
    int txd2;
    int volume;
    double inhibitDuration;
    bool playing;
    bool inhibited;
    unsigned long inhibitStartTime;
    std::vector<TonePlayerListener*> listeners;
    SoftwareSerial* serial2player;  // Add this line
    
    void notifyListeners();
};

#endif // TONEPLAYER_H
