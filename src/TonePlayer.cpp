#include <SoftwareSerial.h>
#include "TonePlayer.h"
#include "Arduino.h"

TonePlayer::TonePlayer(int _rxd2, int _txd2) : volume(15), inhibitDuration(2000), playing(false), inhibited(false), inhibitStartTime(0) {
    rxd2 = _rxd2;
    txd2 = _txd2;
    serial2player = new SoftwareSerial(rxd2, txd2);
}

TonePlayer::~TonePlayer() {
    if (serial2player) {
        delete serial2player;
        serial2player = nullptr;
    }
}

void TonePlayer::begin() {
    serial2player->begin(9600);
    myMP3player.setTimeOut(1000); //donne plus de temps pour lire les informations
    myMP3player.begin(*serial2player, /*isACK = */true, /*doReset = */true);
    Serial.println(F("Waiting DF player"));
    delay(1000);
    myMP3player.reset();
    while (!myMP3player.available()) {
        Serial.print(F("."));
        delay(1000);
    }
    Serial.println(F(""));

    Serial.println(F("DFPlayer Mini online."));

    myMP3player.enableDAC();
    //myMP3player.volume(volume);
    adjustVolume(volume);
    playTone(3);
    /*
    this->myMP3player.play(1);
    delay(5000);
    Serial.println(F("message 1 joué."));
    this->myMP3player.play(2);
    delay(5000);
    playTone(3);
    delay(5000);
    Serial.println(F("message 2 joué."));
    */
}

void TonePlayer::playTone(int messageNumber) {
    Serial.println("playing Tone " + String(messageNumber));
    myMP3player.play(messageNumber);
    Serial.println("tone playing...");
    /*
    Serial.println("playTone called");
    if (! playing && ! inhibited) {
        Serial.println("playTone " + String(messageNumber));
        myMP3player.play(messageNumber);
        playing = true;
        notifyListeners();
    } else {
        Serial.println("can't play playTone : playing " + String(playing)+ " - inhibited "+String(inhibited));
        //notifyListeners();
    }*/
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
    /*
    Serial.println("TonePlayer update called : Playing = "+String(playing)+
        " - Inihibited = "+String(inhibited)+
        " - Start time inibit ="+String(inhibitStartTime)+
        " - duration inibit "+String(inhibitDuration)+
       // " - m3player available="+String(myMP3player.available())+
        " - Player readType="+String(myMP3player.readType())
        );
        */

    if (playing && myMP3player.available() && myMP3player.readType() == DFPlayerPlayFinished) {
        Serial.println(".....................................Tone finished playing");
        playing = false; 
        inhibited = true;
        inhibitStartTime = millis();
        //notifyListeners();
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
