#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#ifdef ESP8266_BOARD
    #include <ESP8266WiFi.h>
#else
    #include <WiFi.h>
#endif
#include <WiFiUdp.h>
#include "config.h"

class NetworkManager {
public:
    NetworkManager();
    bool begin();
    bool isConnected();
    void reconnect();
    
    // F1 UDP handling
    bool hasF1Data();
    bool readF1Data(uint8_t* buffer, int& packetSize, IPAddress& sourceIP);
    
    // PCARS UDP handling
    bool hasPCARSData();
    bool readPCARSData(uint8_t* buffer, int& packetSize, IPAddress& sourceIP);
    
private:
    WiFiUDP f1Udp;
    WiFiUDP pcarsUdp;
    bool wifiConnected;
    unsigned long lastConnectionAttempt;
    
    bool connectWiFi();
    void setupUDP();
};

#endif // NETWORK_MANAGER_H