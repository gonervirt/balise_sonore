#pragma once
#include <WebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "Config.h"

class WebServerSPA {
private:
    WebServer server;
    Config& config;
    
    // API Handlers
    void handleGetConfig();
    void handleSetWifiConfig();
    void handleSetMessageConfig();
    void handleSetVolume();
    void handleSystemAction();
    
    // Utility methods
    void sendJsonResponse(const JsonDocument& doc);
    void sendJsonError(const char* message, int code = 400);
    void setupStaticFiles();
    void setupApiRoutes();

public:
    WebServerSPA(Config& config, int port = 80);
    ~WebServerSPA();
    
    void begin();
    void handleClient();
};
