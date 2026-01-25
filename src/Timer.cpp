/**
 * @file Timer.cpp
 * @brief Implementation of the Timer class
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
#include "Timer.h"

Timer::Timer() : startTime(0), duration(0), timerArmed(false) {}

/**
 * @brief Arms the timer with a specific duration
 * @param duration Duration in milliseconds
 */
void Timer::armTimer(unsigned long duration) {
    this->duration = duration;
    this->startTime = millis();
    this->timerArmed = true;
}

/**
 * @brief Checks if the timer has expired
 * @return true if the timer was armed and the duration has passed
 */
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

/**
 * @brief Resets the timer state to unarmed
 */
void Timer::resetTimer() {
    timerArmed = false;
    startTime = 0;
    duration = 0;
}