Filum_Vitae — Code Explainer



This document explains every part of both Root Flow sketches in plain English.

No prior programming knowledge is assumed. Each section maps directly to a

labelled block in the code files so you can find the matching lines easily.



\---



\## How the Two Devices Talk to Each Other



Root Flow uses two separate devices.



The \*\*Soil Pod\*\* is an Arduino UNO. It has no Wi-Fi and cannot reach the

internet. Its only job is to read the sensors plugged into it and wait. When

the Leaf Pod asks for data, the Soil Pod sends a single line of numbers and

goes back to waiting.



The \*\*Leaf Pod\*\* is an ESP32-CAM. It has Wi-Fi. It fetches Seattle weather

from the internet, asks the Soil Pod for readings every 30 seconds, combines

both sets of data, and sends Telegram messages to your phone.



They communicate over a two-wire connection called RS485. Think of it like a

very simple telephone line between the two boards. One wire is called A, the

other is B. Only one device talks at a time — the other listens.



\---



\## Soil Pod Code — Section by Section



\### 1. The Include Lines



```

\#include <Wire.h>

\#include <OneWire.h>

\#include <DallasTemperature.h>

\#include <Adafruit\_AS7341.h>

\#include <Adafruit\_TCS34725.h>

```



These lines tell the Arduino to load pre-written code libraries before your

sketch runs. Each library adds support for a specific sensor or feature.



\- \*\*Wire.h\*\* — adds I2C support. I2C is the communication protocol used by

&#x20; the AS7341 and TCS34725 sensors. Both sensors share two wires: SDA (data)

&#x20; on pin A4 and SCL (clock) on pin A5.

\- \*\*OneWire.h\*\* — adds support for the 1-Wire protocol used by the DS18B20

&#x20; temperature probe.

\- \*\*DallasTemperature.h\*\* — built on top of OneWire, this makes it easy to

&#x20; read a temperature in Celsius or Fahrenheit from the DS18B20.

\- \*\*Adafruit\_AS7341.h\*\* — driver for the NULLLAB AS7341 spectrometer. Even

&#x20; though the board is not from Adafruit, the chip is the same and this

&#x20; library works with it.

\- \*\*Adafruit\_TCS34725.h\*\* — driver for the HiLetgo TCS34725 RGB colour sensor.



\---



\### 2. Pin Definition Constants



```

\#define MOISTURE\_PIN    A0

\#define DS18B20\_PIN     4

\#define DE\_PIN          2

\#define TCS\_LED\_PIN     7

```



A `#define` is a name substitution. Every time the code later says

`MOISTURE\_PIN`, the Arduino reads it as `A0`. This means if you ever rewire

the sensor to a different pin, you only need to change this one line instead

of hunting through the whole sketch.



\- \*\*MOISTURE\_PIN A0\*\* — the capacitive soil sensor's signal wire connects to

&#x20; the Arduino's analog input pin A0. The Arduino measures voltage here and

&#x20; converts it to a number between 0 and 1023.

\- \*\*DS18B20\_PIN 4\*\* — the DS18B20 temperature probe's yellow data wire

&#x20; connects to digital pin 4. It also needs a 4.7 kilohm resistor bridging

&#x20; pin 4 and the 5V pin — without this resistor the sensor cannot communicate.

\- \*\*DE\_PIN 2\*\* — controls the direction of the MAX485 RS485 module. When this

&#x20; pin is HIGH (3.3V or 5V), the module transmits. When LOW (0V), it receives.

&#x20; The DE and RE pins on the MAX485 are wired together so one pin controls both.

\- \*\*TCS\_LED\_PIN 7\*\* — the TCS34725 has a small white LED that illuminates the

&#x20; area in front of it for consistent colour readings. Setting pin 7 HIGH turns

&#x20; this LED on.



\---



\### 3. Moisture Calibration Constants



```

\#define MOISTURE\_DRY    550

\#define MOISTURE\_WET    250

```



The capacitive soil sensor outputs a voltage that changes with moisture. The

Arduino reads this as a number from 0 to 1023. Dry soil gives a higher number;

wet soil gives a lower number — counterintuitive but correct for this sensor

