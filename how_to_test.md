# How to Test the ESP8266 Telemetry Dashboard

This guide provides step-by-step instructions for testing the ESP8266 telemetry dashboard with both F1 2020 and Project CARS 2.

## Prerequisites

- ESP8266 flashed with the telemetry dashboard firmware
- Python 3.6+ installed on your PC
- ESP8266 and PC connected to the same network (hotspot "fox" recommended)

## Quick Test Setup

### 1. Flash the ESP8266

```bash
# Using PlatformIO
cd /path/to/PLAN_FOR_HER
pio run -t upload -e esp8266

# Monitor serial output to get ESP8266 IP
pio device monitor
```

**Expected Output**:
```
ESP8266 F1/PCARS Telemetry Dashboard Starting...
Connecting to WiFi: fox
WiFi connected!
IP address: 192.168.43.100
Setup complete!
```

### 2. Note the ESP8266 IP Address

The ESP8266 will display its IP address on both:
- Serial monitor
- OLED display (briefly during startup)

Common IP addresses:
- Hotspot "fox": `192.168.43.100`
- Home router: `192.168.1.xxx` or `192.168.0.xxx`

## F1 2020 Testing

### Test 1: F1 Simulator (No Game Required)

1. **Run the F1 simulator**:
   ```bash
   cd test
   python sim_send_f1.py 192.168.43.100
   ```
   
   Replace `192.168.43.100` with your ESP32's actual IP.

2. **Expected Output**:
   ```
   F1 2020 UDP Telemetry Simulator
   Target ESP32: 192.168.43.100:20777
   Press Ctrl+C to stop

   Frame      1: Speed= 180.5 km/h, Gear=4, RPM=6500, Size=1347 bytes
   Frame      2: Speed= 185.2 km/h, Gear=4, RPM=6650, Size=1347 bytes
   Frame      3: Speed= 190.1 km/h, Gear=5, RPM=6200, Size=1347 bytes
   ```

3. **ESP32 Display Should Show**:
   - Page 1: Large speed and gear numbers, RPM bar at bottom
   - Speed changing from 50-300 km/h
   - Gear changing 2-7 based on speed
   - RPM bar filling/emptying

