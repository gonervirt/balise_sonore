#include "Timer.h"

Timer::Timer() : startTime(0), duration(0), timerArmed(false) {}

void Timer::armTimer(unsigned long duration) {
    this->duration = duration;
    this->startTime = millis();
    this->timerArmed = true;
}

bool Timer::checkTimer() {
    if (!timerArmed) {
        return false;
    }
    if (millis() - startTime >= duration) {
        Serial.println("Timer expired");
        resetTimer();
        return true;
    }
    //Serial.println ("Timer still running");
    //Serial.printf("Time elapsed: %lu ms, Duration: %lu ms\n", millis() - startTime, duration);
    return false;
}

void Timer::resetTimer() {
    timerArmed = false;
    startTime = 0;
    duration = 0;
}