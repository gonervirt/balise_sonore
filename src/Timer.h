#ifndef TIMER_H
#define TIMER_H

#include <Arduino.h>

class Timer {
public:
    Timer();
    void armTimer(unsigned long duration);
    bool checkTimer();
    void resetTimer();

private:
    unsigned long startTime;
    unsigned long duration;
    bool timerArmed;
};

#endif