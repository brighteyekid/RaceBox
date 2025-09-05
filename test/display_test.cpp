/*
 * Simple Display Test for ESP8266
 * Tests OLED display initialization without WiFi
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Pin definitions for ESP8266 NodeMCU
#define SDA_PIN D1  // GPIO5
#define SCL_PIN D2  // GPIO4

// Display configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup() {
    Serial.begin(115200);
    delay(2000); // Give serial time to initialize
    
    Serial.println("\nESP8266 Display Test");
    Serial.println("===================");
    Serial.printf("SDA Pin: D1 (GPIO5)\n");
    Serial.printf("SCL Pin: D2 (GPIO4)\n");
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
    Serial.println();
    
    // Initialize I2C
    Serial.println("Initializing I2C...");
    Wire.begin(SDA_PIN, SCL_PIN);
    Wire.setClock(100000); // 100kHz for better compatibility
    
    // Test I2C scanner first
    Serial.println("Scanning for I2C devices...");
    byte error, address;
    int nDevices = 0;
    
    for(address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();
        
        if (error == 0) {
            Serial.printf("I2C device found at address 0x%02X\n", address);
            nDevices++;
        }
    }
    
    if (nDevices == 0) {
        Serial.println("No I2C devices found!");
        Serial.println("Check wiring:");
        Serial.println("  D1 (GPIO5) -> OLED SDA");
        Serial.println("  D2 (GPIO4) -> OLED SCL");
        Serial.println("  3V3 -> OLED VCC");
        Serial.println("  GND -> OLED GND");
    } else {
        Serial.printf("Found %d I2C device(s)\n", nDevices);
    }
    
    Serial.println();
    
    // Try to initialize display
    Serial.println("Attempting display initialization...");
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println("SSD1306 allocation failed!");
        Serial.println("Trying alternative address 0x3D...");
        
        if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3D)) {
            Serial.println("Display initialization failed with both addresses!");
            return;
        } else {
            Serial.println("Display found at address 0x3D!");
        }
    } else {
        Serial.println("Display initialized successfully at 0x3C!");
    }
    
    // Test display
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("ESP8266 Test");
    display.println("Display OK!");
    display.printf("Heap: %d", ESP.getFreeHeap());
    display.display();
    
    Serial.println("Display test complete!");
}

void loop() {
    // Blink display every 2 seconds
    static unsigned long lastBlink = 0;
    static bool displayOn = true;
    
    if (millis() - lastBlink > 2000) {
        lastBlink = millis();
        displayOn = !displayOn;
        
        if (displayOn) {
            display.ssd1306_command(SSD1306_DISPLAYON);
            Serial.println("Display ON");
        } else {
            display.ssd1306_command(SSD1306_DISPLAYOFF);
            Serial.println("Display OFF");
        }
    }
    
    delay(100);
}