type.



These two values define the ends of the scale. The code uses Arduino's `map()`

function to convert any raw reading between these two numbers into a percentage

from 0 to 100.



Your sensor is powered at 3.3V, which shifts both endpoints lower than the

5V defaults. If the percentage reads wrong after calibration, adjust these

numbers using the instructions printed in the Serial Monitor at startup.



\---



\### 4. Sensor Objects



```

OneWire           oneWire(DS18B20\_PIN);

DallasTemperature tempSensor(\&oneWire);

Adafruit\_AS7341   as7341;

Adafruit\_TCS34725 tcs = Adafruit\_TCS34725(...);

```



These lines create software representations of each physical sensor. Think of

each line as introducing a character that the rest of the code will refer to

by name.



\- \*\*oneWire\*\* — manages the raw 1-Wire electrical signal on pin D4.

\- \*\*tempSensor\*\* — sits on top of oneWire and handles DS18B20-specific

&#x20; commands like requesting a temperature conversion.

\- \*\*as7341\*\* — handles all communication with the spectrometer over I2C.

\- \*\*tcs\*\* — handles communication with the RGB sensor over I2C. The two

&#x20; arguments set the integration time (how long it collects light) and the

&#x20; gain (how much it amplifies the signal). `50 milliseconds` and `4X gain`

&#x20; work well for indoor light levels.



\---



\### 5. The setup() Function



Everything inside `setup()` runs exactly once, when the Arduino first powers

on or resets. It is used for one-time configuration.



\*\*pinMode and digitalWrite\*\* — configure pin 2 as an output and set it LOW

so the MAX485 starts in receive mode. Configure pin 7 as an output and set it

HIGH to turn on the TCS34725 LED.



\*\*Serial.begin(9600)\*\* — starts the serial communication channel at 9600 bits

per second. On the Soil Pod, this same serial channel is also used for RS485

communication with the Leaf Pod. The baud rate must match on both ends.



\*\*Wire.begin()\*\* — activates the I2C bus on pins A4 and A5 so both I2C

sensors can be reached.



\*\*I2C Scanner\*\* — a short loop that checks every possible I2C address (1

through 126) and reports which ones respond. The AS7341 should appear at

address 0x39 and the TCS34725 at address 0x29. If an address is missing, the

sensor at that address has a wiring problem.



\*\*as7341.begin()\*\* — sends the initialisation command to the AS7341 over I2C.

If it returns false, the sensor did not respond and the code halts with a

diagnostic message.



\*\*as7341.setATIME / setASTEP / setGain\*\* — configure how the AS7341 measures

light. ATIME and ASTEP together set the integration time (how long the sensor

collects photons per reading). Gain of 256X amplifies the signal — suitable

for the relatively dim light levels near a plant indoors.



\*\*tcs.begin()\*\* — same pattern as AS7341. Halts with a message if the sensor

does not respond.



\*\*tempSensor.begin() and getDeviceCount()\*\* — initialises the DS18B20 bus and

counts how many probes are found. Unlike the other sensors, a missing DS18B20

prints a warning but does not halt — the sketch continues working without it.



\---



\### 6. The loop() Function



Everything inside `loop()` repeats continuously, thousands of times per

second, for as long as the Arduino has power.



\*\*Serial.available() and Serial.read()\*\* — checks whether any bytes have

arrived over the serial connection (RS485 cable from the Leaf Pod). If a

byte has arrived and it equals 0x52 (the ASCII code for the letter R),

the code calls `sendPacket()` to respond.



\*\*Static unsigned long lastPrint\*\* — a variable that remembers the last time

a bench reading was printed. The `static` keyword means this variable keeps

its value between loop iterations instead of resetting to zero each time.

`millis()` returns the number of milliseconds since the Arduino powered on.

Comparing the current time to `lastPrint` lets the code trigger an action

every 2000 milliseconds (2 seconds) without using `delay()`, which would

block the RS485 listening while waiting.



\---



\### 7. sendPacket() — RS485 Response



This function is called when the Leaf Pod sends a request. It reads all

sensors, formats the results as a single line of comma-separated numbers,

and sends it back over RS485.



