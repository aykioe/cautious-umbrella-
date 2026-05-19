// ============================================================
// Root Flow — Soil Pod (Arduino Nano)
// Reads: moisture, soil temp, RGB light
// Sends: RS485 packet to Leaf Pod on request
// ============================================================

// CONFIRMED WIRING:
//   A0       <- Capacitive Soil Sensor v2.0 AOUT
//   A4 (SDA) <- AS7341 SDA + TCS34725 SDA  (shared I2C bus)
//   A5 (SCL) <- AS7341 SCL + TCS34725 SCL  (shared I2C bus)
//   3.3V     <- AS7341 VCC + TCS34725 VCC + Capacitive Sensor VCC
//   GND      <- All sensor GND
//   D2       <- MAX485 DE+RE (reserved for RS485 — wire when ready)
//   D4       <- DS18B20 yellow (data) wire
//   D7       <- TCS34725 LED enable
//
//   DS18B20 wiring:
//     Red    -> UNO 5V
//     Black  -> UNO GND
//     Yellow -> UNO D4
//     4.7kΩ resistor bridges UNO 5V and UNO D4  <- mandatory
//
// 
// RS485 PACKET FORMAT (sent on byte 0x52 request):
//   "moisture,soilTempC,F1,F4,F6,F8,CLR,NIR,CCT,LUX\n"
//
// Seattle ambient weather is fetched by the ESP32-CAM Leaf Pod
// over Wi-Fi and does not travel over this RS485 link.

#include <Wire.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_AS7341.h>
#include <Adafruit_TCS34725.h>

// ---- Pin definitions ----------------------------------------
#define MOISTURE_PIN    A0
#define DS18B20_PIN     4
#define DE_PIN          2
#define TCS_LED_PIN     7

// ---- Moisture calibration -----------------------------------
#define MOISTURE_DRY    550
#define MOISTURE_WET    250

// ==========================================================
// VOLUMETRIC WATER CONTENT (VWC) EXPIREMENT CONSTANTS
// ==========================================================
// Soil volume: 90 cubic inches -> converted to cm^3
// 1 cubic inch = 16.387 cm^3
// 90 in^3 x 16.387 = 1474.83 cm^3
#define SOIL_VOLUME_CM3   1474.83f

// Soil porosity assumption: 0.43 (standard loamy potting mix)
// This means 43% of the soil volume can hold water 
// Max water capacity = 0.43 x 1474.83 = 634.18 ml
#define SOIL_POROSITY     0.43f

// Water stages in mL
// Stage 0 = dry baseline (0 mL added)
// Stages 1-4 = cumulative water added
const float WATER_ADDED_ML[5] = { 0.0f, 30.0f, 50.0f, 70.0f, 90.0f };
const char* STAGE_LABELS[5]   = {
  "Baseline (0 mL)",
  "After 30 mL",
  "After 50 mL",
  "After 70 mL",
  "After 90 mL"
};

// Number of measurement locations per stage
#define NUM_LOCATIONS   3

// Success window from criteria: sensor reads 93–97% of theoretical VWC
#define SUCCESS_MIN     0.93f
#define SUCCESS_MAX     0.97f

// ============================================================
// EXPERIMENT STATE
// ============================================================
// Stores raw moisture readings: [stage][location]
int   sessionRaw[5][3];
int   sessionMoisture[5][3];   // mapped 0-100%
float sessionVWC_theoretical[5]; // theoretical VWC % for each stage
float sessionVWC_measured[5];    // average measured VWC % across 3 locations
float sessionError[5];           // % error per stage
bool  sessionComplete = false;
bool  sessionActive   = false;
int   currentStage    = 0;
int   currentLocation = 0;

// ---- Sensor objects -----------------------------------------
OneWire           oneWire(DS18B20_PIN);
DallasTemperature tempSensor(&oneWire);
Adafruit_AS7341   as7341;
Adafruit_TCS34725 tcs = Adafruit_TCS34725(
  TCS34725_INTEGRATIONTIME_50MS,
  TCS34725_GAIN_4X
);

