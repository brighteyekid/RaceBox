#include <Arduino.h>
#ifdef ESP8266_BOARD
    #include <ESP8266WiFi.h>
#else
    #include <WiFi.h>
#endif
#include <WiFiUdp.h>
#include <Wire.h>
#include "config.h"
#include "network_manager.h"
#include "display_manager_sh1106.h"
#include "buttons.h"
#include "telemetry_f1.h"
#include "telemetry_pcars.h"

// Global objects
NetworkManager networkManager;
DisplayManagerSH1106 displayManager;
ButtonManager buttonManager;
F1TelemetryParser f1Parser;
PCARSTelemetryParser pcarsParser;

// Global state
int currentPage = PAGE_SPEED_GEAR;
int currentGame = GAME_F1;
unsigned long lastTelemetryUpdate = 0;
unsigned long lastWiFiCheck = 0;

// Telemetry data structure
struct TelemetryData {
    float speed = 0.0f;
    int gear = 0;
    int rpm = 0;
    float fuel = 0.0f;
    float lapTime = 0.0f;
    int position = 0;
    bool dataValid = false;
    unsigned long lastUpdate = 0;
    String lastPacketType = "None";
    int lastPacketSize = 0;
    String sourceIP = "0.0.0.0";
} telemetryData;

void setup() {
    Serial.begin(115200);
    delay(1000); // Give serial time to initialize
    
    #ifdef ESP8266_BOARD
    Serial.println("ESP8266 F1/PCARS Telemetry Dashboard Starting...");
    #else
    Serial.println("ESP32 F1/PCARS Telemetry Dashboard Starting...");
    #endif
    
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    
    // Initialize I2C
    Serial.println("Initializing I2C...");
    Wire.begin(SDA_PIN, SCL_PIN);
    
    // Initialize display
    Serial.println("About to initialize display...");
    if (!displayManager.begin()) {
        Serial.println("Display initialization failed!");
        Serial.println("Continuing without display...");
        // Don't halt - continue without display
    } else {
        Serial.println("Display initialization SUCCESS!");
    }
    
    // Initialize buttons
    buttonManager.begin();
    
    // Initialize network
    displayManager.showStatus("Connecting WiFi...");
    if (!networkManager.begin()) {
        Serial.println("Network initialization failed!");
        displayManager.showStatus("WiFi Failed!");
        delay(2000);
    } else {
        String ip = WiFi.localIP().toString();
        Serial.println("Connected! IP: " + ip);
        displayManager.showStatus("IP: " + ip);
        delay(2000);
    }
    
    // Initialize telemetry parsers
    f1Parser.begin();
    pcarsParser.begin();
    
    Serial.println("Setup complete!");
    
    // Force display a test message
    Serial.println("Showing initial display page...");
    displayManager.showStatus("Ready!");
    delay(2000);
    
    displayManager.showPage(currentPage, telemetryData, currentGame);
    Serial.printf("Showing page %d for game %d\n", currentPage, currentGame);
}