\*\*analogRead(MOISTURE\_PIN)\*\* — reads the voltage on pin A0 and returns a

number from 0 (0 volts) to 1023 (3.3 volts on this board). This is the raw

moisture reading.



\*\*map(raw, MOISTURE\_DRY, MOISTURE\_WET, 0, 100)\*\* — rescales the raw number.

If raw equals MOISTURE\_DRY the result is 0. If raw equals MOISTURE\_WET the

result is 100. Values in between are proportional.



\*\*constrain(moisture, 0, 100)\*\* — clamps the result so it can never go below

0 or above 100, even if the raw reading drifts outside the calibration range.



\*\*tempSensor.requestTemperatures()\*\* — sends the DS18B20 a command to

perform a temperature conversion. The sensor then spends about 750

milliseconds measuring before the result is ready.



\*\*getTempCByIndex(0)\*\* — retrieves the converted temperature in Celsius from

the first (and only) probe on the bus. Returns -127.0 if no probe responds.



\*\*as7341.readAllChannels()\*\* — triggers a full spectral reading across all

11 channels simultaneously. Returns true on success.



\*\*as7341.getChannel(...)\*\* — retrieves the count value for a specific

wavelength channel after readAllChannels() has completed.



\*\*tcs.getRawData(\&r, \&g, \&b, \&c)\*\* — reads raw red, green, blue, and clear

(broadband) light counts from the TCS34725.



\*\*calculateColorTemperature\_dn40 / calculateLux\*\* — library functions that

convert raw RGB counts into colour temperature in Kelvin (the warmth or

coolness of the light) and illuminance in lux (overall brightness).



\*\*Packet format\*\* — the resulting string looks like:

`45,21.5,230,410,180,95,1200,340,4200,83`

Each number is separated by a comma. The Leaf Pod splits this string on the

commas to recover each value. A newline character at the end tells the Leaf

Pod the packet is complete.



\*\*digitalWrite(DE\_PIN, HIGH) / LOW\*\* — switches the MAX485 from receive mode

to transmit mode, sends the packet, waits for the serial buffer to empty

(Serial.flush()), then switches back to receive mode. The 100 microsecond

delays give the MAX485 chip time to switch direction before data starts

flowing.



\---



\### 8. printReadings() — Bench Debug Output



This function prints a human-readable version of all sensor readings to the

Serial Monitor every 2 seconds. It serves no purpose once the RS485 cable is

connected and the Leaf Pod is running — at that point the Leaf Pod drives all

output. You can comment out the `printReadings()` call in loop() to disable it.



\---



\### 9. printPad() — Alignment Helper



A small utility function that prints extra spaces before a number so that all

the AS7341 channel values line up in neat columns in the Serial Monitor. Has

no effect on the RS485 data packet.



\---



\---



\## Leaf Pod Code — Section by Section



\### 1. The Include Lines



```

\#include <WiFi.h>

\#include <WiFiClientSecure.h>

\#include <HTTPClient.h>

\#include <ArduinoJson.h>

\#include <UniversalTelegramBot.h>

\#include <HardwareSerial.h>

```



\- \*\*WiFi.h\*\* — connects the ESP32 to your home Wi-Fi network.

\- \*\*WiFiClientSecure.h\*\* — like WiFi.h but adds support for encrypted HTTPS

&#x20; connections. Open-Meteo and Telegram both use HTTPS.

