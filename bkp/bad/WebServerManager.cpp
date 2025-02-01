#include "WebServerManager.h"
#include "Arduino.h"

WebServerManager::WebServerManager(Config& config) : server(80), config(config) {}

void WebServerManager::begin() {
    setupRoutes();
    server.begin();
}

void WebServerManager::handleClient() {
    // No need to handle client manually, AsyncWebServer handles it
}

int WebServerManager::getNumeroMessage() const {
    return config.getNumeroMessage();
}

void WebServerManager::setupRoutes() {
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html", "Click <a href=\"/1\">here</a> ALLUME LA LED ET PREPARE LE PREMIER MESSAGE ENTREE MAIRIE ET POSTE.<br>"
                                       "Click <a href=\"/2\">here</a> ALLUME LA LED ET PREPARE LE SECOND MESSAGE POSTE FERMEE.<br>"
                                       "Click <a href=\"/3\">here</a> ALLUME LA LED ET PREPARE LE TROISIEME MESSAGE MAIRIE FERMEE.<br>"
                                       "Click <a href=\"/0\">here</a> ETEINT LA LED.<br>");
    });

    server.on("/0", HTTP_GET, [this](AsyncWebServerRequest *request) {
        digitalWrite(2, LOW);
        Serial.println("Extinction de la led");
        config.setNumeroMessage(0);
        config.saveConfig();
        request->send(200, "text/plain", "LED Off");
    });

    server.on("/1", HTTP_GET, [this](AsyncWebServerRequest *request) {
        digitalWrite(2, HIGH);
        config.setNumeroMessage(1);
        config.saveConfig();
        Serial.println("Messsage 1, allumage LED");
        request->send(200, "text/plain", "Message 1");
    });

    server.on("/2", HTTP_GET, [this](AsyncWebServerRequest *request) {
        digitalWrite(2, HIGH);
        config.setNumeroMessage(2);
        config.saveConfig();
        Serial.println("Messsage 2, allumage LED");
        request->send(200, "text/plain", "Message 2");
    });

    server.on("/3", HTTP_GET, [this](AsyncWebServerRequest *request) {
        digitalWrite(2, HIGH);
        config.setNumeroMessage(3);
        config.saveConfig();
        Serial.println("Messsage 3, allumage LED");
        request->send(200, "text/plain", "Message 3");
    });

    server.on("/volume", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if (request->hasParam("value")) {
            int volume = request->getParam("value")->value().toInt();
            //tonePlayer.adjustVolume(volume);
            Serial.println("Volume ajusté à : " + volume);
            request->send(200, "text/plain", "Volume adjusted");
        } else {
            request->send(400, "text/plain", "Missing volume value");
        }
    });

    server.on("/inhibit", HTTP_GET, [this](AsyncWebServerRequest *request) {
        if (request->hasParam("value")) {
            double duration = request->getParam("value")->value().toDouble();
            //tonePlayer.adjustInhibitDuration(duration);
            String durationStr = String(duration, 2); // Convert double to String with 2 decimal places
            Serial.println("Durée d'inhibition ajustée à : " + durationStr);
            request->send(200, "text/plain", "Inhibit duration adjusted");
        } else {
            request->send(400, "text/plain", "Missing inhibit duration value");
        }
    });
}
