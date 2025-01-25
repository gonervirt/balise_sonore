#ifndef TONEPLAYER_H
#define TONEPLAYER_H

#include "DFRobotDFPlayerMini.h"

class TonePlayer {
public:
    TonePlayer(int rxd2, int txd2);
    void begin();
    void playTone(int messageNumber);
    void adjustVolume(int volume);
    void adjustInhibitDuration(double duration);
    bool isPlaying();
    bool isInhibited();
    void update();

private:
    DFRobotDFPlayerMini myMP3player;
    int rxd2;
    int txd2;
    int volume;
    double inhibitDuration;
    bool playing;
    bool inhibited;
    unsigned long inhibitStartTime;
};

#endif // TONEPLAYER_H
