#include "wifi_manager.h"

WiFiManager::WiFiManager(const char* ap_ssid, const char* ap_password) {
    this->ssid = ap_ssid;
    this->password = ap_password;
    this->isAP = true;
    this->channel = 6;
    this->hidden_ssid = false;
    this->lastCheckTime = 0;  // Initialize timer
    this->lastLogTime = 0;    // Initialize log timer
}

WiFiManager::WiFiManager(const char* sta_ssid, const char* sta_password, bool station) {
    this->ssid = sta_ssid;
    this->password = sta_password;
    this->isAP = false;
    this->channel = 6;
    this->hidden_ssid = false;
}

WiFiManager::WiFiManager(Config &config) {
    this->ssid = config.getWifiSSID();
    this->password = config.getWifiPassword();
    this->isAP = config.isAccessPoint();
    this->channel = config.getWifiChannel();
    this->hidden_ssid = config.isHiddenSSID();
}

bool WiFiManager::begin() {
    if (isAP) {
        Serial.println("Starting Access Point mode...");
        WiFi.mode(WIFI_AP);
        WiFi.setSleep(false);
        bool success = WiFi.softAP(ssid, password, channel, hidden_ssid);
        if (success) {
            Serial.println("Access Point started successfully");
            Serial.printf("SSID: %s\n", ssid);
            Serial.printf("IP Address: %s\n", WiFi.softAPIP().toString().c_str());
        } else {
            Serial.println("Failed to start Access Point");
        }

        // Register event handlers for station connect/disconnect and other events
        esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_STACONNECTED, &event_handler, NULL);
        esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_STADISCONNECTED, &event_handler, NULL);
        esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_STOP, &event_handler, NULL);
        esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_START, &event_handler, NULL);
        esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_PROBEREQRECVED, &event_handler, NULL);

        return success;
    } else {
        Serial.println("Starting Station mode...");
        Serial.printf("Connecting to SSID: %s\n", ssid);
        
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);
        
        int timeout = 20;
        while (WiFi.status() != WL_CONNECTED && timeout > 0) {
            Serial.print(".");
            delay(500);
            timeout--;
        }
        Serial.println();
        
        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("Successfully connected to WiFi");
            Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
            return true;
        }
        Serial.println("Failed to connect to WiFi");
        return false;
    }
}

String WiFiManager::getIP() {
    if (isAP) {
        return WiFi.softAPIP().toString();
    }
    return WiFi.localIP().toString();
}

WiFiManager::WifiStatus WiFiManager::checkStatus() {
    WifiStatus status;
    if (isAP) {
        status.ssid = String(ssid);
        status.ip = WiFi.softAPIP().toString();
        status.isConnected = WiFi.softAPgetStationNum() > 0;
        status.rssi = 0; // Not applicable for AP mode
    } else {
        status.ssid = String(ssid);
        status.ip = WiFi.localIP().toString();
        status.isConnected = WiFi.status() == WL_CONNECTED;
        status.rssi = WiFi.RSSI();
    }
    return status;
}

void WiFiManager::checkAndRestartAP() {
    if (!isAP) return;
    
    if (WiFi.softAPgetStationNum() == 0) {
        Serial.println("No stations connected. Restarting AP...");
        WiFi.softAPdisconnect(true);
        delay(1000);
        WiFi.softAP(ssid, password, channel, hidden_ssid);
        Serial.println("AP restarted");
    }
}

void WiFiManager::logAPStatus() {
    if (!isAP) return;

    unsigned long currentTime = millis();
    if (currentTime - lastLogTime >= 60000) {  // Log every minute
        Serial.printf("AP Status: SSID=%s, IP=%s, Stations Connected=%d, Free Heap=%d bytes\n",
                      ssid, WiFi.softAPIP().toString().c_str(), WiFi.softAPgetStationNum(), ESP.getFreeHeap());
        lastLogTime = currentTime;
    }
}

void WiFiManager::loop() {
    if (!isAP) return;
    
    unsigned long currentTime = millis();
    if (currentTime - lastCheckTime >= 60000) {  // Check every minute
        checkAndRestartAP();
        lastCheckTime = currentTime;
    }

    logAPStatus();  // Log AP status periodically

    // Check heap size
    if (ESP.getFreeHeap() < 10000) {  // Example threshold, adjust as needed
        Serial.println("Warning: Low heap memory!");
    }
}

// Event handler for station connect/disconnect and other events
void WiFiManager::event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        Serial.printf("Station " MACSTR " joined, AID=%d\n", MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        Serial.printf("Station " MACSTR " left, AID=%d\n", MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STOP) {
        Serial.println("Access Point stopped");
    } else if (event_id == WIFI_EVENT_AP_START) {
        Serial.println("Access Point started");
    } else if (event_id == WIFI_EVENT_AP_PROBEREQRECVED) {
        Serial.println("Probe request received");
    }
}

