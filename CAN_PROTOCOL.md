# Saab 9-3 CAN Protocol Documentation

This document details the specific CAN bus messages that the emulator reads from the Saab 9-3.

## CAN Bus Specifications

- **Protocol**: CAN 2.0A (11-bit identifiers)
- **Baud Rate**: 33.3 kbps  
- **Bus Type**: Single-wire with ground reference
- **Connection**: 4.7kΩ resistor between CAN-L and ground

## Monitored CAN Messages

### 0x060 - Key Position / Ignition Status

**What we read**: Current ignition state

**Format**:
```
ID: 0x060
Byte 1: Key position value
```

**Values we use**:
| Value | Meaning |
|-------|---------|
| 0xA0  | Ignition ON |
| 0xE0  | START position |
| Other | Ignition OFF |

### 0x211 - Turn Signals and Hazard Lights

**What we read**: Turn signal and hazard states

**Format**:
```
ID: 0x211  
Byte 2: Signal type
Byte 3: Must be 0x43 when active
```

**Values we use** (when Byte 3 = 0x43):
| Byte 2 | Meaning |
|--------|---------|
| 0x54   | Right Turn Signal |
| 0xA8   | Left Turn Signal |
| 0xFC   | Hazard Lights |

### 0x300 - Light Panel Position

**What we read**: Light switch position for running lights

**Format**:
```
ID: 0x300
Byte 1: Light mode
```

**Values we use**:
| Value | Meaning |
|-------|---------|
| 0x20  | Position lights ON |
| 0x18  | Low beam lights ON |
| 0x10  | Auto mode (needs ignition) |
| 0x28  | DRL/Auto active (needs ignition) |

### 0x370 - Reverse and Front Fog

**What we read**: Reverse gear and front fog status

**Format**:
```
ID: 0x370
Byte 0: Status bits
```

**Bits we use**:
| Bit | Meaning |
|-----|---------|
| 0   | Reverse gear (1 = engaged) |
| 6   | Front fog lights (1 = on) |

### 0x380 - Brake and Rear Fog

**What we read**: Brake pedal and rear fog status

**Format**:
```
ID: 0x380
Byte 0: Brake status  
Byte 1: Rear fog status
```

**Values we use**:
| Byte | Value | Meaning |
|------|-------|---------|
| 0    | 0x20  | Brake pedal pressed |
| 1    | 0x20  | Rear fog lights on |

## Implementation Notes

- **Blink timing**: We generate our own 500ms blink cycles in software
- **Turn signal priority**: Hazards override individual turn signals  
- **Running lights**: Combine position lights + auto mode (when ignition on)
- **All other timing and logic**: Implemented in ESP32 code, not read from CAN

## Model Compatibility

Tested on Saab 9-3 (2003-2012). Other Saab models may use different message formats.