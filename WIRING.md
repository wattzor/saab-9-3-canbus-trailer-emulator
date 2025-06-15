# Wiring Diagram

## ESP32 to MCP2515 CAN Controller

```
ESP32 Pin    │ MCP2515 Pin │ Description
─────────────┼─────────────┼──────────────────
3.3V         │ VCC         │ Power supply
GND          │ GND         │ Ground
GPIO 23      │ MOSI        │ SPI Master Out Slave In
GPIO 19      │ MISO        │ SPI Master In Slave Out
GPIO 18      │ SCK         │ SPI Clock
GPIO 5       │ CS          │ SPI Chip Select
```

## ESP32 to 8-Channel Relay Board

```
ESP32 Pin    │ Relay Board │ Function
─────────────┼─────────────┼─────────────────────
3.3V         │ VCC         │ Power supply (logic)
GND          │ GND         │ Ground
GPIO 13      │ IN1         │ Relay 1 - Left Turn Signal
GPIO 27      │ IN2         │ Relay 2 - Right Turn Signal
GPIO 26      │ IN3         │ Relay 3 - Brake Lights
GPIO 25      │ IN4         │ Relay 4 - Reverse Lights
GPIO 33      │ IN5         │ Relay 5 - Front Fog Lights
GPIO 32      │ IN6         │ Relay 6 - Rear Fog Lights
GPIO 4       │ IN7         │ Relay 7 - Running Lights
GPIO 2       │ IN8         │ Relay 8 - Unused/Spare
```

## MCP2515 to Vehicle CAN Bus (IMPORTANT: Single-Wire Connection)

**Critical**: Saab 9-3 uses single-wire CAN bus. Standard two-wire connection will NOT work.

```
MCP2515 Pin  │ Vehicle Connection │ Notes
─────────────┼────────────────────┼─────────────────────────────────
CAN-H        │ Vehicle CAN wire   │ OBD pin 6 or SID unit CAN line
CAN-L        │ MCP2515 GND        │ Via 4.7kΩ resistor (ESSENTIAL!)
GND          │ Vehicle Ground     │ OBD pin 4/5 or chassis ground
```

**Resistor Connection Detail**:
```
MCP2515 CAN-L ──── 4.7kΩ Resistor ──── MCP2515 GND
```

**Why the resistor is needed**: Direct connection of CAN-L to ground doesn't work reliably with Saab's single-wire CAN system. The 4.7kΩ resistor provides proper termination.

## Relay Board to 13-Pin EU Trailer Connector

Connect relay outputs (COM terminals) to standard 13-pin European trailer connector:

```
Relay        │ Trailer Pin │ Wire Color │ Function
─────────────┼─────────────┼────────────┼─────────────────────
Relay 1 COM  │ Pin 1       │ Yellow     │ Left Turn Signal
Relay 2 COM  │ Pin 2       │ Green      │ Right Turn Signal
Relay 3 COM  │ Pin 3       │ Red        │ Brake/Stop Lights
Relay 4 COM  │ Pin 4       │ White      │ Reverse/Backup Lights
Relay 7 COM  │ Pin 5       │ Brown      │ Running/Tail Lights
Relay 8 COM  │ Pin 6       │ Blue       │ Electric Brakes (optional)
Direct       │ Pin 7       │ Black      │ Ground/Common
Relay 5 COM  │ Pin 8       │ Pink       │ Front Fog Lights
Relay 6 COM  │ Pin 9       │ Orange     │ Rear Fog Lights
Combined     │ Pin 10      │ Gray       │ Left Side Light
Combined     │ Pin 11      │ Black      │ Right Side Light
Spare        │ Pin 12      │ White/Blue │ Spare/Auxiliary
Direct       │ Pin 13      │ Black      │ Ground/Common
```

## Power Supply Requirements

- **ESP32**: 3.3V (can be powered via USB or external 5V with onboard regulator)
- **MCP2515**: 3.3V or 5V (check your module specifications)
- **Relay Board**: 
  - Logic: 3.3V (from ESP32)
  - Coil Power: 5V or 12V (depending on relay board type)
- **Trailer Lights**: 12V from vehicle battery (through relay contacts)

## Vehicle Connection Points

### Option 1: OBD-II Port (Recommended for testing)
```
OBD-II Pin   │ Function    │ Connection
─────────────┼─────────────┼──────────────────
Pin 4        │ Chassis GND │ MCP2515 GND
Pin 5        │ Signal GND  │ Alternative ground
Pin 6        │ CAN-H       │ MCP2515 CAN-H
Pin 14       │ CAN-L       │ Not used (single-wire)
Pin 16       │ +12V        │ Power (if needed)
```

### Option 2: SID Unit (Permanent installation)
- **Location**: Behind dashboard, near steering column
- **Advantage**: Dedicated connection, no OBD interference
- **Disadvantage**: Requires vehicle disassembly

## CAN Bus Specifications
- **Protocol**: CAN 2.0A (11-bit identifiers)
- **Baud Rate**: 33.3 kbps (Saab I-Bus)
- **Topology**: Single-wire with ground reference
- **Termination**: 4.7kΩ resistor between CAN-L and ground

## Safety Notes

⚠️ **Critical Safety Information**:
- Always disconnect vehicle battery before making CAN connections
- Test all connections with multimeter before applying power
- Ensure relay current ratings exceed trailer load requirements
- Use proper automotive-grade connectors for permanent installation
- Follow local trailer lighting regulations and standards

## Troubleshooting Wiring Issues

**No CAN Communication**:
1. Verify 4.7kΩ resistor between CAN-L and MCP2515 GND
2. Check CAN-H connection to vehicle CAN wire
3. Confirm ground connection quality
4. Test with multimeter: CAN line should show ~2.5V DC

**Relay Problems**:
1. Relays are active-LOW (ESP32 LOW = relay ON)
2. Check 12V supply to relay board VCC
3. Verify ESP32 GPIO voltage levels (3.3V)
4. Test individual relays with manual toggle function

**Power Issues**:
1. ESP32 should show 3.3V on VCC pin
2. Relay board logic should receive 3.3V or 5V as specified
3. Relay coils need adequate current supply
4. Check for voltage drops in long wire runs