// ============================================================
// SETUP
// ============================================================
void setup() {
  pinMode(DE_PIN, OUTPUT);
  digitalWrite(DE_PIN, LOW);

  pinMode(TCS_LED_PIN, OUTPUT);
  digitalWrite(TCS_LED_PIN, HIGH);

  Serial.begin(9600);
  while (!Serial);

  Wire.begin();

  Serial.println(F("========================================"));
  Serial.println(F("  Root Flow Soil Pod"));
  Serial.println(F("========================================"));
  Serial.println();

  // I2C scan
  Serial.println(F("Scanning I2C bus..."));
  byte found = 0;
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print(F("  Found 0x"));
      if (addr < 16) Serial.print(F("0"));
      Serial.print(addr, HEX);
      if (addr == 0x29) Serial.print(F("  <- TCS34725"));
      if (addr == 0x39) Serial.print(F("  <- AS7341"));
      Serial.println();
      found++;
    }
  }
  if (found < 2) {
    Serial.println(F("  WARNING: expected 0x29 and 0x39"));
    Serial.println(F("  Check SDA->A4, SCL->A5, VCC->3.3V, GND->GND"));
  }
  Serial.println();

  // AS7341
  Serial.print(F("AS7341   ... "));
  if (!as7341.begin()) {
    Serial.println(F("FAILED. Check A4/A5 wiring. Halting."));
    while (1) { delay(10); }
  }
  as7341.setATIME(100);
  as7341.setASTEP(999);
  as7341.setGain(AS7341_GAIN_256X);
  Serial.println(F("OK"));

  // TCS34725
  Serial.print(F("TCS34725 ... "));
  if (!tcs.begin()) {
    Serial.println(F("FAILED. Check A4/A5 wiring. Halting."));
    while (1) { delay(10); }
  }
  Serial.println(F("OK"));

  // DS18B20
  Serial.print(F("DS18B20  ... "));
  tempSensor.begin();
  byte probeCount = tempSensor.getDeviceCount();
  if (probeCount == 0) {
    Serial.println(F("NOT FOUND"));
    Serial.println(F("  Check: Red->5V  Black->GND  Yellow->D4"));
    Serial.println(F("  Check: 4.7k resistor bridges 5V rail and D4 row"));
    Serial.println(F("  Continuing without soil temperature."));
  } else {
    Serial.print(probeCount);
    Serial.println(F(" probe(s) found. OK"));
  }

  Serial.println();
  Serial.println(F("Live readings every 2 seconds."));
  Serial.println(F("----------------------------------------"));
  Serial.println(F("COMMANDS:"));
  Serial.println(F("  S  — start guided moisture experiment"));
  Serial.println(F("  R  — print experiment report"));
  Serial.println(F("----------------------------------------"));
}

// ============================================================
// LOOP
// ============================================================
void loop() {
  if (Serial.available() > 0) {
    byte cmd = Serial.read();

    if (cmd == 'S' || cmd == 's') {
      startSession();
      return;
    }
    if (cmd == 'R' || cmd == 'r') {
      printReport();
      return;
    }
    // Space or Enter while session active = record this location
    if (sessionActive && (cmd == ' ' || cmd == '\r' || cmd == '\n')) {
      recordLocation();
      return;
    }
    if (cmd == 0x52) {
      sendPacket();
      return;
    }
  }

  static unsigned long lastPrint = 0;
  if (millis() - lastPrint >= 2000) {
    lastPrint = millis();
    if (!sessionActive) {
      printReadings();
    } else {
      // During session: just print live moisture so user can watch it stabilize
      int raw = analogRead(MOISTURE_PIN);
      int pct = constrain(map(raw, MOISTURE_DRY, MOISTURE_WET, 0, 100), 0, 100);
      Serial.print(F("  Live moisture: "));
      Serial.print(pct);
      Serial.print(F("%  (raw "));
      Serial.print(raw);
      Serial.println(F(")  — press SPACE or ENTER to record"));
    }
  }
}

