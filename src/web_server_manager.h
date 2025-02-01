#ifndef WEB_SERVER_MANAGER_H
#define WEB_SERVER_MANAGER_H

#include <WebServer.h>

class WebServerManager {
private:
    WebServer server;
    
public:
    WebServerManager(int port = 80);
    
    void begin();
    void handleClient();
    
    // Setup route handlers
    void setupRoutes();
    
    // Default handlers
    void handleRoot();
    void handleNotFound();
};

#endif
