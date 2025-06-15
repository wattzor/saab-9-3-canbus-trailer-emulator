# Installation Guide

## Pre-Installation Checklist

Before starting, ensure you have:
- [ ] ESP32 development board
- [ ] MCP2515 CAN controller module (8MHz crystal)
- [ ] 8-channel relay board (12V compatible)
- [ ] 4.7kΩ resistor (critical for single-wire CAN)
- [ ] Breadboard jumper wires
- [ ] Multimeter for testing
- [ ] 13-pin trailer connector
- [ ] Heat shrink tubing and electrical tape

## Step 1: Software Setup

### Install Arduino IDE
1. Download Arduino IDE from arduino.cc
2. Install ESP32 board support:
   - File → Preferences → Additional Board Manager URLs
   - Add: `https://dl.espressif.com/dl/package_esp32_index.json`
   - Tools → Board → Boards Manager → Search "ESP32" → Install

### Install Libraries
1. Tools → Manage Libraries
2. Search and install: "MCP2515" by coryjfowler
3. ESP32 WiFi and WebServer libraries are included

### Upload Code
1. Open `saab_canbus_emulator.ino`
2. Select Board: "ESP32 Dev Module"
3. Select correct COM port
4. Upload code to ESP32

## Step 2: Hardware Assembly

### ESP32 to MCP2515 Connections
```
ESP32    → MCP2515
─────────────────────
3.3V     → VCC
GND      → GND
GPIO 23  → MOSI
GPIO 19  → MISO  
GPIO 18  → SCK
GPIO 5   → CS
```

### ESP32 to Relay Board
```
ESP32    → Relay Board
────────────────────────
3.3V     → VCC (logic)
GND      → GND
GPIO 13  → IN1 (Left Turn)
GPIO 27  → IN2 (Right Turn)
GPIO 26  → IN3 (Brake)
GPIO 25  → IN4 (Reverse)
GPIO 33  → IN5 (Front Fog)
GPIO 32  → IN6 (Rear Fog)
GPIO 4   → IN7 (Running Lights)
GPIO 2   → IN8 (Spare)
```

### Critical: CAN Bus Connection
⚠️ **IMPORTANT**: Saab uses single-wire CAN. Standard two-wire won't work!

```
MCP2515 CAN-H → Vehicle CAN wire (OBD pin 6)
MCP2515 CAN-L → 4.7kΩ resistor → MCP2515 GND
MCP2515 GND   → Vehicle ground (OBD pin 4/5)
```

## Step 3: Initial Testing

### 1. Power-On Test
1. Connect ESP32 via USB
2. Open Serial Monitor (9600 baud)
3. Look for startup messages:
   ```
   ======== SAAB 9-3 CAN WiFi Debugger ==========
   WiFi AP started: ESP32_CAN
   Web server started
   MCP2515 ready in listen-only mode
   ```

### 2. WiFi Connection Test
1. Connect device to "ESP32_CAN" network
2. Password: "password123"  
3. Open browser to `http://192.168.4.1`
4. Should see dashboard interface

### 3. Relay Test
1. In web interface, click "Toggle Relay" buttons
2. Listen for relay clicking sounds
3. Measure relay output with multimeter
4. All relays should respond to web commands

## Step 4: Vehicle Connection

### Safety First
⚠️ **Disconnect vehicle battery before CAN connections**

### OBD-II Port Connection (Testing)
```
OBD Pin → Connection
──────────────────────
Pin 4   → MCP2515 GND
Pin 6   → MCP2515 CAN-H  
Pin 16  → +12V (if needed)
```

### Verification
1. Reconnect battery
2. Turn ignition to ON position
3. Check web interface for CAN messages
4. Look for "Raw CAN" data in log

## Step 5: CAN Signal Verification

### Expected Signals
Test each function and verify in web interface:

| Action | Expected Web Display |
|--------|---------------------|
| Ignition ON | "Ignition: ON" |
| Press brake | "Brake: ON" |
| Select reverse | "Reverse: ON" |
| Left turn signal | "Left: ON" |
| Right turn signal | "Right: ON" |
| Hazard lights | "Hazard: ON" (red text) |
| Position lights | "Side Lights: ON" |

### Troubleshooting No CAN Data
1. Check 4.7kΩ resistor connection
2. Verify CAN-H to OBD pin 6
3. Confirm ground connection
4. Try different ignition positions
5. Check MCP2515 crystal frequency (8MHz required)

## Step 6: Trailer Connector Wiring

### Standard 13-Pin EU Connector
```
Pin → Relay → Wire Color → Function
────────────────────────────────────
1   → 1     → Yellow     → Left Turn
2   → 2     → Green      → Right Turn  
3   → 3     → Red        → Brake/Stop
4   → 4     → White      → Reverse
5   → 7     → Brown      → Running Lights
6   → 8     → Blue       → Electric Brakes
7   → -     → Black      → Ground
8   → 5     → Pink       → Front Fog
9   → 6     → Orange     → Rear Fog
10  → 7     → Gray       → Left Side
11  → 7     → Black      → Right Side  
12  → -     → White/Blue → Spare
13  → -     → Black      → Ground
```

### Power Distribution
- Relay common (COM) terminals → +12V from battery
- Relay NO (Normally Open) → Trailer connector pins
- Grounds (pins 7, 13) → Vehicle chassis ground

## Step 7: Final Testing

### System Integration Test
1. Connect trailer or load resistors
2. Test all lighting functions:
   - Turn signals (left/right)
   - Hazard lights
   - Brake lights  
   - Reverse lights
   - Running lights
   - Fog lights

### Timing Verification
- Turn signals should blink in sync with vehicle
- Hazards should blink both sides simultaneously  
- Brake/reverse should be instant response

### Load Testing  
- Verify relay contacts handle trailer current
- Check for voltage drops under load
- Monitor for overheating

## Step 8: Permanent Installation

### Mounting
- Choose protected location in vehicle
- Secure ESP32 and relay board
- Protect CAN connections from moisture
- Use automotive-grade connectors

### Documentation
- Take photos of all connections
- Record any customizations made
- Keep wiring diagram accessible

## Troubleshooting Common Issues

### Problem: No WiFi Connection
- Check ESP32 power supply
- Verify "ESP32_CAN" network appears
- Try different device/browser

### Problem: Relays Don't Activate  
- Check 12V supply to relay board
- Verify ESP32 GPIO output voltage
- Test with multimeter on relay coils

### Problem: Erratic CAN Messages
- Check 4.7kΩ resistor value and connection
- Verify CAN-H wire integrity
- Ensure good ground connection

### Problem: Wrong Blink Timing
- Confirm vehicle turn signal timing
- Check for multiple CAN message sources
- Verify software blink interval (500ms)

## Maintenance

### Regular Checks
- Verify all relay outputs periodically
- Check CAN connection integrity
- Monitor for firmware updates
- Test with trailer before trips

### Updates
- Software updates via Arduino IDE
- Hardware modifications may require rewiring
- Keep backup of working configuration