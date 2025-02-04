#include "TonePlayer.h"
#include "Arduino.h"

TonePlayer::TonePlayer(int _rxd2, int _txd2) : volume(15), inhibitDuration(2000), playing(false), inhibited(false), inhibitStartTime(0) {
    rxd2 = _rxd2;
    txd2 = _txd2;
}

void TonePlayer::begin() {
    static HardwareSerial serial2player(0); // Ensure the serial object is static to maintain scope
    serial2player.begin(9600, SERIAL_8N1, rxd2, txd2);
    myMP3player.setTimeOut(1000); //donne plus de temps pour lire les informations
    if (!myMP3player.begin(serial2player)) {
        Serial.println(F("Unable to initialise Tone Player"));
    } else {
        Serial.println(F("Tone Player initialised"));
        myMP3player.enableDAC();
        myMP3player.volume(volume);
    }
}

void TonePlayer::playTone(int messageNumber) {
    if (! playing && ! inhibited) {
        Serial.println("playTone " + String(messageNumber));
        myMP3player.play(messageNumber);
        playing = true;
    } else notifyListeners();
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
    Serial.println("TonePlayer update called "+String(playing)+"-"+String(inhibited)+"-"+String(inhibitStartTime)+"-"+String(inhibitDuration)+"-"+String(myMP3player.available()+"-"+String(myMP3player.readType())));

    if (playing && myMP3player.available() && myMP3player.readType() == DFPlayerPlayFinished) {
        Serial.println(".....................................Tone finished playing");
        playing = false; 
        inhibited = true;
        inhibitStartTime = millis();
        notifyListeners();
    }

    if (inhibited && (millis() - inhibitStartTime) > inhibitDuration) {
        Serial.println("Inhibit period ended");
        inhibited = false;
        notifyListeners();
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
