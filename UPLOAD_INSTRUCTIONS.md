# Upload Instructions

## You have an ESP8266 chip!

The error message shows "This chip is ESP8266, not ESP32", which means you need to use the ESP8266 environment.

## Correct Upload Commands

### For ESP8266 (Your chip):
```bash
pio run -t upload -e esp8266
```

### For ESP32 (if you had an ESP32):
```bash
pio run -t upload -e esp32dev
```

## Quick Build and Upload Script

Use the provided script for ESP8266:
```bash
./build_and_upload.sh
```

## Monitor Serial Output

After successful upload:
```bash
pio device monitor
```

## What Changed

The code is now compatible with both ESP32 and ESP8266:
- Conditional compilation based on board type
- ESP8266-specific optimizations (yield() calls, smaller buffers)
- Correct pin definitions for each board
- Platform-specific WiFi libraries

## Pin Mapping for ESP8266 (NodeMCU) - UPDATED

**New wiring (pins reversed):**
- D1 (GPIO5) → OLED SDA
- D2 (GPIO4) → OLED SCL  
- D5 (GPIO14) → NEXT Button
- D6 (GPIO12) → SELECT Button
- 3V3 → OLED VCC
- GND → OLED GND

## Troubleshooting Display Issues

If the display still doesn't work:

1. **Check connections** - Make sure wires are firmly connected
2. **Try different OLED** - Some displays have different addresses
3. **Run I2C scanner**:
   ```bash
   # Copy test/i2c_scanner.cpp to src/main.cpp temporarily
   # Build and upload to scan for I2C devices
   ```
4. **Check power** - ESP8266 needs stable 3.3V power
5. **Try different I2C address** - Some OLEDs use 0x3D instead of 0x3C