\- \*\*HTTPClient.h\*\* — provides a simple way to make HTTP GET requests (fetch

&#x20; a web page or API). Used to call the Open-Meteo URL.

\- \*\*ArduinoJson.h\*\* — parses JSON. Open-Meteo returns its weather data as a

&#x20; JSON document. This library reads that document and lets you extract

&#x20; individual values by name.

\- \*\*UniversalTelegramBot.h\*\* — handles all Telegram Bot API communication.

&#x20; Sending a message is as simple as calling bot.sendMessage().

\- \*\*HardwareSerial.h\*\* — gives access to the ESP32's second hardware serial

&#x20; port (UART1), used for RS485 communication on GPIO 14 and 15 without

&#x20; interfering with the USB debug port.



\---



\### 2. Credentials



```

const char\*  WIFI\_SSID  = "YOUR\_WIFI\_SSID";

const char\*  WIFI\_PASS  = "YOUR\_WIFI\_PASSWORD";

const String BOT\_TOKEN  = "YOUR\_TELEGRAM\_BOT\_TOKEN";

const String CHAT\_ID    = "YOUR\_TELEGRAM\_CHAT\_ID";

```



These four strings must be filled in before uploading. The sketch will

compile and upload without them but will fail to connect at runtime.



\- \*\*WIFI\_SSID\*\* — your network name exactly as it appears in your phone's

&#x20; Wi-Fi settings. Case-sensitive.

\- \*\*WIFI\_PASS\*\* — your Wi-Fi password. Case-sensitive.

\- \*\*BOT\_TOKEN\*\* — obtained from @BotFather on Telegram. A string of the form

&#x20; `1234567890:ABCdefGhIJKlmNoPQRstuVWXyz`.

\- \*\*CHAT\_ID\*\* — your Telegram account's numeric ID. Obtained by sending your

&#x20; bot any message and visiting

&#x20; `https://api.telegram.org/botYOUR\_TOKEN/getUpdates` in a browser.



\---



\### 3. Open-Meteo URL Parts



The URL is split into four `const char\*` variables because the full URL

(which includes dozens of field names) is too long to fit in one string on

the ESP32's stack. The four pieces are joined into a single String when the

HTTP request is made.



\- \*\*OWM\_BASE\*\* — the server address and your Seattle coordinates (latitude

&#x20; 47.6062, longitude -122.3321), plus the unit settings: Fahrenheit,

&#x20; inches, and metres per second.

\- \*\*OWM\_CURRENT\*\* — the fields you want from the current conditions block:

&#x20; temperature, humidity, wind speed, weather code, etc.

\- \*\*OWM\_DAILY\*\* — today's forecast summary: high, low, precipitation

&#x20; probability, UV index maximum, sunrise, sunset.

\- \*\*OWM\_HOURLY\*\* — hourly model data including outdoor soil temperature at

&#x20; 0 cm and 6 cm depth, outdoor soil moisture, and UV index per hour.



No API key is required. Open-Meteo is a free, open weather service.



\---



\### 4. Alert Thresholds



```

\#define SOIL\_MOISTURE\_CRITICAL    15

\#define SOIL\_MOISTURE\_LOW         30

...

```



These constants define the trigger points for notifications. Adjust them to

suit your specific plant. A succulent needs very different thresholds from

a fern.



Two moisture thresholds exist:

\- \*\*SOIL\_MOISTURE\_LOW (30%)\*\* triggers a standard WARNING message.

\- \*\*SOIL\_MOISTURE\_CRITICAL (15%)\*\* triggers a CRITICAL ALERT with a

&#x20; double Telegram message for haptic double-pulse on the phone.



\---



\### 5. Global Variables



The sketch uses global variables to store the most recent values from both

the Soil Pod and Open-Meteo. These are updated each time a new poll or fetch

runs, and read by `checkAndAlert()` and `sendStatusReport()`.



`soilValid` and `weatherValid` are boolean flags. Before any data has been

received they are false, which prevents the alert system from firing on

uninitialised values.



\---



\### 6. setup() on the Leaf Pod



\*\*Serial.begin(115200)\*\* — starts the debug serial port at 115200 baud for

the USB Serial Monitor. This is separate from the RS485 port.



\*\*RS485Serial.begin(9600, SERIAL\_8N1, RS485\_RX\_PIN, RS485\_TX\_PIN)\*\* —

starts UART1 on GPIO 15 (receive) and GPIO 14 (transmit) at 9600 baud.

The `SERIAL\_8N1` setting means 8 data bits, no parity, 1 stop bit — the

standard for RS485 communication.



\*\*WiFi.begin / while loop\*\* — attempts to connect to Wi-Fi. The `while` loop

waits in 500 millisecond steps until the connection succeeds or 20 seconds

pass (40 attempts × 500 ms). If it fails the sketch continues running but

cannot fetch weather or send Telegram messages.



\*\*secureClient.setInsecure()\*\* — skips TLS certificate verification. This is

acceptable for a sensor project on a private network. Proper certificate

pinning can be added later if needed.



\*\*fetchSeattleWeather()\*\* — called immediately in setup() so the first

Telegram startup message can include current weather data.



\---



\### 7. Accessible Startup Message



The startup message follows the accessibility rules described in the alert

section below. It opens with the words `STATUS UPDATE.` so a screen reader

announces the severity immediately. It writes temperature as a complete

sentence: `"Current Seattle temperature is 52.3 degrees Fahrenheit"` rather

than `"Seattle: 52.3°F"`, which text-to-speech might read as `"52 point 3

degrees F"` or mispronounce the degree symbol.



\---



\### 8. loop() on the Leaf Pod



Two timers run independently using the same `millis()` pattern as the Soil Pod:



\- Every \*\*30 seconds\*\* — call `pollSoilPod()` to request a reading from the UNO.

\- Every \*\*10 minutes\*\* — call `fetchSeattleWeather()` to refresh Open-Meteo data.



After each poll, `checkAndAlert()` is called to evaluate current values

against thresholds.



\---



\### 9. fetchSeattleWeather()



Builds the complete Open-Meteo URL by joining the four string parts, then

makes an HTTPS GET request using `HTTPClient`.



\*\*DynamicJsonDocument doc(6144)\*\* — allocates 6144 bytes of memory on the

heap to hold the parsed JSON. Open-Meteo responses are large (often 3-5 KB

for this many fields). If the allocation is too small, parsing fails silently.

6144 bytes provides a safe margin.



\*\*deserializeJson(doc, http.getStream())\*\* — parses the JSON directly from

the HTTP response stream. This is more memory-efficient than loading the

full response string first.



After parsing, individual values are extracted using the `|` operator which

provides a default value if a field is missing:

`wx\_tempF = cur\["temperature\_2m"] | -999.0f;`

If `temperature\_2m` is absent, `wx\_tempF` becomes -999 instead of crashing.



\*\*Hourly soil model index\*\* — the hourly arrays start at midnight and have

one entry per hour. Index 12 corresponds to noon. To use the actual current

hour, uncomment the NTP block at the bottom of the file, which syncs the

ESP32's clock to an internet time server.



\---



\### 10. pollSoilPod()



Sends the byte `0x52` (the letter R in ASCII) to the UNO over RS485, then

listens for up to 600 milliseconds for a response.



The response is a single line of comma-separated numbers terminated by a

newline. The `nextInt()` and `nextFloat()` helper functions at the bottom

of the file peel values off the front of the string one at a time, which is

why they must be called in the same order the Soil Pod produced them:

moisture, soil temperature, F1, F4, F6, F8, CLR, NIR, CCT, lux.



\---



\### 11. checkAndAlert() — The Accessible Notification System



This is the most important function for blind and low-vision users. Every

design decision here is documented below.



\*\*Alert cooldown\*\* — `ALERT\_COOLDOWN` (5 minutes) prevents the same alert

from firing every 30 seconds if a threshold stays breached. The alert fires

once, then waits 5 minutes before it can fire again.



\*\*Priority label as first word\*\* — every message starts with one of three

words that convey severity:

\- `CRITICAL ALERT.` — immediate action required (frost, near-zero moisture,

&#x20; snow forecast).

\- `WARNING.` — action needed within hours (low moisture, low temperature,

&#x20; high UV).

\- `STATUS UPDATE.` — informational (rain likely today).



When a Telegram notification arrives on a phone, the notification preview

shows the first line of the message. Placing the priority label first means

a blind user's screen reader announces the severity level before they open

the app.



\*\*Full sentences with units spelled out\*\* — text-to-speech engines handle

natural language more reliably than abbreviated forms. The code writes

`"37 percent"` instead of `"37%"`, `"degrees Fahrenheit"` instead of `"°F"`,

`"metres per second"` instead of `"m/s"`. This prevents mispronunciation.



\*\*Action instruction in every alert\*\* — every alert message ends with a

`"Recommended action: ..."` sentence. A blind user receives not just the

problem but the specific step to take, without needing to navigate away from

the notification.



\*\*Double-message haptic pattern for CRITICAL\*\* — when a CRITICAL alert fires,

two Telegram messages are sent 2 seconds apart. Telegram produces a haptic

vibration for each incoming message. Two vibrations in quick succession is a

distinct pattern that differs from the single vibration of a WARNING. A blind

user can distinguish CRITICAL from WARNING by touch alone, without looking at

or listening to the screen.



\*\*Emoji placed after spoken text\*\* — emoji are included as visual decoration

but always appear after the plain text. A screen reader reading left-to-right

speaks the informative words first and may skip or describe the emoji

separately. Meaning is never carried by an emoji alone.



\---



\### 12. sendStatusReport()



Sends a complete reading of all current values as a single accessible

paragraph. Every value is a complete sentence. The report ends with the words

`"End of status report."` so a screen reader user knows the message is

finished and there is no more content to scroll through.



This function is not called automatically. You can wire it to a Telegram bot

command handler so a user can request a full report on demand by sending

`/status` to the bot.



\---



\### 13. wmoDescription()



Open-Meteo uses the World Meteorological Organisation numeric weather codes

instead of text descriptions. This function converts each code to a plain

English phrase that text-to-speech reads naturally. For example, code 63

becomes `"moderate rain"` rather than `"63"`.



\---



\### 14. nextInt() and nextFloat()



These two helper functions parse the CSV packet from the Soil Pod. Each call

reads the next number from the front of a String, removes it (and its

following comma) from the string, and returns the value. They must be called

in the exact order the Soil Pod wrote the values — if you add a new field to

the Soil Pod packet, add a matching `nextInt()` or `nextFloat()` call here in

the same position.



\---



\### 15. NTP Time Sync Block (commented out)



Near the bottom of the Leaf Pod file is a commented block that adds real-time

clock synchronisation using an internet time server (pool.ntp.org). When

active, this makes the hourly soil model index accurate — the sketch picks

the current hour's value from the Open-Meteo hourly arrays instead of always

using noon. Uncomment and follow the inline instructions to enable it.



\---



\## Accessibility Design Summary



| Feature | How it works | Benefit |

|---------|-------------|---------|

| Priority label first | CRITICAL ALERT / WARNING / STATUS UPDATE as first word | Screen reader announces severity in notification preview |

| Full sentence values | "37 percent" not "37%" | Text-to-speech pronounces correctly |

| Action instruction | Every alert ends with "Recommended action: ..." | User knows what to do without extra navigation |

| Double haptic for CRITICAL | Two messages sent 2 seconds apart | Distinguishable by touch without looking at screen |

| Emoji after text | Decorative only, never the sole carrier of meaning | No loss of meaning if emoji are skipped or misread |

| Plain text format | No Markdown bold, no tables, no headers in alerts | Screen readers do not have to parse formatting syntax |

| WMO code translation | Numeric codes converted to plain English | No numeric codes spoken aloud |

| "End of status report" | Explicit terminal phrase | User knows message is complete |



\---



\## Quick Reference — What Each File Does



| File | Board | Sends to | Receives from |

|------|-------|---------|--------------|

| SoilPod\_UNO.ino | Arduino UNO | RS485 packet to Leaf Pod | 0x52 byte request from Leaf Pod |

| LeafPod\_ESP32CAM.ino | ESP32-CAM | Telegram messages to phone | RS485 from Soil Pod, HTTPS from Open-Meteo |



\---



\## Glossary



\*\*I2C\*\* — a two-wire communication protocol (SDA and SCL) that lets multiple

sensors share the same two pins. Each sensor has a unique address so the

microcontroller can talk to them individually.



\*\*RS485\*\* — a two-wire serial bus (A and B) designed for long cable runs and

noisy environments. Unlike standard serial, it uses a differential signal

that is immune to electrical interference.



\*\*MAX485\*\* — the chip that converts the Arduino's standard 5V serial signal

into the differential RS485 signal on the A/B wires, and back again.



\*\*I2C address\*\* — a number (written in hexadecimal, e.g. 0x39) that

identifies a specific sensor on the I2C bus. Like a house number on a street.



\*\*ADC\*\* — Analog to Digital Converter. The Arduino's built-in circuit that

measures a voltage and converts it to a number between 0 and 1023.



\*\*JSON\*\* — JavaScript Object Notation. A text format used by web APIs to

send structured data. Open-Meteo uses it to send weather readings.



\*\*WMO code\*\* — a standardised numeric code from the World Meteorological

Organisation that describes weather conditions. Code 63 means moderate rain,

code 0 means clear sky, and so on.



\*\*Lux\*\* — a unit of illuminance. 1 lux is the brightness of a candle 1 metre

away. Bright office lighting is about 500 lux. A sunny day outdoors is about

100,000 lux.



\*\*Kelvin (colour temperature)\*\* — describes the warmth or coolness of a light

source. Around 2700K is warm/amber (like a sunset or incandescent bulb).

Around 6500K is cool/blue (like overcast daylight). Plants generally prefer

light in the 3000-6500K range depending on growth stage.



\*\*m³/m³ (cubic metres per cubic metre)\*\* — the unit Open-Meteo uses for soil

moisture in its model. It represents the volume of water per volume of soil.

A value of 0.1 means 10% of the soil volume is water. Field capacity (ideal

for most plants) is roughly 0.25 to 0.40 depending on soil type.





https://api.open-meteo.com/v1/forecast?latitude=47.6062\&longitude=-122.3321\&daily=temperature\_2m\_max,weather\_code,temperature\_2m\_min,apparent\_temperature\_max,apparent\_temperature\_min,sunrise,sunset,daylight\_duration,sunshine\_duration,uv\_index\_max,uv\_index\_clear\_sky\_max,rain\_sum,showers\_sum,snowfall\_sum,precipitation\_sum,precipitation\_hours,precipitation\_probability\_max,wind\_speed\_10m\_max,wind\_gusts\_10m\_max,wind\_direction\_10m\_dominant,shortwave\_radiation\_sum,et0\_fao\_evapotranspiration,visibility\_mean,visibility\_min,visibility\_max,winddirection\_10m\_dominant,wind\_gusts\_10m\_mean,wind\_speed\_10m\_mean,wind\_gusts\_10m\_min,wind\_speed\_10m\_min,dew\_point\_2m\_max,dew\_point\_2m\_mean,dew\_point\_2m\_min,cloud\_cover\_max,cloud\_cover\_min,temperature\_2m\_mean\&hourly=temperature\_2m,relative\_humidity\_2m,dew\_point\_2m,precipitation\_probability,apparent\_temperature,precipitation,rain,showers,snowfall,snow\_depth,weather\_code,pressure\_msl,surface\_pressure,cloud\_cover\_low,cloud\_cover,cloud\_cover\_mid,cloud\_cover\_high,visibility,evapotranspiration,et0\_fao\_evapotranspiration,vapour\_pressure\_deficit,wind\_speed\_10m,wind\_speed\_80m,wind\_speed\_120m,wind\_speed\_180m,wind\_direction\_10m,wind\_direction\_80m,wind\_direction\_120m,wind\_direction\_180m,temperature\_80m,wind\_gusts\_10m,temperature\_120m,temperature\_180m,soil\_temperature\_0cm,soil\_temperature\_6cm,soil\_temperature\_18cm,soil\_temperature\_54cm,soil\_moisture\_0\_to\_1cm,soil\_moisture\_1\_to\_3cm,soil\_moisture\_3\_to\_9cm,soil\_moisture\_9\_to\_27cm,soil\_moisture\_27\_to\_81cm,uv\_index,uv\_index\_clear\_sky,is\_day,sunshine\_duration\&current=temperature\_2m,relative\_humidity\_2m,apparent\_temperature,precipitation,rain,showers,weather\_code,cloud\_cover,wind\_speed\_10m,wind\_direction\_10m,wind\_gusts\_10m,is\_day\&timezone=America%2FLos\_Angeles\&wind\_speed\_unit=ms\&temperature\_unit=fahrenheit\&precipitation\_unit=inch\&start\_date=2026-04-24\&end\_date=2026-05-15

