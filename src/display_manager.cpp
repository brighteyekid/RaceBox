#include "display_manager.h"

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

DisplayManager::DisplayManager() : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET) {
}

bool DisplayManager::begin() {
    Serial.println("Initializing display...");
    Serial.printf("SDA Pin: %d, SCL Pin: %d\n", SDA_PIN, SCL_PIN);
    
    // Initialize I2C with custom pins
    Wire.begin(SDA_PIN, SCL_PIN);
    
    // Set I2C clock speed (try slower speed for better compatibility)
    Wire.setClock(100000); // 100kHz instead of default 400kHz
    
    Serial.println("I2C initialized, attempting display connection...");
    
    if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println("SSD1306 allocation failed");
        Serial.println("Check wiring:");
        Serial.printf("  SDA: D1 (GPIO5) -> OLED SDA\n");
        Serial.printf("  SCL: D2 (GPIO4) -> OLED SCL\n");
        Serial.printf("  VCC: 3V3 -> OLED VCC\n");
        Serial.printf("  GND: GND -> OLED GND\n");
        return false;
    }
    
    Serial.println("Display initialized successfully!");
    
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.setTextSize(1);
    #ifdef ESP8266_BOARD
    display.println("ESP8266 Telemetry");
    #else
    display.println("ESP32 Telemetry");
    #endif
    display.println("Dashboard");
    display.println("Starting...");
    display.display();
    
    return true;
}

void DisplayManager::showPage(int pageNumber, const TelemetryData& data, int gameType) {
    Serial.printf("DisplayManager::showPage - Page: %d, DataValid: %s\n", 
                  pageNumber, data.dataValid ? "true" : "false");
    
    display.clearDisplay();
    
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
            display.setCursor(0, 0);
            display.setTextSize(1);
            display.println("Invalid Page");
            break;
    }
    
    display.display();
}

void DisplayManager::showSpeedGearPage(const TelemetryData& data) {
    // Page 1: Big speed (left) + gear (right) + RPM bar (bottom)
    
    if (!data.dataValid) {
        drawCenteredText("NO DATA", 8, 1);
        drawCenteredText("Waiting...", 18, 1);
        return;
    }
    
    // Speed on the left (large text)
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.print(String((int)data.speed));
    
    // Speed unit
    display.setTextSize(1);
    display.setCursor(0, 16);
    display.print("km/h");
    
    // Gear on the right (large text)
    display.setTextSize(2);
    String gearStr;
    if (data.gear == 0) {
        gearStr = "N";
    } else if (data.gear == -1) {
        gearStr = "R";
    } else {
        gearStr = String(data.gear);
    }
    
    // Right align gear
    int gearWidth = gearStr.length() * LARGE_CHAR_WIDTH;
    display.setCursor(SCREEN_WIDTH - gearWidth, 0);
    display.print(gearStr);
    
    // Gear label
    display.setTextSize(1);
    display.setCursor(SCREEN_WIDTH - 24, 16);
    display.print("GEAR");
    
    // RPM bar at bottom
    drawRPMBar(data.rpm);
}

void DisplayManager::showLapFuelPage(const TelemetryData& data) {
    // Page 2: Lap time, fuel %, position
    
    if (!data.dataValid) {
        drawCenteredText("NO DATA", 12, 1);
        return;
    }
    
    display.setTextSize(1);
    
    // Lap time
    display.setCursor(0, 0);
    display.print("LAP: ");
    if (data.lapTime > 0) {
        display.print(formatTime(data.lapTime));
    } else {
        display.print("--:--.---");
    }
    
    // Fuel percentage
    display.setCursor(0, 10);
    display.print("FUEL: ");
    if (data.fuel > 0) {
        display.print(formatFloat(data.fuel, 1));
        display.print("%");
    } else {
        display.print("--.--%");
    }
    
    // Position (if available)
    display.setCursor(0, 20);
    display.print("POS: ");
    if (data.position > 0) {
        display.print(data.position);
    } else {
        display.print("--");
    }
    
    // Speed (smaller, right side)
    String speedStr = String((int)data.speed) + " km/h";
    drawRightAlignedText(speedStr, SCREEN_WIDTH, 0, 1);
    
    // RPM (smaller, right side)
    String rpmStr = String(data.rpm) + " RPM";
    drawRightAlignedText(rpmStr, SCREEN_WIDTH, 10, 1);
}