// ============================================================
// SESSION — start
// ============================================================
void startSession() {
  sessionActive   = true;
  sessionComplete = false;
  currentStage    = 0;
  currentLocation = 0;

  // Pre-calculate theoretical VWC for each stage
  // Theoretical VWC (%) = (water added mL / max water capacity mL) × 100
  // Max water capacity = SOIL_POROSITY × SOIL_VOLUME_CM3
  // (1 mL water = 1 cm³)
  float maxCapacity = SOIL_POROSITY * SOIL_VOLUME_CM3; // 634.18 mL

  for (int s = 0; s < 5; s++) {
    // Clamp: can't exceed 100% of pore space
    float fraction = WATER_ADDED_ML[s] / maxCapacity;
    if (fraction > 1.0f) fraction = 1.0f;
    sessionVWC_theoretical[s] = fraction * 100.0f;
  }

  Serial.println();
  Serial.println(F("========================================"));
  Serial.println(F("  MOISTURE EXPERIMENT — SESSION START"));
  Serial.println(F("========================================"));
  Serial.println(F("Soil volume : 90 in³ (1474.83 cm³)"));
  Serial.println(F("Porosity    : 0.43 (loamy potting mix)"));
  Serial.println(F("Max capacity: 634.18 mL"));
  Serial.println(F("Success band: 93% – 97% of theoretical VWC"));
  Serial.println();
  Serial.println(F("Theoretical VWC per stage:"));
  for (int s = 0; s < 5; s++) {
    Serial.print(F("  "));
    Serial.print(STAGE_LABELS[s]);
    Serial.print(F(" -> "));
    Serial.print(sessionVWC_theoretical[s], 2);
    Serial.println(F("%"));
  }
  Serial.println();
  promptNextLocation();
}

// ============================================================
// SESSION — prompt user for next action
// ============================================================
void promptNextLocation() {
  Serial.println(F("----------------------------------------"));

  if (currentStage == 0) {
    Serial.println(F("STAGE 0 — Baseline (no water added yet)"));
  } else {
    Serial.print(F("STAGE "));
    Serial.print(currentStage);
    Serial.print(F(" — "));
    Serial.println(STAGE_LABELS[currentStage]);

    // Prompt watering only at start of a new stage (location 0)
    if (currentLocation == 0) {
      Serial.print(F(">>> Pour water to reach "));
      Serial.print((int)WATER_ADDED_ML[currentStage]);
      Serial.println(F(" mL total, then press ENTER to continue."));
      Serial.println(F("    Wait ~30 seconds for water to absorb."));
    }
  }

  Serial.print(F("Insert sensor at location "));
  Serial.print(currentLocation + 1);
  Serial.println(F(" of 3 (3 inches deep)."));
  Serial.println(F("Let reading stabilize, then press SPACE or ENTER to record."));
}

