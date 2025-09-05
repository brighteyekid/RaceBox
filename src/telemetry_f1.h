#ifndef TELEMETRY_F1_H
#define TELEMETRY_F1_H

#include <Arduino.h>
#include "config.h"

// F1 2020 Constants
#define F1_PACKET_FORMAT_2020 2020
#define F1_MAX_CARS 22
#define F1_PACKET_ID_CAR_TELEMETRY 6

#pragma pack(push, 1)

// Packet Header (24 bytes)
struct PacketHeader {
    uint16_t m_packetFormat;             // 2020
    uint8_t  m_gameMajorVersion;         // Game major version - "X.00"
    uint8_t  m_gameMinorVersion;         // Game minor version - "1.XX"
    uint8_t  m_packetVersion;            // Version of this packet type
    uint8_t  m_packetId;                 // Identifier for the packet type
    uint64_t m_sessionUID;               // Unique identifier for the session
    float    m_sessionTime;              // Session timestamp
    uint32_t m_frameIdentifier;          // Identifier for the frame
    uint8_t  m_playerCarIndex;           // Index of player's car in the array
    uint8_t  m_secondaryPlayerCarIndex;  // Index of secondary player's car
};

// Car Telemetry Data (58 bytes per car)
struct CarTelemetryData {
    uint16_t m_speed;                    // Speed of car in km/h
    float    m_throttle;                 // Amount of throttle applied (0.0 to 1.0)
    float    m_steer;                    // Steering (-1.0 (full lock left) to 1.0 (full lock right))
    float    m_brake;                    // Amount of brake applied (0.0 to 1.0)
    uint8_t  m_clutch;                   // Amount of clutch applied (0 to 100)
    int8_t   m_gear;                     // Gear selected (1-8, N=0, R=-1)
    uint16_t m_engineRPM;                // Engine RPM
    uint8_t  m_drs;                      // 0 = off, 1 = on
    uint8_t  m_revLightsPercent;         // Rev lights indicator (percentage)
    uint16_t m_brakesTemperature[4];     // Brakes temperature (celsius)
    uint8_t  m_tyresSurfaceTemperature[4]; // Tyres surface temperature (celsius)
    uint8_t  m_tyresInnerTemperature[4]; // Tyres inner temperature (celsius)
    uint16_t m_engineTemperature;        // Engine temperature (celsius)
    float    m_tyresPressure[4];         // Tyres pressure (PSI)
    uint8_t  m_surfaceType[4];           // Driving surface, see appendices
};

// Complete Car Telemetry Packet
struct PacketCarTelemetryData {
    PacketHeader    m_header;               // Header
    CarTelemetryData m_carTelemetryData[F1_MAX_CARS]; // Data for all cars on track
    uint32_t        m_buttonStatus;         // Bit flags for button states
    uint8_t         m_mfdPanelIndex;        // Index of MFD panel open (255 = MFD closed)
    uint8_t         m_mfdPanelIndexSecondaryPlayer;  // Secondary player MFD (split screen)
    int8_t          m_suggestedGear;        // Suggested gear for the player (1-8)
};

#pragma pack(pop)

// Simplified telemetry data for display
struct F1TelemetryData {
    float speed = 0.0f;           // km/h
    int gear = 0;                 // Current gear
    int engineRPM = 0;            // Engine RPM
    float throttle = 0.0f;        // Throttle position (0-1)
    float brake = 0.0f;           // Brake position (0-1)
    float fuelInTank = 0.0f;      // Fuel remaining (from car status packet)
    float lastLapTime = 0.0f;     // Last lap time (from lap data packet)
    bool dataValid = false;       // Data validity flag
    unsigned long timestamp = 0;   // When data was received
};

class F1TelemetryParser {
public:
    F1TelemetryParser();
    void begin();
    bool parsePacket(const uint8_t* buffer, int size);
    F1TelemetryData getLatestData() const;
    bool isDataValid() const;
    
private:
    F1TelemetryData latestData;
    unsigned long lastUpdateTime;
    
    bool validateHeader(const PacketHeader* header);
    void parseCarTelemetry(const PacketCarTelemetryData* packet);
    uint16_t readUint16LE(const uint8_t* data);
    uint32_t readUint32LE(const uint8_t* data);
    float readFloatLE(const uint8_t* data);
};

#endif // TELEMETRY_F1_H