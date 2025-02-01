#include "web_server_manager.h"

WebServerManager::WebServerManager(int port) : server(port) {
}

void WebServerManager::begin() {
    setupRoutes();
    server.begin();
}

void WebServerManager::handleClient() {
    server.handleClient();
}

void WebServerManager::setupRoutes() {
    server.on("/", [this]() { this->handleRoot(); });
    server.onNotFound([this]() { this->handleNotFound(); });
}

void WebServerManager::handleRoot() {
    String html = "<html><body>";
    html += "<h1>ESP32 Web Server</h1>";
    html += "</body></html>";
    server.send(200, "text/html", html);
}

void WebServerManager::handleNotFound() {
    server.send(404, "text/plain", "Not found");
}