// ============================================================
// SESSION — record one location reading
// ============================================================
void recordLocation() {
  int raw = analogRead(MOISTURE_PIN);
  int pct = constrain(map(raw, MOISTURE_DRY, MOISTURE_WET, 0, 100), 0, 100);

  sessionRaw[currentStage][currentLocation]      = raw;
  sessionMoisture[currentStage][currentLocation] = pct;

  Serial.print(F("  Recorded location "));
  Serial.print(currentLocation + 1);
  Serial.print(F(": "));
  Serial.print(pct);
  Serial.print(F("%  (raw "));
  Serial.print(raw);
  Serial.println(F(")"));

  currentLocation++;

  if (currentLocation >= NUM_LOCATIONS) {
    // All 3 locations done for this stage — compute averages
    float avg = 0;
    for (int l = 0; l < NUM_LOCATIONS; l++) {
      avg += sessionMoisture[currentStage][l];
    }
    avg /= NUM_LOCATIONS;
    sessionVWC_measured[currentStage] = avg;

    // % error = |measured - theoretical| / theoretical × 100
    float theory = sessionVWC_theoretical[currentStage];
    float err = 0.0f;
    if (theory > 0.0f) {
      err = abs(avg - theory) / theory * 100.0f;
    }
    sessionError[currentStage] = err;

    // Ratio of measured to theoretical (for pass/fail)
    float ratio = (theory > 0.0f) ? (avg / theory) : 0.0f;
    bool  pass  = (ratio >= SUCCESS_MIN && ratio <= SUCCESS_MAX);

    Serial.println();
    Serial.print(F("  Stage average   : "));
    Serial.print(avg, 1);
    Serial.println(F("%"));
    Serial.print(F("  Theoretical VWC : "));
    Serial.print(theory, 2);
    Serial.println(F("%"));
    Serial.print(F("  % error         : "));
    Serial.print(err, 2);
    Serial.println(F("%"));
    Serial.print(F("  Ratio measured/theoretical: "));
    Serial.print(ratio * 100.0f, 1);
    Serial.println(F("%"));
    Serial.print(F("  Result          : "));
    Serial.println(pass ? F("PASS (within 93-97% of theoretical)") : F("FAIL (outside 93-97% band)"));
    Serial.println();

    currentLocation = 0;
    currentStage++;

    if (currentStage >= 5) {
      // All stages complete
      sessionActive   = false;
      sessionComplete = true;
      Serial.println(F("========================================"));
      Serial.println(F("  ALL STAGES COMPLETE"));
      Serial.println(F("  Type R to print the full report."));
      Serial.println(F("========================================"));
    } else {
      promptNextLocation();
    }
  } else {
    promptNextLocation();
  }
}

// ============================================================
// REPORT — full summary with % error table
// ============================================================
void printReport() {
  if (!sessionComplete) {
    Serial.println(F("No complete session yet. Type S to start."));
    return;
  }

  Serial.println();
  Serial.println(F("========================================"));
  Serial.println(F("  ROOT FLOW — EXPERIMENT REPORT"));
  Serial.println(F("========================================"));
  Serial.println(F("Soil volume : 90 in³ (1474.83 cm³)"));
  Serial.println(F("Porosity    : 0.43"));
  Serial.println(F("Max capacity: 634.18 mL"));
  Serial.println(F("Success band: 93% – 97% of theoretical VWC"));
  Serial.println();

  // Raw data table
  Serial.println(F("--- Raw Readings per Location ---"));
  Serial.println(F("Stage           | Loc1  Loc2  Loc3  | Avg%  | Theory%| %Err | Pass?"));
  Serial.println(F("----------------|-------------------|-------|--------|------|------"));

  int passCount = 0;
  for (int s = 0; s < 5; s++) {
    // Stage label padded
    Serial.print(STAGE_LABELS[s]);
    // pad to 16 chars
    int labelLen = strlen(STAGE_LABELS[s]);
    for (int p = labelLen; p < 16; p++) Serial.print(' ');
    Serial.print(F("| "));

    for (int l = 0; l < NUM_LOCATIONS; l++) {
      int v = sessionMoisture[s][l];
      if (v < 10)  Serial.print(' ');
      if (v < 100) Serial.print(' ');
      Serial.print(v);
      Serial.print(F("%  "));
    }
    Serial.print(F("| "));

    float avg = sessionVWC_measured[s];
    if (avg < 10.0f) Serial.print(' ');
    Serial.print(avg, 1);
    Serial.print(F("% | "));

    float theory = sessionVWC_theoretical[s];
    if (theory < 10.0f) Serial.print(' ');
    Serial.print(theory, 2);
    Serial.print(F("% | "));

    float err = sessionError[s];
    if (err < 10.0f) Serial.print(' ');
    Serial.print(err, 2);
    Serial.print(F("% | "));

    float ratio = (theory > 0.0f) ? (avg / theory) : 0.0f;
    bool  pass  = (ratio >= SUCCESS_MIN && ratio <= SUCCESS_MAX);
    if (pass) passCount++;
    Serial.println(pass ? F("PASS") : F("FAIL"));
  }

  Serial.println();
  Serial.print(F("Stages passed: "));
  Serial.print(passCount);
  Serial.println(F(" / 5"));
  Serial.println();

  // Overall verdict
  if (passCount == 5) {
    Serial.println(F("OVERALL: SUCCESS"));
    Serial.println(F("Sensor accuracy within 93-97% of theoretical"));
    Serial.println(F("VWC across all water stages. Criteria met."));
  } else {
    Serial.println(F("OVERALL: CRITERIA NOT FULLY MET"));
    Serial.println(F("Review failed stages. Consider:"));
    Serial.println(F("  - Recalibrate MOISTURE_DRY / MOISTURE_WET"));
    Serial.println(F("  - Allow more absorption time between pours"));
    Serial.println(F("  - Check sensor insertion depth (3 inches)"));
  }
  Serial.println(F("========================================"));
}

