#include "display_manager_sh1106.h"

// Include the telemetry data structure from main.cpp
struct TelemetryData {
    float speed;
    int gear;
    int rpm;
    float fuel;
    float lapTime;
    int position;
    bool dataValid;
    unsigned long lastUpdate;
    String lastPacketType;
    int lastPacketSize;
    String sourceIP;
};

DisplayManagerSH1106::DisplayManagerSH1106() : u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE) {
}

bool DisplayManagerSH1106::begin() {
    Serial.println("Initializing SH1106 display...");
    Serial.printf("SDA Pin: %d, SCL Pin: %d\n", SDA_PIN, SCL_PIN);
    
    // Initialize I2C with custom pins
    Wire.begin(SDA_PIN, SCL_PIN);
    
    // Set I2C clock speed (try slower speed for better compatibility)
    Wire.setClock(100000); // 100kHz instead of default 400kHz
    
    Serial.println("I2C initialized, attempting SH1106 display connection...");
    
    if (!u8g2.begin()) {
        Serial.println("SH1106 initialization failed");
        Serial.println("Check wiring:");
        Serial.printf("  SDA: D1 (GPIO5) -> OLED SDA\n");
        Serial.printf("  SCL: D2 (GPIO4) -> OLED SCL\n");
        Serial.printf("  VCC: 3V3 -> OLED VCC\n");
        Serial.printf("  GND: GND -> OLED GND\n");
        return false;
    }
    
    Serial.println("SH1106 Display initialized successfully!");
    
    // Set font and show startup message
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 10, "ESP8266 Telemetry");
    u8g2.drawStr(0, 25, "Dashboard");
    u8g2.drawStr(0, 40, "Starting...");
    u8g2.sendBuffer();
    
    return true;
}

void DisplayManagerSH1106::showPage(int pageNumber, const TelemetryData& data, int gameType) {
    Serial.printf("DisplayManagerSH1106::showPage - Page: %d, DataValid: %s\n", 
                  pageNumber, data.dataValid ? "true" : "false");
    
    u8g2.clearBuffer();
    
    switch (pageNumber) {
        case PAGE_SPEED_GEAR:
            showSpeedGearPage(data);
            break;
        case PAGE_LAP_FUEL:
            showLapFuelPage(data);
            break;
        case PAGE_DEBUG:
            showDebugPage(data);
            break;
        case PAGE_SETTINGS:
            showSettingsPage(gameType);
            break;
        default:
            u8g2.setFont(u8g2_font_6x10_tf);
            u8g2.drawStr(0, 20, "Invalid Page");
            break;
    }
    
    u8g2.sendBuffer();
}

void DisplayManagerSH1106::showSpeedGearPage(const TelemetryData& data) {
    // Page 1: Big speed (left) + gear (right) + RPM bar (bottom)
    
    if (!data.dataValid) {
        drawCenteredText("NO DATA", 25);
        drawCenteredText("Waiting...", 40);
        return;
    }
    
    // Speed on the left (large text)
    u8g2.setFont(u8g2_font_logisoso20_tn);
    char speedStr[8];
    snprintf(speedStr, sizeof(speedStr), "%d", (int)data.speed);
    u8g2.drawStr(5, 25, speedStr);
    
    // "km/h" label
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(5, 35, "km/h");
    
    // Gear on the right
    u8g2.setFont(u8g2_font_logisoso20_tn);
    char gearStr[4];
    if (data.gear == 0) {
        snprintf(gearStr, sizeof(gearStr), "N");
    } else if (data.gear == -1) {
        snprintf(gearStr, sizeof(gearStr), "R");
    } else {
        snprintf(gearStr, sizeof(gearStr), "%d", data.gear);
    }
    u8g2.drawStr(90, 25, gearStr);
    
    // "GEAR" label
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(90, 35, "GEAR");
    
    // RPM bar at bottom
    drawRPMBar(data.rpm);
}

