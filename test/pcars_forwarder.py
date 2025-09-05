#!/usr/bin/env python3
"""
Project CARS 2 UDP Forwarder
Receives PCARS2 UDP telemetry and forwards simplified JSON to ESP32.

This script provides two modes:
1. UDP Mode: Listen for PCARS2 UDP broadcasts and forward parsed data
2. Simulator Mode: Generate fake PCARS2 data for testing

Usage:
    python pcars_forwarder.py [ESP8266_IP] [--simulate]

Default ESP8266 IP: 192.168.43.100
"""

import socket
import json
import time
import sys
import struct
import math
import threading

# Configuration
PCARS_UDP_PORT = 5606
ESP32_FORWARDER_PORT = 20778
DEFAULT_ESP8266_IP = "192.168.43.100"

class PCARSForwarder:
    def __init__(self, esp8266_ip, simulate_mode=False):
        self.esp8266_ip = esp8266_ip
        self.simulate_mode = simulate_mode
        self.running = False
        
        # Create UDP socket for sending to ESP8266
        self.esp8266_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        
        if not simulate_mode:
            # Create UDP socket for receiving PCARS data
            self.pcars_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            self.pcars_sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            try:
                self.pcars_sock.bind(('', PCARS_UDP_PORT))
                print(f"Listening for PCARS2 UDP on port {PCARS_UDP_PORT}")
            except Exception as e:
                print(f"Failed to bind to PCARS UDP port {PCARS_UDP_PORT}: {e}")
                print("Make sure PCARS2 is configured to send UDP data and no other app is using the port")
                sys.exit(1)
    
    def parse_pcars_udp(self, data):
        """
        Parse PCARS2 UDP data (simplified implementation)
        
        Note: PCARS2 UDP format is complex and varies by version.
        This is a basic implementation that attempts to extract key data.
        For production use, consider using established libraries or
        the JSON forwarder approach.
        """
        if len(data) < 100:
            return None
        
        try:
            # PCARS2 packets typically start with build version and packet type
            # These offsets are approximate and may vary by game version
            
            # Try to extract basic telemetry data
            # Note: These are rough estimates based on community reverse engineering
            
            # Speed (m/s) - approximate offset
            speed_ms = struct.unpack('<f', data[16:20])[0] if len(data) > 20 else 0
            speed_kmh = speed_ms * 3.6
            
            # RPM - approximate offset
            rpm = struct.unpack('<f', data[24:28])[0] if len(data) > 28 else 0
            
            # Gear - approximate offset
            gear = struct.unpack('<b', data[32:33])[0] if len(data) > 33 else 0
            
            # Fuel level (0-1) - approximate offset
            fuel_level = struct.unpack('<f', data[40:44])[0] if len(data) > 44 else 0.5
            fuel_percent = fuel_level * 100
            
            # Lap time - approximate offset
            lap_time = struct.unpack('<f', data[48:52])[0] if len(data) > 52 else 0
            
            # Validate data ranges
            if speed_kmh < 0 or speed_kmh > 400:
                speed_kmh = 0
            if rpm < 0 or rpm > 15000:
                rpm = 0
            if gear < -1 or gear > 8:
                gear = 0
            if fuel_percent < 0 or fuel_percent > 100:
                fuel_percent = 50
            
            return {
                "speed": round(speed_kmh, 1),
                "gear": int(gear),
                "rpm": int(rpm),
                "fuel": round(fuel_percent, 1),
                "lapTime": round(lap_time, 3)
            }
            
        except Exception as e:
            print(f"Error parsing PCARS data: {e}")
            return None
    
    def generate_simulated_data(self, elapsed_time):
        """Generate simulated PCARS2 telemetry data"""
        # Simulate a racing scenario
        lap_progress = (elapsed_time % 120.0) / 120.0  # 2-minute lap
        
        # Speed varies from 60 to 280 km/h
        base_speed = 170 + 80 * math.sin(lap_progress * 2 * math.pi)
        speed_variation = 15 * math.sin(lap_progress * 10 * math.pi)
        speed = max(60, base_speed + speed_variation)
        
        # Gear based on speed
        if speed < 90:
            gear = 2
            max_rpm = 6500
        elif speed < 130:
            gear = 3
            max_rpm = 7000
        elif speed < 170:
            gear = 4
            max_rpm = 7500
        elif speed < 210:
            gear = 5
            max_rpm = 7800
        else:
            gear = 6
            max_rpm = 7500
        
        # RPM calculation
        rpm_ratio = (speed / 280.0) * 0.8 + 0.2  # 20-100% of max RPM
        rpm = int(rpm_ratio * max_rpm)
        
        # Fuel decreases over time
        fuel_percent = max(5, 95 - (elapsed_time / 10.0))  # Lose ~10% per 100 seconds
        
        # Lap time (simulate completed laps)
        if lap_progress < 0.1:  # Just finished a lap
            lap_time = 118.5 + 5 * math.sin(elapsed_time / 30.0)  # Vary lap times
        else:
            lap_time = 0  # No completed lap time
        
        return {
            "speed": round(speed, 1),
            "gear": gear,
            "rpm": rpm,
            "fuel": round(fuel_percent, 1),
            "lapTime": round(lap_time, 3) if lap_time > 0 else 0
        }
    
    def send_to_esp8266(self, telemetry_data):
        """Send JSON telemetry data to ESP8266"""
        try:
            json_data = json.dumps(telemetry_data)
            self.esp8266_sock.sendto(json_data.encode('utf-8'), (self.esp8266_ip, ESP32_FORWARDER_PORT))
            return True
        except Exception as e:
            print(f"Error sending to ESP8266: {e}")
            return False
    
    def run_udp_mode(self):
        """Run in UDP listening mode"""
        print("Waiting for PCARS2 UDP data...")
        print("Make sure PCARS2 is running and UDP output is enabled")
        print()
        
        packet_count = 0
        last_data_time = time.time()
        
        while self.running:
            try:
                # Set timeout to check for stop condition
                self.pcars_sock.settimeout(1.0)
                data, addr = self.pcars_sock.recvfrom(2048)
                
                packet_count += 1
                current_time = time.time()
                
                # Parse PCARS data
                telemetry = self.parse_pcars_udp(data)
                
                if telemetry:
                    # Send to ESP8266
                    if self.send_to_esp8266(telemetry):
                        print(f"Packet {packet_count:6d}: Speed={telemetry['speed']:6.1f} km/h, "
                              f"Gear={telemetry['gear']}, RPM={telemetry['rpm']:4d}, "
                              f"Fuel={telemetry['fuel']:5.1f}%")
                    
                    last_data_time = current_time
                else:
                    print(f"Packet {packet_count:6d}: Failed to parse ({len(data)} bytes from {addr})")
                
                # Check for data timeout
                if current_time - last_data_time > 5.0:
                    print("No valid PCARS data received for 5 seconds...")
                    last_data_time = current_time
                    
            except socket.timeout:
                continue
            except Exception as e:
                print(f"UDP receive error: {e}")
                time.sleep(1)
    
    def run_simulator_mode(self):
        """Run in simulation mode"""
        print("Running in simulation mode - generating fake PCARS2 data")
        print()
        
        start_time = time.time()
        frame_count = 0
        
        while self.running:
            current_time = time.time()
            elapsed_time = current_time - start_time
            
            # Generate simulated data
            telemetry = self.generate_simulated_data(elapsed_time)
            
            # Send to ESP8266
            if self.send_to_esp8266(telemetry):
                print(f"Frame {frame_count:6d}: Speed={telemetry['speed']:6.1f} km/h, "
                      f"Gear={telemetry['gear']}, RPM={telemetry['rpm']:4d}, "
                      f"Fuel={telemetry['fuel']:5.1f}%")
            
            frame_count += 1
            time.sleep(0.1)  # 10 Hz update rate
    
    def start(self):
        """Start the forwarder"""
        self.running = True
        
        if self.simulate_mode:
            self.run_simulator_mode()
        else:
            self.run_udp_mode()
    
    def stop(self):
        """Stop the forwarder"""
        self.running = False
        if hasattr(self, 'pcars_sock'):
            self.pcars_sock.close()
        self.esp8266_sock.close()

def main():
    # Parse command line arguments
    esp8266_ip = DEFAULT_ESP8266_IP
    simulate_mode = False
    
    for arg in sys.argv[1:]:
        if arg == "--simulate":
            simulate_mode = True
        elif not arg.startswith("--"):
            esp8266_ip = arg
    
    print("Project CARS 2 UDP Forwarder")
    print(f"Target ESP8266: {esp8266_ip}:{ESP32_FORWARDER_PORT}")
    print(f"Mode: {'Simulation' if simulate_mode else 'UDP Listening'}")
    print("Press Ctrl+C to stop")
    print()
    
    # Create and start forwarder
    forwarder = PCARSForwarder(esp8266_ip, simulate_mode)
    
    try:
        forwarder.start()
    except KeyboardInterrupt:
        print("\nStopping forwarder...")
    finally:
        forwarder.stop()

if __name__ == "__main__":
    main()