void DisplayManager::showDebugPage(const TelemetryData& data) {
    // Page 3: Raw debug info
    
    display.setTextSize(1);
    
    // Last packet type
    display.setCursor(0, 0);
    display.print("PKT: ");
    display.print(data.lastPacketType);
    
    // Packet size
    display.setCursor(0, 8);
    display.print("SIZE: ");
    display.print(data.lastPacketSize);
    display.print(" bytes");
    
    // Source IP
    display.setCursor(0, 16);
    display.print("FROM: ");
    display.print(data.sourceIP);
    
    // Data age
    display.setCursor(0, 24);
    display.print("AGE: ");
    if (data.dataValid) {
        unsigned long age = (millis() - data.lastUpdate) / 1000;
        display.print(age);
        display.print("s");
    } else {
        display.print("STALE");
    }
}

void DisplayManager::showSettingsPage(int gameType) {
    // Page 4: Settings
    
    display.setTextSize(1);
    
    // Title
    drawCenteredText("SETTINGS", 0, 1);
    
    // Game type selection
    display.setCursor(0, 12);
    display.print("GAME: ");
    if (gameType == GAME_F1) {
        display.print("F1 2020");
    } else {
        display.print("PCARS 2");
    }
    
    // Instructions
    display.setCursor(0, 24);
    display.print("SELECT=Change");
}

void DisplayManager::drawRPMBar(int rpm, int maxRPM) {
    // Draw RPM bar at bottom of screen
    int barY = SCREEN_HEIGHT - 4;
    int barHeight = 3;
    int barWidth = SCREEN_WIDTH - 2;
    
    // Background
    display.drawRect(1, barY, barWidth, barHeight, SSD1306_WHITE);
    
    // Fill based on RPM
    if (rpm > 0 && maxRPM > 0) {
        int fillWidth = (rpm * (barWidth - 2)) / maxRPM;
        fillWidth = constrain(fillWidth, 0, barWidth - 2);
        
        if (fillWidth > 0) {
            display.fillRect(2, barY + 1, fillWidth, barHeight - 2, SSD1306_WHITE);
        }
    }
}

void DisplayManager::drawCenteredText(const String& text, int y, int textSize) {
    display.setTextSize(textSize);
    int textWidth = text.length() * CHAR_WIDTH * textSize;
    int x = (SCREEN_WIDTH - textWidth) / 2;
    display.setCursor(x, y);
    display.print(text);
}

void DisplayManager::drawRightAlignedText(const String& text, int x, int y, int textSize) {
    display.setTextSize(textSize);
    int textWidth = text.length() * CHAR_WIDTH * textSize;
    display.setCursor(x - textWidth, y);
    display.print(text);
}

String DisplayManager::formatTime(float seconds) {
    // Format time as MM:SS.mmm
    int minutes = (int)(seconds / 60);
    float remainingSeconds = seconds - (minutes * 60);
    
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%d:%06.3f", minutes, remainingSeconds);
    return String(buffer);
}

String DisplayManager::formatFloat(float value, int decimals) {
    char buffer[16];
    char format[8];
    snprintf(format, sizeof(format), "%%.%df", decimals);
    snprintf(buffer, sizeof(buffer), format, value);
    return String(buffer);
}

void DisplayManager::showStatus(const String& message) {
    display.clearDisplay();
    drawCenteredText(message, 12, 1);
    display.display();
}

void DisplayManager::clear() {
    display.clearDisplay();
    display.display();
}

void DisplayManager::update() {
    display.display();
}