void loop() {
    unsigned long currentTime = millis();
    
    // Check WiFi connection periodically
    if (currentTime - lastWiFiCheck > WIFI_RECONNECT_INTERVAL_MS) {
        if (!networkManager.isConnected()) {
            Serial.println("WiFi disconnected, attempting reconnect...");
            displayManager.showStatus("Reconnecting...");
            networkManager.reconnect();
        }
        lastWiFiCheck = currentTime;
    }
    
    #ifdef ESP8266_BOARD
    // ESP8266 needs yield() to prevent watchdog reset
    yield();
    #endif
    
    // Handle button presses
    ButtonEvent event = buttonManager.update();
    if (event == BUTTON_NEXT_PRESSED) {
        currentPage = (currentPage + 1) % MAX_PAGES;
        displayManager.showPage(currentPage, telemetryData, currentGame);
    } else if (event == BUTTON_SELECT_PRESSED) {
        if (currentPage == PAGE_SETTINGS) {
            currentGame = (currentGame == GAME_F1) ? GAME_PCARS : GAME_F1;
            Serial.println("Switched to game: " + String(currentGame == GAME_F1 ? "F1" : "PCARS"));
            displayManager.showPage(currentPage, telemetryData, currentGame);
        }
    }
    
    // Process telemetry data
    bool dataUpdated = false;
    
    if (currentGame == GAME_F1) {
        // Process F1 telemetry
        if (networkManager.hasF1Data()) {
            uint8_t buffer[UDP_BUFFER_SIZE];
            int packetSize;
            IPAddress sourceIP;
            
            if (networkManager.readF1Data(buffer, packetSize, sourceIP)) {
                telemetryData.lastPacketSize = packetSize;
                telemetryData.sourceIP = sourceIP.toString();
                
                if (f1Parser.parsePacket(buffer, packetSize)) {
                    F1TelemetryData f1Data = f1Parser.getLatestData();
                    
                    telemetryData.speed = f1Data.speed;
                    telemetryData.gear = f1Data.gear;
                    telemetryData.rpm = f1Data.engineRPM;
                    telemetryData.fuel = f1Data.fuelInTank;
                    telemetryData.lapTime = f1Data.lastLapTime;
                    telemetryData.dataValid = true;
                    telemetryData.lastUpdate = currentTime;
                    telemetryData.lastPacketType = "F1 CarTelemetry";
                    
                    dataUpdated = true;
                    
                    #if DEBUG_UDP
                    Serial.printf("F1 Data: Speed=%.1f, Gear=%d, RPM=%d\n", 
                                 telemetryData.speed, telemetryData.gear, telemetryData.rpm);
                    #endif
                } else {
                    #if DEBUG_UDP
                    Serial.printf("F1 Parse FAILED: %d bytes from %s\n", packetSize, sourceIP.toString().c_str());
                    #endif
                }
            } else {
                #if DEBUG_UDP
                Serial.println("F1 Read FAILED");
                #endif
            }
        }
    } else {
        // Process PCARS telemetry
        if (networkManager.hasPCARSData()) {
            uint8_t buffer[UDP_BUFFER_SIZE];
            int packetSize;
            IPAddress sourceIP;
            
            if (networkManager.readPCARSData(buffer, packetSize, sourceIP)) {
                telemetryData.lastPacketSize = packetSize;
                telemetryData.sourceIP = sourceIP.toString();
                
                if (pcarsParser.parsePacket(buffer, packetSize)) {
                    PCARSTelemetryData pcarsData = pcarsParser.getLatestData();
                    
                    telemetryData.speed = pcarsData.speed;
                    telemetryData.gear = pcarsData.gear;
                    telemetryData.rpm = pcarsData.rpm;
                    telemetryData.fuel = pcarsData.fuel;
                    telemetryData.lapTime = pcarsData.lapTime;
                    telemetryData.dataValid = true;
                    telemetryData.lastUpdate = currentTime;
                    telemetryData.lastPacketType = pcarsData.isForwarderData ? "PCARS JSON" : "PCARS UDP";
                    
                    dataUpdated = true;
                    
                    #if DEBUG_UDP
                    Serial.printf("PCARS Data: Speed=%.1f, Gear=%d, RPM=%d\n", 
                                 telemetryData.speed, telemetryData.gear, telemetryData.rpm);
                    #endif
                }
            }
        }
    }
    
    // Check for data timeout (2 seconds for more stability)
    if (currentTime - telemetryData.lastUpdate > 2000) {
        telemetryData.dataValid = false;
    }
    
    // Update display if data changed or periodically
    if (dataUpdated || (currentTime - lastTelemetryUpdate > 100)) {
        displayManager.showPage(currentPage, telemetryData, currentGame);
        lastTelemetryUpdate = currentTime;
    }
    
    #ifdef ESP8266_BOARD
    // ESP8266 needs more frequent yields and shorter delays
    yield();
    delay(5);
    #else
    // ESP32 can handle longer delays
    delay(10);
    #endif
}