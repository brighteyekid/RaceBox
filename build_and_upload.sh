#!/bin/bash

# ESP8266 Telemetry Dashboard Build and Upload Script
# This script builds and uploads the firmware to ESP8266

echo "ESP8266 Telemetry Dashboard Build Script"
echo "========================================"

# Check if PlatformIO is installed
PIO_CMD=""
if command -v pio &> /dev/null; then
    PIO_CMD="pio"
elif command -v platformio &> /dev/null; then
    PIO_CMD="platformio"
elif [ -f "/home/fox/.platformio/penv/bin/platformio" ]; then
    PIO_CMD="/home/fox/.platformio/penv/bin/platformio"
else
    echo "Error: PlatformIO is not installed or not found"
    echo "Install with: pip install platformio"
    exit 1
fi

echo "Using PlatformIO: $PIO_CMD"

# Navigate to project directory
cd "$(dirname "$0")"

echo "Building for ESP8266..."
$PIO_CMD run -e esp8266

if [ $? -eq 0 ]; then
    echo "Build successful!"
    
    # Ask if user wants to upload
    read -p "Upload to ESP8266? (y/n): " -n 1 -r
    echo
    if [[ $REPLY =~ ^[Yy]$ ]]; then
        echo "Uploading to ESP8266..."
        $PIO_CMD run -t upload -e esp8266
        
        if [ $? -eq 0 ]; then
            echo "Upload successful!"
            
            # Ask if user wants to monitor
            read -p "Start serial monitor? (y/n): " -n 1 -r
            echo
            if [[ $REPLY =~ ^[Yy]$ ]]; then
                echo "Starting serial monitor (Ctrl+C to exit)..."
                $PIO_CMD device monitor
            fi
        else
            echo "Upload failed!"
            exit 1
        fi
    fi
else
    echo "Build failed!"
    exit 1
fi

echo "Done!"