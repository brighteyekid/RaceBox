#ifndef DISPLAY_MANAGER_SH1106_H
#define DISPLAY_MANAGER_SH1106_H

#include <U8g2lib.h>
#include <Wire.h>
#include "config.h"

// Forward declaration for telemetry data
struct TelemetryData;

class DisplayManagerSH1106 {
public:
    DisplayManagerSH1106();
    bool begin();
    void showPage(int pageNumber, const TelemetryData& data, int gameType);
    void showStatus(const String& message);
    void clear();
    void update();
    
private:
    U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2;
    
    // Page rendering functions
    void showSpeedGearPage(const TelemetryData& data);
    void showLapFuelPage(const TelemetryData& data);
    void showDebugPage(const TelemetryData& data);
    void showSettingsPage(int gameType);
    
    // Helper functions
    void drawRPMBar(int rpm, int maxRPM = 8000);
    void drawCenteredText(const String& text, int y);
    void drawRightAlignedText(const String& text, int x, int y);
    String formatTime(float seconds);
    String formatFloat(float value, int decimals = 1);
    
    // Display constants (using config.h values)
};

#endif // DISPLAY_MANAGER_SH1106_H