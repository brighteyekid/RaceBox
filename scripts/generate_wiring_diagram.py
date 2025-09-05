#!/usr/bin/env python3
"""
Generate a simple wiring diagram for the ESP32 Telemetry Dashboard
Creates a PNG image showing the connections between ESP32, OLED, and buttons.
"""

try:
    from PIL import Image, ImageDraw, ImageFont
    PIL_AVAILABLE = True
except ImportError:
    PIL_AVAILABLE = False
    print("PIL not available. Install with: pip install Pillow")

def create_wiring_diagram():
    if not PIL_AVAILABLE:
        print("Cannot create diagram without PIL/Pillow")
        return False
    
    # Image dimensions
    width, height = 800, 600
    
    # Create image
    img = Image.new('RGB', (width, height), 'white')
    draw = ImageDraw.Draw(img)
    
    # Try to use a font, fall back to default if not available
    try:
        font = ImageFont.truetype("arial.ttf", 12)
        title_font = ImageFont.truetype("arial.ttf", 16)
    except:
        font = ImageFont.load_default()
        title_font = ImageFont.load_default()
    
    # Colors
    black = (0, 0, 0)
    blue = (0, 0, 255)
    red = (255, 0, 0)
    green = (0, 128, 0)
    gray = (128, 128, 128)
    
    # Title
    draw.text((width//2 - 150, 20), "ESP32 Telemetry Dashboard Wiring", 
              fill=black, font=title_font)
    
    # ESP32 board
    esp32_x, esp32_y = 150, 150
    esp32_w, esp32_h = 200, 300
    draw.rectangle([esp32_x, esp32_y, esp32_x + esp32_w, esp32_y + esp32_h], 
                   outline=black, fill=(240, 240, 240))
    draw.text((esp32_x + 70, esp32_y + 10), "ESP32", fill=black, font=title_font)
    
    # ESP32 pins
    pins = [
        ("3.3V", esp32_x - 40, esp32_y + 50),
        ("GND", esp32_x - 40, esp32_y + 80),
        ("GPIO21", esp32_x + esp32_w + 10, esp32_y + 100),
        ("GPIO22", esp32_x + esp32_w + 10, esp32_y + 130),
        ("GPIO18", esp32_x + esp32_w + 10, esp32_y + 200),
        ("GPIO19", esp32_x + esp32_w + 10, esp32_y + 230),
    ]
    
    for pin_name, x, y in pins:
        draw.text((x, y), pin_name, fill=black, font=font)
        # Draw pin connection point
        if pin_name.startswith("GPIO"):
            draw.circle([esp32_x + esp32_w, y + 5], 3, fill=blue)
        else:
            draw.circle([esp32_x, y + 5], 3, fill=red if pin_name == "3.3V" else black)
    
    # OLED Display
    oled_x, oled_y = 500, 100
    oled_w, oled_h = 120, 80
    draw.rectangle([oled_x, oled_y, oled_x + oled_w, oled_y + oled_h], 
                   outline=black, fill=(50, 50, 50))
    draw.text((oled_x + 20, oled_y + 10), "OLED", fill=(255, 255, 255), font=font)
    draw.text((oled_x + 10, oled_y + 25), "128x32", fill=(255, 255, 255), font=font)
    
    # OLED pins
    oled_pins = [
        ("VCC", oled_x - 30, oled_y + oled_h + 10),
        ("GND", oled_x + 10, oled_y + oled_h + 10),
        ("SCL", oled_x + 50, oled_y + oled_h + 10),
        ("SDA", oled_x + 90, oled_y + oled_h + 10),
    ]
    
    for pin_name, x, y in oled_pins:
        draw.text((x, y), pin_name, fill=black, font=font)
        draw.circle([x + 15, oled_y + oled_h], 3, fill=green)
    
    # Buttons
    btn1_x, btn1_y = 500, 250
    btn2_x, btn2_y = 500, 320
    
    # Next button
    draw.circle([btn1_x, btn1_y], 20, outline=black, fill=(200, 200, 200))
    draw.text((btn1_x - 20, btn1_y + 25), "NEXT", fill=black, font=font)
    
    # Select button
    draw.circle([btn2_x, btn2_y], 20, outline=black, fill=(200, 200, 200))
    draw.text((btn2_x - 25, btn2_y + 25), "SELECT", fill=black, font=font)
    
    # Wiring connections
    connections = [
        # OLED connections
        ((esp32_x, esp32_y + 55), (oled_x - 15, oled_y + oled_h + 15), red, "3.3V"),
        ((esp32_x, esp32_y + 85), (oled_x + 25, oled_y + oled_h + 15), black, "GND"),
        ((esp32_x + esp32_w, esp32_y + 105), (oled_x + 105, oled_y + oled_h + 15), blue, "SDA"),
        ((esp32_x + esp32_w, esp32_y + 135), (oled_x + 65, oled_y + oled_h + 15), green, "SCL"),
        
        # Button connections
        ((esp32_x + esp32_w, esp32_y + 205), (btn1_x - 20, btn1_y), blue, "GPIO18"),
        ((esp32_x + esp32_w, esp32_y + 235), (btn2_x - 20, btn2_y), blue, "GPIO19"),
    ]
    
    for start, end, color, label in connections:
        draw.line([start, end], fill=color, width=2)
        # Add label at midpoint
        mid_x = (start[0] + end[0]) // 2
        mid_y = (start[1] + end[1]) // 2 - 10
        draw.text((mid_x, mid_y), label, fill=color, font=font)
    
    # Legend
    legend_x, legend_y = 50, 500
    draw.text((legend_x, legend_y), "Connections:", fill=black, font=title_font)
    draw.text((legend_x, legend_y + 25), "• Red: 3.3V Power", fill=red, font=font)
    draw.text((legend_x, legend_y + 45), "• Black: Ground", fill=black, font=font)
    draw.text((legend_x, legend_y + 65), "• Blue: GPIO Pins", fill=blue, font=font)
    draw.text((legend_x, legend_y + 85), "• Green: I2C Data", fill=green, font=font)
    
    # Notes
    notes_x = 400
    draw.text((notes_x, legend_y), "Notes:", fill=black, font=title_font)
    draw.text((notes_x, legend_y + 25), "• Use short I2C wires (<20cm)", fill=black, font=font)
    draw.text((notes_x, legend_y + 45), "• Buttons use internal pull-ups", fill=black, font=font)
    draw.text((notes_x, legend_y + 65), "• Double-check before powering", fill=black, font=font)
    
    # Save image
    try:
        img.save('docs/wiring_diagram.png')
        print("Wiring diagram saved as docs/wiring_diagram.png")
        return True
    except Exception as e:
        print(f"Error saving image: {e}")
        return False

if __name__ == "__main__":
    create_wiring_diagram()