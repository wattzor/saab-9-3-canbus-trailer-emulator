# Saab 9-3 Trailer Hitch CAN Bus Emulator

An ESP32-based solution that reads CAN bus signals from Saab 9-3 vehicles to control trailer lighting without triggering bulb-out warnings or requiring TECH2 programming.

![Completed Build](pic1.jpg)
*Completed ESP32 CAN bus emulator with relay board*

## The Problem This Solves

Saab vehicles cannot use traditional trailer wiring methods because:
- **Splicing into existing wiring changes circuit resistance** → triggers "bulb out" errors
- **Factory trailer modules require TECH2 programming** → expensive dealer visit required
- **Aftermarket solutions often cause dashboard warnings** → unreliable operation

This project bypasses these issues by reading lighting signals directly from the CAN bus and controlling independent relay circuits.

## What You Get

This emulator provides **standard 13-pin European trailer connector outputs**:

| Pin | Wire Color | Function | Controlled By |
|-----|------------|----------|---------------|
| 1   | Yellow     | Left Turn Signal | Relay 1 (synchronized blinking) |
| 2   | Green      | Right Turn Signal | Relay 2 (synchronized blinking) |
| 3   | Red        | Brake/Stop Lights | Relay 3 (instant response) |
| 4   | White      | Reverse/Backup | Relay 4 (instant response) |
| 5   | Brown      | Running/Tail Lights | Relay 7 (position lights) |
| 6   | Blue       | Electric Brakes | Available (Relay 8) |
| 7   | Black      | Ground | Direct connection |
| 8   | Pink       | Front Fog | Relay 5 |
| 9   | Orange     | Rear Fog | Relay 6 |
| 10  | Gray       | Left Side Light | Combined with Pin 5 |
| 11  | Black      | Right Side Light | Combined with Pin 5 |
| 12  | White/Blue | Spare | Available |
| 13  | Black      | Ground | Direct connection |


## Hardware Requirements

- ESP32 development board
- MCP2515 CAN controller module  
- 8-channel relay board (12V)
- 4.7kΩ resistor (for single-wire CAN connection)
- Breadboard jumper wires
- 13-pin trailer connector

## Single-Wire CAN Bus Connection

**Important**: Saab 9-3 uses single-wire CAN bus. The standard two-wire connection won't work.

```
MCP2515 CAN Module → Vehicle
──────────────────────────────
CAN-H → Vehicle CAN wire (OBD pin 6 or SID unit)
CAN-L → MCP2515 GND via 4.7kΩ resistor
GND   → Vehicle ground
```

**Note**: Connecting CAN-L directly to ground doesn't always work reliably. The 4.7kΩ resistor to MCP2515's GND pin is essential for proper operation with single-wire CAN systems.

## Wiring Diagram

### ESP32 to MCP2515 CAN Module
```
ESP32 Pin → MCP2515 Pin
─────────────────────────
3.3V      → VCC
GND       → GND  
GPIO 23   → MOSI
GPIO 19   → MISO
GPIO 18   → SCK
GPIO 5    → CS
```

### ESP32 to Relay Board
```
ESP32 Pin → Relay → Function
──────────────────────────────
GPIO 13   → Relay 1 → Left Turn Signal
GPIO 27   → Relay 2 → Right Turn Signal  
GPIO 26   → Relay 3 → Brake Lights
GPIO 25   → Relay 4 → Reverse Lights
GPIO 33   → Relay 5 → Front Fog Lights
GPIO 32   → Relay 6 → Rear Fog Lights
GPIO 4    → Relay 7 → Running/Tail Lights
GPIO 2    → Relay 8 → Spare/Electric Brakes
```

## Installation

1. **Hardware Assembly**:
   - Connect ESP32 to MCP2515 module
   - Connect ESP32 to 8-channel relay board
   - Connect MCP2515 to vehicle CAN bus with 4.7kΩ resistor

2. **Software Setup**:
   - Install Arduino IDE with ESP32 support
   - Install MCP2515 CAN library
   - Upload `saab_canbus_emulator.ino` to ESP32

3. **Vehicle Connection**:
   - Connect to green CAN wire in boot (left side, going to backup sensor module)
   - Alternative: OBD-II port (pins 6, 4/5, 16) or SID unit
   - Use 4.7kΩ resistor between CAN-L and MCP2515 GND

## Usage

1. Power on the ESP32
2. Connect to "ESP32_CAN" network (password: "password123")  
3. Open http://192.168.4.1 in web browser
4. Monitor real-time signals and test relays

## Web Interface Features

- **Live Signal Display**: Shows all lighting states in real-time
- **CAN Message Decoder**: Human-readable interpretation of raw data
- **Relay Test Panel**: Manual control for installation verification
- **Raw CAN Log**: Optional detailed message logging for diagnostics

## CAN Messages Monitored

| CAN ID | Description | Signals Extracted |
|--------|-------------|-------------------|
| 0x060  | Key position | Ignition on/off state |
| 0x211  | Turn signals | Left, right, hazard indicators |
| 0x300  | Light panel | Running lights, position lights |
| 0x370  | Misc signals | Reverse, front fog |
| 0x380  | Brake/fog | Brake lights, rear fog |

## Dependencies

Install in Arduino IDE:
- ESP32 board package
- MCP2515 CAN library
- Built-in WiFi and WebServer libraries

## Troubleshooting

**CAN Connection Issues**:
- Verify 4.7kΩ resistor between CAN-L and MCP2515 GND
- Check CAN-H connection to vehicle
- Ensure proper ground connection

**No CAN Messages**:
- Confirm 33.3 kbps baud rate
- Check MCP2515 crystal frequency (8MHz)
- Verify vehicle ignition is on

**Relay Problems**:
- Relays are active-low (LOW = ON)
- Check 12V power to relay board
- Verify GPIO pin connections

## Safety & Legal

⚠️ **Always follow local trailer lighting regulations**  
⚠️ **Test thoroughly before permanent installation**  
⚠️ **Disconnect battery when making CAN connections**  
⚠️ **Ensure relay current ratings match trailer load**

## License

MIT License - See LICENSE file for details

## Disclaimer

This project involves vehicle electrical modifications. Users are responsible for safe installation, local regulation compliance, and proper testing. Not responsible for vehicle damage or regulation violations.

