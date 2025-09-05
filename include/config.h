#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
#define WIFI_SSID "Fox"
#define WIFI_PASSWORD "12345678"
#define WIFI_TIMEOUT_MS 10000
#define WIFI_RECONNECT_INTERVAL_MS 30000

// UDP Configuration
#define F1_UDP_PORT 20777
#define PCARS_UDP_PORT 5606
#define PCARS_FORWARDER_PORT 20778
#define UDP_BUFFER_SIZE 2048
#define UDP_TIMEOUT_MS 100

// Display Configuration (SH1106 128x64)
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

// Pin definitions based on board type
#ifdef ESP8266_BOARD
    // I2C Pins (ESP8266 NodeMCU - reversed)
    #define SDA_PIN D1  // GPIO5
    #define SCL_PIN D2  // GPIO4
    
    // Button Pins
    #define BUTTON_NEXT_PIN D5  // GPIO14
    #define BUTTON_SELECT_PIN D6  // GPIO12
#else
    // I2C Pins (ESP32 defaults)
    #define SDA_PIN 21
    #define SCL_PIN 22
    
    // Button Pins
    #define BUTTON_NEXT_PIN 18
    #define BUTTON_SELECT_PIN 19
#endif
#define BUTTON_DEBOUNCE_MS 50

// Display Pages
#define PAGE_SPEED_GEAR 0
#define PAGE_LAP_FUEL 1
#define PAGE_DEBUG 2
#define PAGE_SETTINGS 3
#define MAX_PAGES 4

// Game Types
#define GAME_F1 0
#define GAME_PCARS 1

// F1 2020 Packet Constants
#define F1_PACKET_FORMAT_2020 2020
#define F1_PACKET_ID_CAR_TELEMETRY 6
#define F1_MAX_CARS 22
#define F1_PACKET_HEADER_SIZE 25
#define F1_CAR_TELEMETRY_SIZE 60

// Debug
#define DEBUG_SERIAL 1
#define DEBUG_UDP 1  // Enable UDP packet debugging

#endif // CONFIG_H