// ============================================================
// RS485 PACKET
// Format: "moisture,soilTempC,F1,F4,F6,F8,CLR,NIR,CCT,LUX\n"
// ============================================================
void sendPacket() {
  int raw      = analogRead(MOISTURE_PIN);
  int moisture = constrain(map(raw, MOISTURE_DRY, MOISTURE_WET, 0, 100), 0, 100);

  float soilTempC = -127.0;
  if (tempSensor.getDeviceCount() > 0) {
    tempSensor.requestTemperatures();
    soilTempC = tempSensor.getTempCByIndex(0);
  }

  if (!as7341.readAllChannels()) {
    digitalWrite(DE_PIN, HIGH);
    delayMicroseconds(100);
    Serial.println(F("ERR"));
    Serial.flush();
    delayMicroseconds(100);
    digitalWrite(DE_PIN, LOW);
    return;
  }
  uint16_t f1  = as7341.getChannel(AS7341_CHANNEL_415nm_F1);
  uint16_t f4  = as7341.getChannel(AS7341_CHANNEL_515nm_F4);
  uint16_t f6  = as7341.getChannel(AS7341_CHANNEL_590nm_F6);
  uint16_t f8  = as7341.getChannel(AS7341_CHANNEL_680nm_F8);
  uint16_t clr = as7341.getChannel(AS7341_CHANNEL_CLEAR);
  uint16_t nir = as7341.getChannel(AS7341_CHANNEL_NIR);

  uint16_t r, g, b, c;
  tcs.getRawData(&r, &g, &b, &c);
  uint16_t cct = tcs.calculateColorTemperature_dn40(r, g, b, c);
  uint16_t lux = tcs.calculateLux(r, g, b);

  String pkt = String(moisture)     + "," +
               String(soilTempC, 1) + "," +
               String(f1)           + "," +
               String(f4)           + "," +
               String(f6)           + "," +
               String(f8)           + "," +
               String(clr)          + "," +
               String(nir)          + "," +
               String(cct)          + "," +
               String(lux)          + "\n";

  digitalWrite(DE_PIN, HIGH);
  delayMicroseconds(100);
  Serial.print(pkt);
  Serial.flush();
  delayMicroseconds(100);
  digitalWrite(DE_PIN, LOW);
}

