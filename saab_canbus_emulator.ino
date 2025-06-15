#include <SPI.h>
#include <WiFi.h>
#include <WebServer.h>
#include <mcp2515.h>
#include <can.h>

// SPI and CAN
spi_device_handle_t spi;
MCP2515 mcp2515(&spi);
struct can_frame frame;

// Web server
WebServer server(80);

// State variables
bool brake = false;
bool reverse = false;
bool left = false;
bool right = false;
bool frontFog = false;
bool rearFog = false;
bool runningLights = false;
bool hazard = false;
bool ignitionOn = false;
uint8_t hazardRawByte3 = 0;
uint8_t hazardRawByte4 = 0;
uint8_t lightPanelRawByte = 0;
uint8_t keyPositionRaw = 0;
String lastMessages = "";
bool showCanLog = false;
// Relay control (Relay 6 = runningLights, Relay 7 unused)
const int relayPins[8] = {13, 27, 26, 25, 33, 32, 4, 2};

// Blinker sync logic
bool prevHazard = false;
bool prevLeft = false;
bool prevRight = false;

// Hazard blink timer
unsigned long lastHazardBlinkTime = 0;
bool hazardBlinkState = false;

// Turn signal blink timers
unsigned long lastLeftBlinkTime = 0;
unsigned long lastRightBlinkTime = 0;
bool leftBlinkState = false;
bool rightBlinkState = false;

void handleRoot() {
  // Decode light panel mode for display
  String lightModeStr;
  switch (lightPanelRawByte) {
    case 0x00: lightModeStr = "OFF"; break;
    case 0x08: lightModeStr = "PARK light (handbrake indicator)"; break;
    case 0x10: lightModeStr = "Auto mode"; break;
    case 0x18: lightModeStr = "Low beam lights"; break;
    case 0x20: lightModeStr = "Position lights"; break;
    case 0x28: lightModeStr = "DRL/Auto active"; break;
    default: lightModeStr = "Unknown"; break;
  }
  // Decode key position
  String keyPositionStr;
  switch (keyPositionRaw) {
    case 0x00: keyPositionStr = "No key"; break;
    case 0x20: keyPositionStr = "LOCK"; break;
    case 0x60: keyPositionStr = "OFF"; break;
    case 0xA0: keyPositionStr = "ON"; break;
    case 0xE0: keyPositionStr = "START"; break;
    default: keyPositionStr = "Unknown"; break;
  }

  String page = "<html><head><meta charset='UTF-8'><meta http-equiv='refresh' content='2'>";
  page += "<style>body{font-family:sans-serif;}pre{background:#f4f4f4;padding:10px;border:1px solid #ccc;max-height:300px;overflow-y:auto;}button{padding:10px;margin:5px;font-size:16px;} .hazard{color:red;font-weight:bold;}</style>";
  page += "<script>window.onload=function(){var pre=document.getElementById('canlog');if(pre)pre.scrollTop=pre.scrollHeight;}</script>";
  page += "</head><body>";
  page += "<h1>Saab 9-3 CAN Dashboard</h1>";

  // CAN Log toggle checkbox
  page += "<form action='/toggleCanLog' method='POST'>";
  page += "<label><input type='checkbox' name='canlog' onchange='this.form.submit()'";
  if (showCanLog) page += " checked";
  page += "> Show Raw CAN Log</label></form>";

  page += "<pre><b>Signals:</b>\n";
  page += ignitionOn    ? "Ignition: ON\n"      : "Ignition: OFF\n";
  page += brake         ? "Brake: ON\n"         : "Brake: OFF\n";
  page += reverse       ? "Reverse: ON\n"       : "Reverse: OFF\n";
  page += left          ? "Left: ON\n"          : "Left: OFF\n";
  page += right         ? "Right: ON\n"         : "Right: OFF\n";
  page += frontFog      ? "Front Fog: ON\n"     : "Front Fog: OFF\n";
  page += rearFog       ? "Rear Fog: ON\n"      : "Rear Fog: OFF\n";
  page += runningLights ? "Side Lights: ON\n"   : "Side Lights: OFF\n";
  page += hazard        ? "<span class='hazard'>Hazard: ON</span>\n" : "Hazard: OFF\n";
  page += "\n<b>Raw Registers:</b>\n";
  page += "0x060 b1 (Key): 0x" + String(keyPositionRaw, HEX) + " → " + keyPositionStr + "\n";
  page += "0x211 (Indicators): " + String(hazardRawByte3, HEX) + " " + String(hazardRawByte4, HEX) + "\n";
  page += "0x300 b1 (Lights): 0x" + String(lightPanelRawByte, HEX) + " → " + lightModeStr + "\n";
  page += "</pre><hr>";

  page += "<h2>Relay Test Panel (CAN controlled)</h2>";
  for (int i = 0; i < 8; i++) {
    page += "<form action='/toggleRelay" + String(i) + "' method='POST'><button>Toggle Relay " + String(i+1) + "</button></form>";
  }

  if (showCanLog) {
    page += "<hr><pre id='canlog'><b>Raw CAN:</b>\n" + lastMessages + "</pre>";
  }

  page += "</body></html>";
  server.send(200, "text/html", page);
}

