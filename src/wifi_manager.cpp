#include "wifi_manager.h"
#include "esp_event.h" // keep if you register esp events

WiFiManager::WiFiManager(const char* ap_ssid, const char* ap_password) {
    this->ssid = ap_ssid ? String(ap_ssid) : String();
    this->password = ap_password ? String(ap_password) : String();
    this->isAP = true;
    this->channel = 6;
    this->hidden_ssid = false;
    this->lastCheckTime = 0;
    this->lastLogTime = 0;
}

WiFiManager::WiFiManager(const char* sta_ssid, const char* sta_password, bool station) {
    this->ssid = sta_ssid ? String(sta_ssid) : String();
    this->password = sta_password ? String(sta_password) : String();
    this->isAP = station;
    this->channel = 6;
    this->hidden_ssid = false;
    this->lastCheckTime = 0;
    this->lastLogTime = 0;
}

WiFiManager::WiFiManager(Config &config) {
    this->ssid = String(config.getWifiSSID());
    this->password = String(config.getWifiPassword());
    this->isAP = config.isAccessPoint();
    this->channel = config.getWifiChannel();
    this->hidden_ssid = config.isHiddenSSID();
    this->lastCheckTime = 0;
    this->lastLogTime = 0;
}

bool WiFiManager::begin() {
    if (isAP) {
        Serial.println("Starting Access Point mode...");
        WiFi.mode(WIFI_AP_STA);
        WiFi.setSleep(false);

        if (ssid.length() == 0) {
            Serial.println("ERROR: AP SSID is empty");
            return false;
        }

        const char* apPass = (password.length() >= 8) ? password.c_str() : nullptr;

        IPAddress localIP(192,168,4,1);
        IPAddress gateway(192,168,4,1);
        IPAddress subnet(255,255,255,0);
        WiFi.softAPConfig(localIP, gateway, subnet);

        bool success = WiFi.softAP(ssid.c_str(), apPass, channel, hidden_ssid);
        delay(50);

        if (!success) {
            Serial.println("Failed to start Access Point");
            return false;
        }

        IPAddress ip = WiFi.softAPIP();
        if (ip == IPAddress(0,0,0,0)) {
            Serial.println("Warning: softAP started but IP is 0.0.0.0");
            return false;
        }

        Serial.println("Access Point started successfully");
        Serial.printf("SSID: %s\n", ssid.c_str());
        Serial.printf("Password: %s\n", apPass);
        Serial.printf("IP Address: %s\n", ip.toString().c_str());
        Serial.printf("Channel: %d Hidden: %s\n", channel, hidden_ssid ? "yes" : "no");

        // register events if needed (keep existing handler)
        esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_STACONNECTED, &event_handler, NULL);
        esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_STADISCONNECTED, &event_handler, NULL);
        esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_STOP, &event_handler, NULL);
        esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_START, &event_handler, NULL);
        esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_PROBEREQRECVED, &event_handler, NULL);

        return true;
    } else {
        Serial.println("Starting Station mode...");

        if (ssid.length() == 0) {
            Serial.println("ERROR: STA SSID is empty");
            return false;
        }

        Serial.printf("Connecting to SSID: %s\n", ssid.c_str());
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid.c_str(), password.c_str());

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
                      ssid.c_str(), WiFi.softAPIP().toString().c_str(), WiFi.softAPgetStationNum(), ESP.getFreeHeap());
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