void DisplayManagerSH1106::showLapFuelPage(const TelemetryData& data) {
    if (!data.dataValid) {
        drawCenteredText("NO DATA", 32);
        return;
    }
    
    u8g2.setFont(u8g2_font_6x10_tf);
    
    // Lap time
    u8g2.drawStr(0, 15, "LAP TIME:");
    String lapTimeStr = formatTime(data.lapTime);
    u8g2.drawStr(0, 30, lapTimeStr.c_str());
    
    // Fuel
    u8g2.drawStr(0, 45, "FUEL:");
    String fuelStr = formatFloat(data.fuel, 1) + "%";
    u8g2.drawStr(0, 60, fuelStr.c_str());
}

void DisplayManagerSH1106::showDebugPage(const TelemetryData& data) {
    u8g2.setFont(u8g2_font_5x7_tf);
    
    // Connection info
    u8g2.drawStr(0, 8, "DEBUG INFO");
    
    String validStr = "Valid: " + String(data.dataValid ? "YES" : "NO");
    u8g2.drawStr(0, 18, validStr.c_str());
    
    if (data.dataValid) {
        String lastUpdateStr = "Age: " + String((millis() - data.lastUpdate) / 1000) + "s";
        u8g2.drawStr(0, 28, lastUpdateStr.c_str());
        
        String packetStr = "Type: " + data.lastPacketType;
        u8g2.drawStr(0, 38, packetStr.c_str());
        
        String sizeStr = "Size: " + String(data.lastPacketSize) + "b";
        u8g2.drawStr(0, 48, sizeStr.c_str());
        
        String ipStr = "From: " + data.sourceIP;
        u8g2.drawStr(0, 58, ipStr.c_str());
    }
}

void DisplayManagerSH1106::showSettingsPage(int gameType) {
    u8g2.setFont(u8g2_font_6x10_tf);
    
    drawCenteredText("SETTINGS", 15);
    
    String gameStr = "Game: " + String(gameType == 0 ? "F1" : "PCARS");
    drawCenteredText(gameStr, 35);
    
    drawCenteredText("Use buttons to", 50);
    drawCenteredText("navigate", 60);
}

void DisplayManagerSH1106::drawRPMBar(int rpm, int maxRPM) {
    // RPM bar at bottom of screen
    int barY = 50;
    int barHeight = 8;
    int barWidth = 120;
    int barX = 4;
    
    // Background
    u8g2.drawFrame(barX, barY, barWidth, barHeight);
    
    // Fill based on RPM
    if (rpm > 0 && maxRPM > 0) {
        int fillWidth = (rpm * (barWidth - 2)) / maxRPM;
        fillWidth = constrain(fillWidth, 0, barWidth - 2);
        
        if (fillWidth > 0) {
            u8g2.drawBox(barX + 1, barY + 1, fillWidth, barHeight - 2);
        }
    }
}

void DisplayManagerSH1106::drawCenteredText(const String& text, int y) {
    u8g2.setFont(u8g2_font_6x10_tf);
    int textWidth = u8g2.getStrWidth(text.c_str());
    int x = (SCREEN_WIDTH - textWidth) / 2;
    u8g2.drawStr(x, y, text.c_str());
}

void DisplayManagerSH1106::drawRightAlignedText(const String& text, int x, int y) {
    u8g2.setFont(u8g2_font_6x10_tf);
    int textWidth = u8g2.getStrWidth(text.c_str());
    u8g2.drawStr(x - textWidth, y, text.c_str());
}

String DisplayManagerSH1106::formatTime(float seconds) {
    // Format time as MM:SS.mmm
    int minutes = (int)(seconds / 60);
    float remainingSeconds = seconds - (minutes * 60);
    
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%d:%06.3f", minutes, remainingSeconds);
    return String(buffer);
}

String DisplayManagerSH1106::formatFloat(float value, int decimals) {
    char buffer[16];
    char format[8];
    snprintf(format, sizeof(format), "%%.%df", decimals);
    snprintf(buffer, sizeof(buffer), format, value);
    return String(buffer);
}

void DisplayManagerSH1106::showStatus(const String& message) {
    u8g2.clearBuffer();
    drawCenteredText(message, 32);
    u8g2.sendBuffer();
}

void DisplayManagerSH1106::clear() {
    u8g2.clearBuffer();
    u8g2.sendBuffer();
}

void DisplayManagerSH1106::update() {
    u8g2.sendBuffer();
}