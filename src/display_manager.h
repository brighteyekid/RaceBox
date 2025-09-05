#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include "config.h"

// Forward declaration for telemetry data
struct TelemetryData;

class DisplayManager {
public:
    DisplayManager();
    bool begin();
    void showPage(int pageNumber, const TelemetryData& data, int gameType);
    void showStatus(const String& message);
    void clear();
    void update();
    
private:
    Adafruit_SSD1306 display;
    
    // Page rendering functions
    void showSpeedGearPage(const TelemetryData& data);
    void showLapFuelPage(const TelemetryData& data);
    void showDebugPage(const TelemetryData& data);
    void showSettingsPage(int gameType);
    
    // Helper functions
    void drawRPMBar(int rpm, int maxRPM = 8000);
    void drawCenteredText(const String& text, int y, int textSize = 1);
    void drawRightAlignedText(const String& text, int x, int y, int textSize = 1);
    String formatTime(float seconds);
    String formatFloat(float value, int decimals = 1);
    
    // Display constants
    static const int CHAR_WIDTH = 6;
    static const int CHAR_HEIGHT = 8;
    static const int LARGE_CHAR_WIDTH = 12;
    static const int LARGE_CHAR_HEIGHT = 16;
};

#endif // DISPLAY_MANAGER_H