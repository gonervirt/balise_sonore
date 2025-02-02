/**
 * @file web_server_manager.h
 * @brief Web interface management for ESP32 Balise Sonore
 * 
 * @copyright Copyright (c) 2024 ESP32 Balise Sonore Project
 * 
 * @license MIT License
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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

    /** Handle ESP32 configuration page */
    void handleEsp32Config();
    
    /** Handle ESP32 actions (reset/clear config) */
    void handleEsp32Action();

    String formatConfigItem(const char* label, const String& value);
};

#endif
