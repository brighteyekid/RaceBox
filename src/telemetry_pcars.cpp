#include "telemetry_pcars.h"

PCARSTelemetryParser::PCARSTelemetryParser() : lastUpdateTime(0) {
}

void PCARSTelemetryParser::begin() {
    Serial.println("PCARS Telemetry Parser initialized");
    latestData = PCARSTelemetryData(); // Reset to defaults
}

bool PCARSTelemetryParser::parsePacket(const uint8_t* buffer, int size) {
    if (size < 4) {
        #if DEBUG_UDP
        Serial.printf("PCARS: Packet too small (%d bytes)\n", size);
        #endif
        return false;
    }
    
    // Check if this looks like a JSON packet (forwarder data)
    if (isJSONPacket(buffer, size)) {
        return parseJSONForwarder(buffer, size);
    } else {
        return parseBinaryUDP(buffer, size);
    }
}

bool PCARSTelemetryParser::isJSONPacket(const uint8_t* buffer, int size) {
    // Simple heuristic: JSON packets start with '{' and are printable
    if (size > 0 && buffer[0] == '{') {
        // Check if the packet contains mostly printable characters
        int printableCount = 0;
        int checkSize = min(size, 50); // Check first 50 bytes
        
        for (int i = 0; i < checkSize; i++) {
            if (isprint(buffer[i]) || buffer[i] == '\n' || buffer[i] == '\r') {
                printableCount++;
            }
        }
        
        return (printableCount > checkSize * 0.8); // 80% printable = likely JSON
    }
    return false;
}

bool PCARSTelemetryParser::parseJSONForwarder(const uint8_t* buffer, int size) {
    // Parse JSON forwarder data
    // Expected format: {"speed": 120.5, "gear": 3, "rpm": 6000, "fuel": 45.2, "lapTime": 87.234}
    
    #ifdef ESP8266_BOARD
    // ESP8266 memory optimization
    yield();
    
    // Limit buffer size for ESP8266 memory constraints
    if (size > 512) {
        size = 512;
    }
    #endif
    
    // Null-terminate the buffer for JSON parsing
    char jsonBuffer[size + 1];
    memcpy(jsonBuffer, buffer, size);
    jsonBuffer[size] = '\0';
    
    #if DEBUG_UDP
    Serial.printf("PCARS JSON: %s\n", jsonBuffer);
    #endif
    
    // Parse JSON - adjust buffer size based on platform
    #ifdef ESP8266_BOARD
    DynamicJsonDocument doc(256);  // Smaller buffer for ESP8266
    #else
    DynamicJsonDocument doc(512);  // Larger buffer for ESP32
    #endif
    DeserializationError error = deserializeJson(doc, jsonBuffer);
    
    if (error) {
        #if DEBUG_UDP
        Serial.printf("PCARS JSON parse error: %s\n", error.c_str());
        #endif
        return false;
    }
    
    // Extract telemetry data
    if (doc.containsKey("speed")) {
        latestData.speed = doc["speed"].as<float>();
    }
    
    if (doc.containsKey("gear")) {
        latestData.gear = doc["gear"].as<int>();
    }
    
    if (doc.containsKey("rpm")) {
        latestData.rpm = doc["rpm"].as<int>();
    }
    
    if (doc.containsKey("fuel")) {
        latestData.fuel = doc["fuel"].as<float>();
    }
    
    if (doc.containsKey("lapTime")) {
        latestData.lapTime = doc["lapTime"].as<float>();
    }
    
    latestData.isForwarderData = true;
    latestData.dataValid = true;
    latestData.timestamp = millis();
    lastUpdateTime = latestData.timestamp;
    
    #if DEBUG_UDP
    Serial.printf("PCARS JSON Parsed: Speed=%.1f, Gear=%d, RPM=%d\n",
                 latestData.speed, latestData.gear, latestData.rpm);
    #endif
    
    return true;
}

bool PCARSTelemetryParser::parseBinaryUDP(const uint8_t* buffer, int size) {
    // Parse binary PCARS2 UDP data
    // Note: This is a simplified implementation. PCARS2 has a complex binary format
    // that varies between different packet types and game versions.
    
    // TODO: Implement full PCARS2 binary parsing
    // For now, this is a placeholder that attempts basic parsing
    
    if (size < 100) { // PCARS packets are typically much larger
        #if DEBUG_UDP
        Serial.printf("PCARS Binary: Packet too small for PCARS2 (%d bytes)\n", size);
        #endif
        return false;
    }
    
    // Attempt to identify PCARS2 packet by looking for known patterns
    // This is a very basic implementation and may not work with all PCARS2 versions
    
    // Check for potential build version at start (common in PCARS packets)
    uint32_t buildVersion = readUint32LE(buffer);
    
    if (buildVersion > 1000 && buildVersion < 10000) { // Reasonable build number range
        // Try to extract basic telemetry data from known approximate offsets
        // WARNING: These offsets are estimates and may not be accurate
        // for all PCARS2 versions. Use JSON forwarder for reliable data.
        
        // Check if we have enough data for basic parsing
        if (size < 40) {
            #if DEBUG_UDP
            Serial.println("PCARS Binary: Packet too small for parsing");
            #endif
            return false;
        }
        
        // These are rough estimates - actual offsets vary by packet type and version
        float speedMS = readFloatLE(buffer + 16);  // Speed in m/s (estimated offset)
        float rpm = readFloatLE(buffer + 24);      // RPM (estimated offset)
        int8_t gear = static_cast<int8_t>(buffer[32]); // Gear (estimated offset)
        
        // Sanity checks for reasonable values
        if (speedMS < 0 || speedMS > 200 || rpm < 0 || rpm > 20000 || gear < -1 || gear > 8) {
            #if DEBUG_UDP
            Serial.println("PCARS Binary: Invalid data values");
            #endif
            return false;
        }
        
        // Convert speed from m/s to km/h
        latestData.speed = speedMS * 3.6f;
        latestData.rpm = static_cast<int>(rpm);
        latestData.gear = static_cast<int>(gear);
        
        // Set defaults for data not easily extractable
        latestData.fuel = 50.0f; // Default fuel level
        latestData.lapTime = 0.0f; // Default lap time
        
        latestData.isForwarderData = false;
        latestData.dataValid = true;
        latestData.timestamp = millis();
        lastUpdateTime = latestData.timestamp;
        
        #if DEBUG_UDP
        Serial.printf("PCARS Binary Parsed: Speed=%.1f, Gear=%d, RPM=%d\n",
                     latestData.speed, latestData.gear, latestData.rpm);
        #endif
        
        return true;
    }
    
    #if DEBUG_UDP
    Serial.printf("PCARS Binary: Unrecognized packet format (build: %u)\n", buildVersion);
    #endif
    
    return false;
}

PCARSTelemetryData PCARSTelemetryParser::getLatestData() const {
    return latestData;
}

bool PCARSTelemetryParser::isDataValid() const {
    // Data is valid if we received it recently (within 5 seconds)
    return latestData.dataValid && (millis() - lastUpdateTime < 5000);
}

// Helper functions for endian handling
float PCARSTelemetryParser::readFloatLE(const uint8_t* data) {
    union {
        uint32_t i;
        float f;
    } converter;
    converter.i = readUint32LE(data);
    return converter.f;
}

uint32_t PCARSTelemetryParser::readUint32LE(const uint8_t* data) {
    return data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
}