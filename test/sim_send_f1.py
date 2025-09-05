#!/usr/bin/env python3
"""
F1 2020 UDP Telemetry Simulator
Sends simulated F1 2020 car telemetry packets to ESP32 for testing.

Usage:
    python sim_send_f1.py [ESP8266_IP]

Default ESP8266 IP: 192.168.43.100 (adjust for your hotspot)
"""

import socket
import struct
import time
import sys
import math

# F1 2020 UDP Configuration
F1_UDP_PORT = 20777
PACKET_FORMAT_2020 = 2020
PACKET_ID_CAR_TELEMETRY = 6
MAX_CARS = 22

# Default ESP8266 IP (adjust for your network)
DEFAULT_ESP8266_IP = "172.20.10.14"

def create_packet_header(packet_id, session_time, frame_id, player_car_index=0):
    """Create F1 2020 packet header (24 bytes)"""
    return struct.pack('<HBBBBBQfLBB',
        PACKET_FORMAT_2020,    # m_packetFormat (uint16)
        2,                     # m_gameMajorVersion (uint8)
        20,                    # m_gameMinorVersion (uint8)
        1,                     # m_packetVersion (uint8)
        packet_id,             # m_packetId (uint8)
        0,                     # padding
        12345678901234567,     # m_sessionUID (uint64)
        session_time,          # m_sessionTime (float)
        frame_id,              # m_frameIdentifier (uint32)
        player_car_index,      # m_playerCarIndex (uint8)
        255                    # m_secondaryPlayerCarIndex (uint8)
    )

def create_car_telemetry_data(speed_kmh, gear, rpm, throttle=0.5, brake=0.0):
    """Create car telemetry data (60 bytes per car)"""
    # Convert gear: 1-8 normal, 0=neutral, -1=reverse
    gear_value = gear
    
    # Brake temperatures (4 wheels)
    brake_temps = [350, 350, 350, 350]
    
    # Tyre surface temperatures (4 wheels)
    tyre_surface_temps = [85, 85, 85, 85]
    
    # Tyre inner temperatures (4 wheels)
    tyre_inner_temps = [90, 90, 90, 90]
    
    # Tyre pressures (4 wheels)
    tyre_pressures = [23.5, 23.5, 23.5, 23.5]
    
    # Surface types (4 wheels) - 0 = tarmac
    surface_types = [0, 0, 0, 0]
    
    return struct.pack('<HfffBbHBB4H4B4BHffff4B',
        int(speed_kmh),        # m_speed (uint16)
        throttle,              # m_throttle (float)
        0.0,                   # m_steer (float)
        brake,                 # m_brake (float)
        0,                     # m_clutch (uint8)
        gear_value,            # m_gear (int8)
        rpm,                   # m_engineRPM (uint16)
        0,                     # m_drs (uint8)
        int((rpm / 8000.0) * 100),  # m_revLightsPercent (uint8)
        *brake_temps,          # m_brakesTemperature[4] (uint16[4])
        *tyre_surface_temps,   # m_tyresSurfaceTemperature[4] (uint8[4])
        *tyre_inner_temps,     # m_tyresInnerTemperature[4] (uint8[4])
        750,                   # m_engineTemperature (uint16)
        *tyre_pressures,       # m_tyresPressure[4] (float[4])
        *surface_types         # m_surfaceType[4] (uint8[4])
    )

def create_car_telemetry_packet(speed_kmh, gear, rpm, session_time, frame_id):
    """Create complete F1 2020 car telemetry packet"""
    # Create header
    header = create_packet_header(PACKET_ID_CAR_TELEMETRY, session_time, frame_id)
    
    # Create telemetry data for all cars (player car at index 0)
    car_data_list = []
    for i in range(MAX_CARS):
        if i == 0:  # Player car
            car_data = create_car_telemetry_data(speed_kmh, gear, rpm)
        else:  # Other cars (dummy data)
            car_data = create_car_telemetry_data(100, 3, 5000)
        car_data_list.append(car_data)
    
    # Additional packet data
    button_status = 0          # No buttons pressed
    mfd_panel_index = 255      # MFD closed
    mfd_panel_index_secondary = 255
    suggested_gear = gear if gear > 0 else 1
    
    # Combine all data
    packet = header
    for car_data in car_data_list:
        packet += car_data
    
    packet += struct.pack('<IBBb',
        button_status,
        mfd_panel_index,
        mfd_panel_index_secondary,
        suggested_gear
    )
    
    return packet

def simulate_driving_data(time_elapsed):
    """Generate realistic driving data based on time"""
    # Simulate a lap with varying speed, gear, and RPM
    lap_progress = (time_elapsed % 90.0) / 90.0  # 90-second lap
    
    # Speed varies from 50 to 300 km/h
    base_speed = 150 + 100 * math.sin(lap_progress * 2 * math.pi)
    speed_variation = 20 * math.sin(lap_progress * 8 * math.pi)  # Quick variations
    speed = max(50, base_speed + speed_variation)
    
    # Gear based on speed
    if speed < 80:
        gear = 2
        max_rpm = 7000
    elif speed < 120:
        gear = 3
        max_rpm = 7500
    elif speed < 160:
        gear = 4
        max_rpm = 8000
    elif speed < 200:
        gear = 5
        max_rpm = 8000
    elif speed < 250:
        gear = 6
        max_rpm = 7800
    else:
        gear = 7
        max_rpm = 7500
    
    # RPM varies with gear and speed
    rpm_base = (speed / 300.0) * max_rpm
    rpm_variation = 500 * math.sin(lap_progress * 16 * math.pi)  # Engine variations
    rpm = int(max(1000, min(max_rpm, rpm_base + rpm_variation)))
    
    return speed, gear, rpm

def main():
    # Get ESP8266 IP from command line or use default
    esp8266_ip = sys.argv[1] if len(sys.argv) > 1 else DEFAULT_ESP8266_IP
    
    print(f"F1 2020 UDP Telemetry Simulator")
    print(f"Target ESP8266: {esp8266_ip}:{F1_UDP_PORT}")
    print(f"Press Ctrl+C to stop")
    print()
    
    # Create UDP socket
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    
    try:
        frame_id = 0
        start_time = time.time()
        
        while True:
            current_time = time.time()
            elapsed_time = current_time - start_time
            
            # Generate realistic telemetry data
            speed, gear, rpm = simulate_driving_data(elapsed_time)
            
            # Create and send packet
            packet = create_car_telemetry_packet(speed, gear, rpm, elapsed_time, frame_id)
            
            try:
                sock.sendto(packet, (esp8266_ip, F1_UDP_PORT))
                print(f"Frame {frame_id:6d}: Speed={speed:6.1f} km/h, Gear={gear}, RPM={rpm:4d}, Size={len(packet)} bytes")
            except Exception as e:
                print(f"Send error: {e}")
            
            frame_id += 1
            
            # Send at ~20 Hz (F1 2020 typical rate)
            time.sleep(0.05)
            
    except KeyboardInterrupt:
        print("\nStopping simulator...")
    finally:
        sock.close()

if __name__ == "__main__":
    main()