#include "network_manager.h"

NetworkManager::NetworkManager() : wifiConnected(false), lastConnectionAttempt(0) {
}

bool NetworkManager::begin() {
    return connectWiFi();
}

bool NetworkManager::connectWiFi() {
    Serial.println("Connecting to WiFi: " + String(WIFI_SSID));
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < WIFI_TIMEOUT_MS) {
        delay(500);
        Serial.print(".");
        #ifdef ESP8266_BOARD
        yield(); // ESP8266 needs yield() in loops
        #endif
    }
    
    if (WiFi.status() == WL_CONNECTED) {
        wifiConnected = true;
        Serial.println();
        Serial.println("WiFi connected!");
        Serial.println("IP address: " + WiFi.localIP().toString());
        
        setupUDP();
        return true;
    } else {
        wifiConnected = false;
        Serial.println();
        Serial.println("WiFi connection failed!");
        return false;
    }
}

void NetworkManager::setupUDP() {
    // Setup F1 UDP listener
    if (f1Udp.begin(F1_UDP_PORT)) {
        Serial.println("F1 UDP listener started on port " + String(F1_UDP_PORT));
    } else {
        Serial.println("Failed to start F1 UDP listener");
    }
    
    // Setup PCARS UDP listener (broadcast)
    if (pcarsUdp.begin(PCARS_UDP_PORT)) {
        Serial.println("PCARS UDP listener started on port " + String(PCARS_UDP_PORT));
    } else {
        Serial.println("Failed to start PCARS UDP listener");
    }
    
    // Also listen on forwarder port for PCARS JSON data
    // Note: We'll use the same pcarsUdp object and check packet content to distinguish
}

bool NetworkManager::isConnected() {
    wifiConnected = (WiFi.status() == WL_CONNECTED);
    return wifiConnected;
}

void NetworkManager::reconnect() {
    unsigned long currentTime = millis();
    if (currentTime - lastConnectionAttempt > 5000) { // Don't attempt too frequently
        lastConnectionAttempt = currentTime;
        
        WiFi.disconnect();
        delay(1000);
        connectWiFi();
    }
}

bool NetworkManager::hasF1Data() {
    return f1Udp.parsePacket() > 0;
}

bool NetworkManager::readF1Data(uint8_t* buffer, int& packetSize, IPAddress& sourceIP) {
    // parsePacket() was already called in hasF1Data(), so packet is ready to read
    packetSize = f1Udp.available();
    if (packetSize > 0 && packetSize <= UDP_BUFFER_SIZE) {
        sourceIP = f1Udp.remoteIP();
        int bytesRead = f1Udp.read(buffer, packetSize);
        
        #if DEBUG_UDP
        Serial.printf("F1 UDP: %d bytes from %s\n", bytesRead, sourceIP.toString().c_str());
        #endif
        
        return bytesRead == packetSize;
    }
    return false;
}

bool NetworkManager::hasPCARSData() {
    return pcarsUdp.parsePacket() > 0;
}

bool NetworkManager::readPCARSData(uint8_t* buffer, int& packetSize, IPAddress& sourceIP) {
    // parsePacket() was already called in hasPCARSData(), so packet is ready to read
    packetSize = pcarsUdp.available();
    if (packetSize > 0 && packetSize <= UDP_BUFFER_SIZE) {
        sourceIP = pcarsUdp.remoteIP();
        int bytesRead = pcarsUdp.read(buffer, packetSize);
        
        #if DEBUG_UDP
        Serial.printf("PCARS UDP: %d bytes from %s\n", bytesRead, sourceIP.toString().c_str());
        #endif
        
        return bytesRead == packetSize;
    }
    return false;
}