/*
 * I2C Scanner for ESP8266
 * This code scans for I2C devices and reports their addresses
 * Useful for debugging OLED display connections
 */

#include <Arduino.h>
#include <Wire.h>

// Pin definitions for ESP8266 NodeMCU
#define SDA_PIN D1  // GPIO5
#define SCL_PIN D2  // GPIO4

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("\nI2C Scanner for ESP8266");
    Serial.println("========================");
    Serial.printf("SDA Pin: D1 (GPIO5)\n");
    Serial.printf("SCL Pin: D2 (GPIO4)\n");
    Serial.println();
    
    // Initialize I2C
    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(100000); // 100kHz
    
    Serial.println("Scanning for I2C devices...");
    Serial.println();
}

void loop() {
    byte error, address;
    int nDevices = 0;
    
    Serial.println("Scanning...");
    
    for(address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
        
        if (error == 0) {
            Serial.printf("I2C device found at address 0x%02X\n", address);
            nDevices++;
        }
        else if (error == 4) {
            Serial.printf("Unknown error at address 0x%02X\n", address);
        }
    }
    
    if (nDevices == 0) {
        Serial.println("No I2C devices found");
        Serial.println("Check wiring:");
        Serial.println("  D1 (GPIO5) -> OLED SDA");
        Serial.println("  D2 (GPIO4) -> OLED SCL");
        Serial.println("  3V3 -> OLED VCC");
        Serial.println("  GND -> OLED GND");
    }
    else {
        Serial.printf("Found %d device(s)\n", nDevices);
        Serial.println("Expected OLED address: 0x3C");
    }
    
    Serial.println();
    delay(5000); // Wait 5 seconds before next scan
}