4. **Validation Steps**:
   - Press Next button to cycle through pages
   - Page 2: Should show lap time as "0:00.000" (simulator doesn't send lap data)
   - Page 3: Should show "F1 CarTelemetry", packet size 1347, source IP
   - Page 4: Should show "GAME: F1 2020"

### Test 2: Real F1 2020 Game

1. **Configure F1 2020**:
   - Settings → Telemetry Settings
   - UDP Telemetry: ON
   - UDP IP Address: `192.168.43.100` (your ESP32 IP)
   - UDP Port: `20777`
   - UDP Send Rate: `20Hz`
   - UDP Format: `2020`

2. **Start a Session**:
   - Practice, Qualifying, or Race mode
   - Telemetry only works during active sessions

3. **Expected Behavior**:
   - Real-time speed, gear, and RPM updates
   - Data should match what you see in-game
   - Smooth updates at 20Hz

## Project CARS 2 Testing

### Test 1: PCARS Simulator (No Game Required)

1. **Run the PCARS simulator**:
   ```bash
   cd test
   python pcars_forwarder.py 192.168.43.100 --simulate
   ```

2. **Expected Output**:
   ```
   Project CARS 2 UDP Forwarder
   Target ESP32: 192.168.43.100:20778
   Mode: Simulation
   Press Ctrl+C to stop

   Frame      1: Speed= 165.2 km/h, Gear=4, RPM=6800, Fuel= 94.5%
   Frame      2: Speed= 170.1 km/h, Gear=5, RPM=6200, Fuel= 94.4%
   Frame      3: Speed= 175.8 km/h, Gear=5, RPM=6400, Fuel= 94.4%
   ```

3. **ESP32 Display Should Show**:
   - Switch to PCARS mode: Page 4 → Press Select → "GAME: PCARS 2"
   - Page 1: Speed 60-280 km/h, gear 2-6, RPM bar
   - Page 2: Fuel percentage decreasing over time

### Test 2: Real Project CARS 2 Game

1. **Configure PCARS2**:
   - Options → System
   - UDP Frequency: `10`
   - UDP Protocol Version: `2`
   - UDP Port: `5606`

2. **Run the Forwarder**:
   ```bash
   cd test
   python pcars_forwarder.py 192.168.43.100
   ```

3. **Start PCARS2 Session**:
   - Any race or practice mode
   - Forwarder should start receiving data

4. **Expected Output**:
   ```
   Project CARS 2 UDP Forwarder
   Target ESP32: 192.168.43.100:20778
   Mode: UDP Listening
   Listening for PCARS2 UDP on port 5606
   
   Packet      1: Speed= 120.5 km/h, Gear=3, RPM=5500, Fuel= 85.2%
   ```

## Validation Checklist

### Display Functionality
- [ ] OLED shows startup message
- [ ] WiFi connection status displayed
- [ ] IP address shown briefly
- [ ] All 4 pages accessible with Next button
- [ ] Settings page allows game switching with Select button

### F1 2020 Data
- [ ] Speed updates in real-time (0-300+ km/h)
- [ ] Gear shows correctly (R, N, 1-8)
- [ ] RPM bar fills proportionally
- [ ] Debug page shows "F1 CarTelemetry"
- [ ] Packet size is 1347 bytes
- [ ] Source IP matches sender

### PCARS2 Data
- [ ] Speed updates in real-time (0-280+ km/h)
- [ ] Gear shows correctly (R, N, 1-6)
- [ ] RPM bar fills proportionally
- [ ] Fuel percentage decreases over time
- [ ] Debug page shows "PCARS JSON"
- [ ] Source IP matches PC running forwarder

### Button Controls
- [ ] Next button cycles through pages (1→2→3→4→1)
- [ ] Select button changes game on settings page
- [ ] Buttons are debounced (no double-presses)

## Troubleshooting Test Issues

### No Data on Display

1. **Check Network Connection**:
   ```bash
   # Ping ESP32
   ping 192.168.43.100
   
   # Test UDP port (Linux/Mac)
   nc -u 192.168.43.100 20777
   ```

2. **Verify Simulator Output**:
   - Should show "Frame X: Speed=..." messages
   - If no output, check Python installation
   - If "connection refused", check ESP32 IP

3. **Check ESP32 Serial Output**:
   ```bash
   pio device monitor
   ```
   Look for:
   - "F1 UDP: X bytes from Y.Y.Y.Y"
   - "F1 Data: Speed=X, Gear=Y, RPM=Z"

### Wrong Data Values

1. **F1 Simulator Issues**:
   - Speed should vary 50-300 km/h
   - Gear should be 2-7
   - RPM should be 1000-8000

2. **PCARS Simulator Issues**:
   - Speed should vary 60-280 km/h
   - Gear should be 2-6
   - Fuel should decrease slowly

### Display Problems

1. **Blank Display**:
   - Check I2C wiring (SDA=21, SCL=22)
   - Verify 3.3V power connection
   - Try I2C scanner code

2. **Garbled Display**:
   - Check display address (0x3C)
   - Verify display size (128x32)
   - Check for loose connections

### Network Issues

1. **ESP32 Not Connecting**:
   - Verify SSID/password in config.h
   - Check hotspot is active
   - Try different network

2. **UDP Not Received**:
   - Check firewall settings
   - Verify ESP32 and PC on same network
   - Try different UDP port

## Performance Validation

### Expected Performance Metrics

- **Update Rate**: 10-20 Hz (display should update smoothly)
- **Latency**: <100ms from simulator to display
- **Memory Usage**: Check serial output for heap info
- **Packet Loss**: Should be minimal (<1%)

### Performance Tests

1. **High Frequency Test**:
   ```bash
   # Modify sim_send_f1.py to send at 50Hz
   # Change: time.sleep(0.05) to time.sleep(0.02)
   python sim_send_f1.py 192.168.43.100
   ```

2. **Long Duration Test**:
   - Run simulator for 30+ minutes
   - Check for memory leaks
   - Verify WiFi stability

3. **Range Test**:
   - Move ESP32 away from router/hotspot
   - Test maximum reliable range
   - Check for reconnection behavior

## Test Completion

When all tests pass, you should have:

✅ **Working F1 2020 Integration**:
- Real-time telemetry display
- Accurate speed, gear, RPM data
- Proper packet parsing

✅ **Working PCARS2 Integration**:
- JSON forwarder operational
- Real-time telemetry display
- Fuel and lap time data

✅ **Robust Hardware**:
- Stable WiFi connection
- Responsive button controls
- Clear OLED display

✅ **Production Ready**:
- Auto-reconnect functionality
- Error handling
- Debug information available

## Next Steps

After successful testing:

1. **Customize Configuration**:
   - Adjust display pages
   - Modify update rates
   - Add new telemetry fields

2. **Deploy for Racing**:
   - Mount ESP32 and display
   - Configure for your network
   - Test with actual games

3. **Extend Functionality**:
   - Add more games
   - Implement data logging
   - Create mobile app interface