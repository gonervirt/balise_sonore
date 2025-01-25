#ifndef WEBSERVERMANAGER_H
#define WEBSERVERMANAGER_H

#include <ESPAsyncWebServer.h>
#include "TonePlayer.h"
#include "Config.h"

class WebServerManager {
public:
    WebServerManager(TonePlayer& tonePlayer, Config& config);
    void begin();
    void handleClient();
    int getNumeroMessage() const;

private:
    AsyncWebServer server;
    TonePlayer& tonePlayer;
    Config& config;
    void setupRoutes();
};

#endif // WEBSERVERMANAGER_H
