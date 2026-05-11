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

// ---- Sensor objects -----------------------------------------
OneWire           oneWire(DS18B20_PIN);
DallasTemperature tempSensor(&oneWire);
Adafruit_AS7341   as7341;
Adafruit_TCS34725 tcs = Adafruit_TCS34725(
  TCS34725_INTEGRATIONTIME_50MS,
  TCS34725_GAIN_4X
);

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
  Serial.println(F("Readings every 2 seconds."));
  Serial.println(F("Send byte 0x52 to trigger RS485 packet."));
  Serial.println(F("----------------------------------------"));
}

// ============================================================
void loop() {
  if (Serial.available() > 0) {
    byte cmd = Serial.read();
    if (cmd == 0x52) {
      sendPacket();
      return;
    }
  }

  static unsigned long lastPrint = 0;
  if (millis() - lastPrint >= 2000) {
    lastPrint = millis();
    printReadings();
  }
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

