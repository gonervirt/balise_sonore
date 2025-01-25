#include "TonePlayer.h"
#include "Arduino.h"

TonePlayer::TonePlayer(int _rxd2, int _txd2) : volume(15), inhibitDuration(2000), playing(false), inhibited(false), inhibitStartTime(0) {
    rxd2 = _rxd2;
    txd2 = _txd2;
}

void TonePlayer::begin() {
    Serial1.begin(9600, SERIAL_8N1, rxd2, txd2);
    if (!myMP3player.begin(Serial1)) {
        Serial.println(F("Unable to begin:"));
        while (true) {
            delay(0);
        }
    }
    myMP3player.enableDAC();
    myMP3player.volume(volume);
}

void TonePlayer::playTone(int messageNumber) {
    myMP3player.play(messageNumber);
    playing = true;
}

void TonePlayer::adjustVolume(int volume) {
    this->volume = constrain(volume, 0, 30);
    myMP3player.volume(this->volume);
}

void TonePlayer::adjustInhibitDuration(double duration) {
    this->inhibitDuration = constrain(duration, 1000, 60000);
}

bool TonePlayer::isPlaying() {
    return playing;
}

bool TonePlayer::isInhibited() {
    return inhibited;
}

void TonePlayer::update() {
    if (playing && myMP3player.available() && myMP3player.readType() == DFPlayerPlayFinished) {
        playing = false;
        inhibited = true;
        inhibitStartTime = millis();
        notifyListeners();
    }

    if (inhibited && (millis() - inhibitStartTime) > inhibitDuration) {
        inhibited = false;
    }
}

void TonePlayer::addListener(TonePlayerListener* listener) {
    listeners.push_back(listener);
}

void TonePlayer::notifyListeners() {
    for (TonePlayerListener* listener : listeners) {
        listener->onToneFinished();
    }
}
