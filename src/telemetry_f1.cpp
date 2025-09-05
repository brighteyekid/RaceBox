#include "telemetry_f1.h"

F1TelemetryParser::F1TelemetryParser() : lastUpdateTime(0) {
}

void F1TelemetryParser::begin() {
    Serial.println("F1 Telemetry Parser initialized");
    latestData = F1TelemetryData(); // Reset to defaults
}

bool F1TelemetryParser::parsePacket(const uint8_t* buffer, int size) {
    #ifdef ESP8266_BOARD
    yield();
    #endif
    
    // Debug: Print packet size
    #if DEBUG_UDP
    Serial.printf("F1: Received packet size: %d bytes\n", size);
    #endif

    // Minimum size check - must have at least a header
    if (size < sizeof(PacketHeader)) {
        #if DEBUG_UDP
        Serial.printf("F1: Packet too small for header (%d < %d)\n", size, sizeof(PacketHeader));
        #endif
        return false;
    }
    
    const PacketHeader* header = reinterpret_cast<const PacketHeader*>(buffer);
    
    #if DEBUG_UDP
    Serial.printf("F1: Header - Format: %d, PacketId: %d\n", header->m_packetFormat, header->m_packetId);
    #endif

    if (!validateHeader(header)) {
        return false;
    }
    
    // Only process car telemetry packets
    if (header->m_packetId != F1_PACKET_ID_CAR_TELEMETRY) {
        #if DEBUG_UDP
        Serial.printf("F1: Ignoring non-telemetry packet (ID: %d)\n", header->m_packetId);
        #endif
        return false;
    }
    
    // Calculate minimum size for car telemetry packet
    // Header + Car data for all cars + Button/MFD data
    size_t minSize = sizeof(PacketHeader) + (sizeof(CarTelemetryData) * F1_MAX_CARS) + 7;
    
    if (size < minSize) {
        #if DEBUG_UDP
        Serial.printf("F1: Packet too small for car telemetry (%d < %d)\n", size, minSize);
        #endif
        return false;
    }
    
    // Parse car telemetry data
    const PacketCarTelemetryData* telemetryPacket = 
        reinterpret_cast<const PacketCarTelemetryData*>(buffer);
    
    parseCarTelemetry(telemetryPacket);
    
    lastUpdateTime = millis();
    latestData.dataValid = true;
    latestData.timestamp = lastUpdateTime;
    
    return true;
}

bool F1TelemetryParser::validateHeader(const PacketHeader* header) {
    // Check packet format (should be 2020 for F1 2020)
    if (header->m_packetFormat != F1_PACKET_FORMAT_2020) {
        #if DEBUG_UDP
        Serial.printf("F1: Invalid packet format (%d, expected %d)\n", 
                     header->m_packetFormat, F1_PACKET_FORMAT_2020);
        #endif
        return false;
    }
    
    // Check player car index is valid
    if (header->m_playerCarIndex >= F1_MAX_CARS) {
        #if DEBUG_UDP
        Serial.printf("F1: Invalid player car index (%d)\n", header->m_playerCarIndex);
        #endif
        return false;
    }
    
    return true;
}

void F1TelemetryParser::parseCarTelemetry(const PacketCarTelemetryData* packet) {
    // Get player car index from header
    uint8_t playerIndex = packet->m_header.m_playerCarIndex;
    
    // Extract telemetry data for player car
    const CarTelemetryData& carData = packet->m_carTelemetryData[playerIndex];
    
    // Parse speed (uint16_t in km/h)
    latestData.speed = static_cast<float>(carData.m_speed);
    
    // Parse gear (int8_t: 1-8, N=0, R=-1)
    latestData.gear = static_cast<int>(carData.m_gear);
    
    // Parse engine RPM (uint16_t)
    latestData.engineRPM = static_cast<int>(carData.m_engineRPM);
    
    // Parse throttle and brake (float 0.0-1.0)
    latestData.throttle = carData.m_throttle;
    latestData.brake = carData.m_brake;
    
    // Note: F1 2020 car telemetry packet doesn't include fuel or lap time
    // These would come from other packet types (Car Status, Lap Data)
    // For now, we'll keep previous values or set defaults
    
    #if DEBUG_UDP
    Serial.printf("F1 Parsed: Speed=%.1f km/h, Gear=%d, RPM=%d, Throttle=%.2f, Brake=%.2f\n",
                 latestData.speed, latestData.gear, latestData.engineRPM, 
                 latestData.throttle, latestData.brake);
    #endif
}

F1TelemetryData F1TelemetryParser::getLatestData() const {
    return latestData;
}

bool F1TelemetryParser::isDataValid() const {
    // Data is valid if we received it recently (within 2 seconds)
    return latestData.dataValid && (millis() - lastUpdateTime < 2000);
}

// Helper functions for endian handling (F1 data is little-endian)
uint16_t F1TelemetryParser::readUint16LE(const uint8_t* data) {
    return data[0] | (data[1] << 8);
}

uint32_t F1TelemetryParser::readUint32LE(const uint8_t* data) {
    return data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
}

float F1TelemetryParser::readFloatLE(const uint8_t* data) {
    union {
        uint32_t i;
        float f;
    } converter;
    converter.i = readUint32LE(data);
    return converter.f;
}