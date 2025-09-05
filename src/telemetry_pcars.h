#ifndef TELEMETRY_PCARS_H
#define TELEMETRY_PCARS_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"

// Project CARS 2 UDP Telemetry Structure
// Note: PCARS2 has a complex binary format. This is a simplified version
// focusing on the most important fields for dashboard display.
// For complete specification, refer to community documentation.

#pragma pack(push, 1)

// Simplified PCARS2 telemetry structure (partial implementation)
// TODO: Complete implementation based on community reverse engineering
// Reference: https://github.com/SHWotever/SimHub/wiki/Project-Cars-2
struct PCARSUDPData {
    // Header-like fields (first few bytes to identify packet type)
    uint32_t buildVersionNumber;     // Build version
    uint8_t  packetType;             // Packet type identifier
    
    // Essential telemetry data (approximate offsets)
    float speed;                     // Speed in m/s
    float engineRPM;                 // Engine RPM
    float maxRPM;                    // Maximum RPM
    int8_t gear;                     // Current gear (-1=R, 0=N, 1-8=gears)
    float fuelLevel;                 // Fuel level (0.0-1.0)
    float lastLapTime;               // Last lap time in seconds
    
    // Note: This is a simplified structure. PCARS2 packets are much larger
    // and contain many more fields. For production use, implement full parsing
    // or use the JSON forwarder approach.
};

#pragma pack(pop)

// Simplified telemetry data for display
struct PCARSTelemetryData {
    float speed = 0.0f;           // km/h (converted from m/s)
    int gear = 0;                 // Current gear
    int rpm = 0;                  // Engine RPM
    float fuel = 0.0f;            // Fuel level percentage
    float lapTime = 0.0f;         // Last lap time
    bool dataValid = false;       // Data validity flag
    bool isForwarderData = false; // True if data came from JSON forwarder
    unsigned long timestamp = 0;  // When data was received
};

class PCARSTelemetryParser {
public:
    PCARSTelemetryParser();
    void begin();
    bool parsePacket(const uint8_t* buffer, int size);
    PCARSTelemetryData getLatestData() const;
    bool isDataValid() const;
    
private:
    PCARSTelemetryData latestData;
    unsigned long lastUpdateTime;
    
    bool parseJSONForwarder(const uint8_t* buffer, int size);
    bool parseBinaryUDP(const uint8_t* buffer, int size);
    bool isJSONPacket(const uint8_t* buffer, int size);
    float readFloatLE(const uint8_t* data);
    uint32_t readUint32LE(const uint8_t* data);
};

#endif // TELEMETRY_PCARS_H