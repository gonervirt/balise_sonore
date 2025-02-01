#ifndef WEB_SERVER_MANAGER_H
#define WEB_SERVER_MANAGER_H

#include <WebServer.h>
#include "Config.h"

/**
 * @brief Web interface manager
 * 
 * Provides a web interface for configuration and status
 * monitoring of the device
 */
class WebServerManager {
private:
    WebServer server;
    Config& config;
    
    /** Generate HTML header with navigation */
    String getHeader(const char* title);
    
    /** Generate HTML footer */
    String getFooter();
    
public:
    /**
     * Create web server manager
     * @param config Configuration object to manage
     * @param port HTTP port to listen on
     */
    WebServerManager(Config& config, int port = 80);
    
    /** Start the web server */
    void begin();
    
    /** Handle incoming client requests */
    void handleClient();
    
    /** Setup all route handlers */
    void setupRoutes();
    
    // Route handlers
    /** Handle root page request */
    void handleRoot();
    
    /** Handle 404 errors */
    void handleNotFound();
    
    /** Handle WiFi configuration page */
    void handleWifiConfig();
    
    /** Process WiFi configuration form submission */
    void handleWifiSave();
    
    /** Handle message configuration page */
    void handleMessageConfig();
    
    /** Process message configuration form submission */
    void handleMessageSave();

    /** Process message configuration form submission */
    void handleMessageTextSave();

    String formatConfigItem(const char* label, const String& value);
};

#endif