void toggleRelay(int relay) {
  digitalWrite(relayPins[relay], LOW);
  delay(300);
  digitalWrite(relayPins[relay], HIGH);
}

void setup() {
  Serial.begin(9600);
  delay(1000);
  Serial.println("\n======== SAAB 9-3 CAN WiFi Debugger ==========");

  WiFi.softAP("ESP32_CAN", "password123");
  Serial.println("WiFi AP started: ESP32_CAN");
  Serial.println("Connect and open http://192.168.4.1");

  for (int i = 0; i < 8; i++) {
    pinMode(relayPins[i], OUTPUT);
    digitalWrite(relayPins[i], HIGH);
  }

  server.on("/", handleRoot);
  server.on("/toggleCanLog", HTTP_POST, []() {
    showCanLog = !showCanLog;
    if (!showCanLog) lastMessages = "";
    server.sendHeader("Location", "/");
    server.send(303);
  });

  for (int i = 0; i < 8; i++) {
    server.on( ("/toggleRelay" + String(i)).c_str(), HTTP_POST, [i]() {
      toggleRelay(i);
      server.sendHeader("Location", "/");
      server.send(303);
    });
  }

  server.begin();
  Serial.println("Web server started");

  spi_bus_config_t buscfg = {.mosi_io_num = 23, .miso_io_num = 19, .sclk_io_num = 18, .quadwp_io_num = -1, .quadhd_io_num = -1, .max_transfer_sz = 32};
  esp_err_t ret = spi_bus_initialize(HSPI_HOST, &buscfg, 1);
  if (ret != ESP_OK) Serial.println("SPI bus initialization failed");
  else Serial.println("SPI bus initialized");

  spi_device_interface_config_t devcfg = {.mode = 0, .clock_speed_hz = 10000000, .spics_io_num = 5, .queue_size = 1};
  ret = spi_bus_add_device(HSPI_HOST, &devcfg, &spi);
  if (ret != ESP_OK) Serial.println("SPI device add failed");
  else Serial.println("MCP2515 SPI device added");

  mcp2515.reset();
  delay(200);
  mcp2515.setBitrate(CAN_33KBPS, MCP_8MHZ);
  mcp2515.setListenOnlyMode();
  Serial.println("MCP2515 ready in listen-only mode");
}

