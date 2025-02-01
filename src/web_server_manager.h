#ifndef WEB_SERVER_MANAGER_H
#define WEB_SERVER_MANAGER_H

#include <WebServer.h>
#include "Config.h"

class WebServerManager {
private:
    WebServer server;
    Config& config;
    
    // Helper methods
    String getHeader(const char* title);
    String getFooter();
    
public:
    WebServerManager(Config& config, int port = 80);
    
    void begin();
    void handleClient();
    void setupRoutes();
    
    // Route handlers
    void handleRoot();
    void handleNotFound();
    void handleWifiConfig();
    void handleWifiSave();
    void handleMessageConfig();
    void handleMessageSave();
};

#endif
