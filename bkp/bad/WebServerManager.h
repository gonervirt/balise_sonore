#ifndef WEBSERVERMANAGER_H
#define WEBSERVERMANAGER_H

#include <ESPAsyncWebServer.h>
//#include <WiFi.h>
//#include "TonePlayer.h"
#include "Config.h"

class WebServerManager {
public:
    WebServerManager (Config& config);
    void begin();
    void handleClient();
    int getNumeroMessage() const;

private:
    AsyncWebServer server;
    //WiFiServer server;
    Config& config;
    void setupRoutes();
};

#endif // WEBSERVERMANAGER_H