// ============================================================
// BENCH PRINT — human readable, every 2 seconds
// ============================================================
void printReadings() {
  int raw      = analogRead(MOISTURE_PIN);
  int moisture = constrain(map(raw, MOISTURE_DRY, MOISTURE_WET, 0, 100), 0, 100);
  Serial.print(F("Moisture  | Raw: "));
  Serial.print(raw);
  Serial.print(F("  -> "));
  Serial.print(moisture);
  Serial.print(F("%"));
  if (raw > 580) Serial.print(F("  [open air - calibrate MOISTURE_DRY]"));
  if (raw < 240) Serial.print(F("  [in water - calibrate MOISTURE_WET]"));
  Serial.println();

  Serial.print(F("DS18B20   | "));
  if (tempSensor.getDeviceCount() == 0) {
    Serial.println(F("Not connected"));
    Serial.println(F("           Check: Red->5V  Black->GND  Yellow->D4"));
    Serial.println(F("           Check: 4.7k resistor between 5V rail and D4 row"));
  } else {
    tempSensor.requestTemperatures();
    float tempC = tempSensor.getTempCByIndex(0);
    float tempF = (tempC * 9.0 / 5.0) + 32.0;
    if (tempC == -127.0) {
      Serial.println(F("Probe not responding (-127)"));
      Serial.println(F("           Most likely cause: 4.7k resistor missing or"));
      Serial.println(F("           wired to GND instead of 5V rail."));
    } else {
      Serial.print(tempC, 1);
      Serial.print(F(" C  /  "));
      Serial.print(tempF, 1);
      Serial.print(F(" F"));
      if (tempC < 5.0)                         Serial.print(F("  [COLD - root activity stops below 5C]"));
      else if (tempC < 10.0)                   Serial.print(F("  [cool - slow root growth]"));
      else if (tempC >= 18.0 && tempC <= 24.0) Serial.print(F("  [ideal range for most houseplants]"));
      else if (tempC > 30.0)                   Serial.print(F("  [warm - monitor moisture more frequently]"));
      Serial.println();
    }
  }

  if (as7341.readAllChannels()) {
    uint16_t f1  = as7341.getChannel(AS7341_CHANNEL_415nm_F1);
    uint16_t f2  = as7341.getChannel(AS7341_CHANNEL_445nm_F2);
    uint16_t f3  = as7341.getChannel(AS7341_CHANNEL_480nm_F3);
    uint16_t f4  = as7341.getChannel(AS7341_CHANNEL_515nm_F4);
    uint16_t f5  = as7341.getChannel(AS7341_CHANNEL_555nm_F5);
    uint16_t f6  = as7341.getChannel(AS7341_CHANNEL_590nm_F6);
    uint16_t f7  = as7341.getChannel(AS7341_CHANNEL_630nm_F7);
    uint16_t f8  = as7341.getChannel(AS7341_CHANNEL_680nm_F8);
    uint16_t nir = as7341.getChannel(AS7341_CHANNEL_NIR);
    uint16_t clr = as7341.getChannel(AS7341_CHANNEL_CLEAR);
    Serial.println(F("AS7341    | 415   445   480   515   555   590   630   680   NIR   CLR"));
    Serial.print  (F("          | "));
    printPad(f1); printPad(f2); printPad(f3); printPad(f4);
    printPad(f5); printPad(f6); printPad(f7); printPad(f8);
    printPad(nir); printPad(clr);
    Serial.println();
  } else {
    Serial.println(F("AS7341    | read failed"));
  }

  uint16_t r, g, b, c;
  tcs.getRawData(&r, &g, &b, &c);
  uint16_t cct = tcs.calculateColorTemperature_dn40(r, g, b, c);
  uint16_t lux = tcs.calculateLux(r, g, b);
  Serial.print(F("TCS34725  | R:"));
  Serial.print(r);
  Serial.print(F("  G:"));
  Serial.print(g);
  Serial.print(F("  B:"));
  Serial.print(b);
  Serial.print(F("  CCT:"));
  Serial.print(cct);
  Serial.print(F("K  Lux:"));
  Serial.println(lux);

  Serial.println(F("----------------------------------------"));
}

void printPad(uint16_t v) {
  if      (v <    10) Serial.print(F("     "));
  else if (v <   100) Serial.print(F("    "));
  else if (v <  1000) Serial.print(F("   "));
  else if (v < 10000) Serial.print(F("  "));
  else                Serial.print(F(" "));
  Serial.print(v);
}