void loop() {
  server.handleClient();

  unsigned long currentTime = millis();

  // Handle hazard blinking timer
  if (hazard) {
    if (currentTime - lastHazardBlinkTime >= 500) { // 500ms blink interval
      hazardBlinkState = !hazardBlinkState;
      lastHazardBlinkTime = currentTime;
      
      // Control hazard relays with blink state
      digitalWrite(relayPins[0], hazardBlinkState ? LOW : HIGH); // Left
      digitalWrite(relayPins[1], hazardBlinkState ? LOW : HIGH); // Right
    }
  } else {
    // Handle individual turn signal blinking when hazards are OFF
    if (left) {
      if (currentTime - lastLeftBlinkTime >= 500) { // 500ms blink interval
        leftBlinkState = !leftBlinkState;
        lastLeftBlinkTime = currentTime;
        digitalWrite(relayPins[0], leftBlinkState ? LOW : HIGH);
      }
    }
    
    if (right) {
      if (currentTime - lastRightBlinkTime >= 500) { // 500ms blink interval
        rightBlinkState = !rightBlinkState;
        lastRightBlinkTime = currentTime;
        digitalWrite(relayPins[1], rightBlinkState ? LOW : HIGH);
      }
    }
  }

  if (mcp2515.readMessage(&frame) == MCP2515::ERROR_OK) {
    char buffer[64];
    snprintf(buffer, sizeof(buffer), "%lu ms 0x%03X [", millis(), frame.can_id);
    String msg = buffer;
    for (int i = 0; i < frame.can_dlc; i++) {
      snprintf(buffer, sizeof(buffer), " %02X", frame.data[i]);
      msg += buffer;
    }
    msg += " ]\n";
    lastMessages += msg;
    if (lastMessages.length() > 3000) lastMessages = lastMessages.substring(lastMessages.length() - 2000);

    if (frame.can_id == 0x290 && frame.can_dlc >= 5) {
      // legacy backup - kept for compatibility
    }
    
    if (frame.can_id == 0x370) {
      reverse = (frame.data[0] & 0x01);
      frontFog = (frame.data[0] & 0x40);
    }
    
    if (frame.can_id == 0x380) {
      brake = (frame.data[0] == 0x20);
      rearFog = (frame.data[1] == 0x20);
    }
    
    if (frame.can_id == 0x060 && frame.can_dlc >= 2) {
      keyPositionRaw = frame.data[1];
      ignitionOn = (keyPositionRaw == 0xA0 || keyPositionRaw == 0xE0); // ON or START
    }
    
    if (frame.can_id == 0x300) {
      lightPanelRawByte = frame.data[1];
      
      // Corrected logic based on real testing:
      // 0x20 = Position lights (always ON when selected)
      // 0x10 = Auto mode (only ON when ignition is ON)
      bool positionLights = (lightPanelRawByte == 0x20 || lightPanelRawByte == 0x18); // Position or low beam
      bool autoModeActive = ((lightPanelRawByte == 0x10 || lightPanelRawByte == 0x28) && ignitionOn); // Auto modes
      
      runningLights = positionLights || autoModeActive;
    }
    
    if (frame.can_id == 0x211 && frame.can_dlc >= 4) {
      hazardRawByte3 = frame.data[2];
      hazardRawByte4 = frame.data[3];

      bool newHazard = (hazardRawByte3 == 0xFC && hazardRawByte4 == 0x43);
      bool newLeft   = (hazardRawByte3 == 0xA8 && hazardRawByte4 == 0x43); // Swapped: A8 is left
      bool newRight  = (hazardRawByte3 == 0x54 && hazardRawByte4 == 0x43); // Swapped: 54 is right

      if (newHazard != prevHazard) {
        if (newHazard) {
          // Hazard just turned ON - start blinking immediately
          hazardBlinkState = true;
          lastHazardBlinkTime = millis();
          digitalWrite(relayPins[0], LOW); // Turn on both
          digitalWrite(relayPins[1], LOW);
        } else {
          // Hazard turned OFF - turn off both relays
          digitalWrite(relayPins[0], HIGH);
          digitalWrite(relayPins[1], HIGH);
        }
        prevHazard = newHazard;
      }
      if (newLeft != prevLeft && !newHazard) {
        if (newLeft) {
          // Left turn signal just turned ON - start blinking immediately
          leftBlinkState = true;
          lastLeftBlinkTime = millis();
          digitalWrite(relayPins[0], LOW);
        } else {
          // Left turn signal turned OFF
          digitalWrite(relayPins[0], HIGH);
        }
        prevLeft = newLeft;
      }
      if (newRight != prevRight && !newHazard) {
        if (newRight) {
          // Right turn signal just turned ON - start blinking immediately
          rightBlinkState = true;
          lastRightBlinkTime = millis();
          digitalWrite(relayPins[1], LOW);
        } else {
          // Right turn signal turned OFF
          digitalWrite(relayPins[1], HIGH);
        }
        prevRight = newRight;
      }

      hazard = newHazard;
      left = newLeft;
      right = newRight;
    }
  }

  // Control relays based on CAN signals
  digitalWrite(relayPins[2], brake ? LOW : HIGH);
  digitalWrite(relayPins[3], reverse ? LOW : HIGH);
  digitalWrite(relayPins[4], frontFog ? LOW : HIGH);
  digitalWrite(relayPins[5], rearFog ? LOW : HIGH);
  digitalWrite(relayPins[6], runningLights ? LOW : HIGH);
  digitalWrite(relayPins[7], HIGH); // unused
}