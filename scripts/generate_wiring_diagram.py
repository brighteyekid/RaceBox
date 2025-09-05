#!/usr/bin/env python3
"""
Generate a detailed wiring diagram for the ESP8266/ESP32 Telemetry Dashboard
Creates a PNG image showing the connections between microcontroller, OLED, and buttons
with pull-up resistors and clear pin labeling.
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
    
    # Image dimensions - increased for more detail
    width, height = 1000, 800
    
    # Create image with white background
    img = Image.new('RGB', (width, height), 'white')
    draw = ImageDraw.Draw(img)
    
    # Try to use a font, fall back to default if not available
    try:
        font = ImageFont.truetype("DejaVuSans.ttf", 12)
        title_font = ImageFont.truetype("DejaVuSans.ttf", 20)
        pin_font = ImageFont.truetype("DejaVuSans.ttf", 10)
    except:
        font = ImageFont.load_default()
        title_font = font
        pin_font = font
    
    # Colors
    black = (0, 0, 0)
    blue = (0, 0, 255)
    red = (255, 0, 0)
    green = (0, 128, 0)
    gray = (128, 128, 128)
    yellow = (255, 200, 0)
    
    # Title and subtitle
    draw.text((width//2 - 200, 20), "ESP8266 Telemetry Dashboard Wiring", 
              fill=black, font=title_font)
    draw.text((width//2 - 150, 50), "F1 2020 & Project CARS 2 Display", 
              fill=gray, font=font)
    
    # ESP8266 board
    esp_x, esp_y = 150, 150
    esp_w, esp_h = 250, 350
    draw.rectangle([esp_x, esp_y, esp_x + esp_w, esp_y + esp_h], 
                   outline=black, fill=(240, 240, 240))
    draw.text((esp_x + 70, esp_y + 10), "ESP8266 NodeMCU", fill=black, font=title_font)
    
    # ESP8266 pin labels - more detailed
    pins = [
        ("3.3V", esp_x - 40, esp_y + 50, red),
        ("GND", esp_x - 40, esp_y + 80, black),
        ("D1/GPIO5 (SCL)", esp_x + esp_w + 10, esp_y + 100, green),
        ("D2/GPIO4 (SDA)", esp_x + esp_w + 10, esp_y + 130, blue),
        ("D5/GPIO14", esp_x + esp_w + 10, esp_y + 200, yellow),
        ("D6/GPIO12", esp_x + esp_w + 10, esp_y + 230, yellow),
    ]
    
    # Draw pin labels with connection points
    for pin_name, x, y, color in pins:
        draw.text((x, y), pin_name, fill=color, font=pin_font)
        if "GPIO" in pin_name:
            draw.circle([esp_x + esp_w, y + 5], 3, fill=color)
        else:
            draw.circle([esp_x, y + 5], 3, fill=color)
    
    # OLED Display - more detailed
    oled_x, oled_y = 600, 100
    oled_w, oled_h = 160, 100
    draw.rectangle([oled_x, oled_y, oled_x + oled_w, oled_y + oled_h], 
                   outline=black, fill=(50, 50, 50))
    draw.text((oled_x + 30, oled_y + 10), "OLED SSD1306", fill=(255, 255, 255), font=font)
    draw.text((oled_x + 45, oled_y + 30), "128x32 I²C", fill=(255, 255, 255), font=font)
    
    # OLED screen visualization
    screen_margin = 20
    draw.rectangle([oled_x + screen_margin, oled_y + screen_margin, 
                   oled_x + oled_w - screen_margin, oled_y + screen_margin + 25], 
                   outline=(100, 100, 255), fill=(0, 0, 40))
    
    # OLED pins with clear labels
    oled_pins = [
        ("VCC", oled_x - 30, oled_y + oled_h + 10, red),
        ("GND", oled_x + 10, oled_y + oled_h + 10, black),
        ("SCL", oled_x + 50, oled_y + oled_h + 10, green),
        ("SDA", oled_x + 90, oled_y + oled_h + 10, blue),
    ]
    
    for pin_name, x, y, color in oled_pins:
        draw.text((x, y), pin_name, fill=color, font=pin_font)
        draw.circle([x + 15, oled_y + oled_h], 3, fill=color)
    
    # Buttons with pull-up resistors
    def draw_button(x, y, label):
        # Button circle
        draw.circle([x, y], 15, outline=black, fill=(200, 200, 200))
        # Button label
        draw.text((x - 25, y + 20), label, fill=black, font=font)
        # Pull-up resistor
        res_x = x - 40
        draw.line([(res_x, y - 15), (res_x, y + 15)], fill=gray, width=2)
        draw.text((res_x - 20, y - 10), "10kΩ", fill=gray, font=pin_font)
    
    # Draw buttons
    btn1_x, btn1_y = 600, 300
    btn2_x, btn2_y = 600, 400
    draw_button(btn1_x, btn1_y, "NEXT")
    draw_button(btn2_x, btn2_y, "SELECT")
    
    # Wiring connections with better routing
    connections = [
        # OLED connections
        ((esp_x, esp_y + 55), (oled_x - 15, oled_y + oled_h + 15), red, "3.3V"),
        ((esp_x, esp_y + 85), (oled_x + 25, oled_y + oled_h + 15), black, "GND"),
        ((esp_x + esp_w, esp_y + 105), (oled_x + 65, oled_y + oled_h + 15), green, "SCL"),
        ((esp_x + esp_w, esp_y + 135), (oled_x + 105, oled_y + oled_h + 15), blue, "SDA"),
        
        # Button connections with routing points
        ((esp_x + esp_w, esp_y + 205), (btn1_x - 60, btn1_y), yellow, "GPIO14"),
        ((esp_x + esp_w, esp_y + 235), (btn2_x - 60, btn2_y), yellow, "GPIO12"),
    ]
    
    # Draw connections with improved routing
    for start, end, color, label in connections:
        # Create routing points for cleaner lines
        mid_x = (start[0] + end[0]) // 2
        
        # Draw segmented lines for cleaner appearance
        draw.line([start, (mid_x, start[1])], fill=color, width=2)
        draw.line([(mid_x, start[1]), (mid_x, end[1])], fill=color, width=2)
        draw.line([(mid_x, end[1]), end], fill=color, width=2)
        
        # Add label at midpoint
        label_x = mid_x + 5
        label_y = (start[1] + end[1]) // 2 - 10
        draw.text((label_x, label_y), label, fill=color, font=pin_font)
    
    # Enhanced legend
    legend_x, legend_y = 50, height - 200
    draw.text((legend_x, legend_y), "Connection Guide:", fill=black, font=title_font)
    
    legend_items = [
        ("Power (3.3V)", red),
        ("Ground (GND)", black),
        ("I²C Clock (SCL)", green),
        ("I²C Data (SDA)", blue),
        ("Button GPIO", yellow),
    ]
    
    for i, (text, color) in enumerate(legend_items):
        y_offset = legend_y + 30 + (i * 20)
        draw.line([(legend_x, y_offset), (legend_x + 30, y_offset)], fill=color, width=3)
        draw.text((legend_x + 40, y_offset - 7), text, fill=color, font=font)
    
    # Important notes
    notes_x = width - 300
    notes_y = legend_y
    draw.text((notes_x, notes_y), "Important Notes:", fill=black, font=title_font)
    notes = [
        "• Keep I²C wires short (<20cm)",
        "• Verify 3.3V power (not 5V)",
        "• Buttons use internal pull-ups",
        "• Double-check connections",
        "• Use NodeMCU v2 or Wemos D1"
    ]
    
    for i, note in enumerate(notes):
        draw.text((notes_x, notes_y + 30 + (i * 20)), note, fill=black, font=font)
    
    # Save image with high quality
    try:
        img.save('docs/wiring_diagram.png', quality=95, optimize=True)
        print("Enhanced wiring diagram saved as docs/wiring_diagram.png")
        
        # Also save the pin mapping as text
        with open('docs/wiring_diagram.txt', 'w') as f:
            f.write("ESP8266 Telemetry Dashboard - Pin Mapping\n")
            f.write("=======================================\n\n")
            f.write("OLED Display (SSD1306 128x32):\n")
            f.write("- VCC  → ESP8266 3.3V\n")
            f.write("- GND  → ESP8266 GND\n")
            f.write("- SCL  → ESP8266 D1 (GPIO5)\n")
            f.write("- SDA  → ESP8266 D2 (GPIO4)\n\n")
            f.write("Buttons:\n")
            f.write("- NEXT    → ESP8266 D5 (GPIO14) → GND\n")
            f.write("- SELECT  → ESP8266 D6 (GPIO12) → GND\n")
            
        return True
    except Exception as e:
        print(f"Error saving diagram: {e}")
        return False

if __name__ == "__main__":
    create_wiring_diagram()