# 📄 ESP32 Smart Irrigation Firmware Documentation (`irrigation.ino`)

This document provides a technical breakdown of the `irrigation.ino` C++ sketch used in the **ESP32 Smart Irrigation System**. The firmware manages soil moisture monitoring, OpenWeatherMap API integration, active-low relay pump control, and ThingSpeak cloud telemetry logging.

---

## 📑 Table of Contents
1. [Firmware Overview](#-firmware-overview)
2. [Libraries & Dependencies](#-libraries--dependencies)
3. [Hardware Pin Definitions & Constants](#-hardware-pin-definitions--constants)
4. [System Configuration & Variables](#-system-configuration--variables)
5. [Core Functions & Logic Breakdown](#-core-functions--logic-breakdown)
6. [Decision Logic & Control Flow](#-decision-logic--control-flow)
7. [ThingSpeak & Weather API Protocols](#-thingspeak--weather-api-protocols)
8. [Configuration & Upload Instructions](#-configuration--upload-instructions)

---

## 🌿 Firmware Overview

The `irrigation.ino` sketch operates an automated closed-loop irrigation system. It reads analog data from a soil moisture probe, fetches local weather forecasts over Wi-Fi from OpenWeatherMap, conditionally activates a mini DC water pump via a relay module, and streams telemetry metrics to a ThingSpeak dashboard.

### Key Capabilities:
* **Analog-to-Digital Conversion:** Converts raw 12-bit ADC readings into a percentage (0–100%).
* **Precipitation Override:** Automatically halts irrigation if active precipitation ("Rain", "Drizzle", "Thunderstorm") is reported in the local area.
* **Timed Safety Pump Cycle:** Runs the water pump in bursts up to 5 seconds or until the moisture threshold is satisfied.
* **Cloud Telemetry:** Updates ThingSpeak every 5 minutes with real-time moisture level and outside temperature.

---

## 📚 Libraries & Dependencies

| Library | Header File | Function / Purpose |
| :--- | :--- | :--- |
| **ESP32 Wi-Fi** | `<WiFi.h>` | Establishes and maintains 802.11 b/g/n wireless connections. |
| **HTTP Client** | `<HTTPClient.h>` | Executes HTTP GET requests to OpenWeatherMap and ThingSpeak endpoints. |
| **ArduinoJson** | `<ArduinoJson.h>` | Parses incoming JSON payloads from OpenWeatherMap REST API responses. |

---

## 🔌 Hardware Pin Definitions & Constants

```cpp
#define MOISTURE_PIN 34 // Analog Input: Soil moisture sensor signal line (ADC1_CH6)
#define RELAY_PIN 5     // Digital Output: Control signal for Active-Low Relay
```

| Pin Name | ESP32 GPIO | Direction | Default State | Description |
| :--- | :--- | :--- | :--- | :--- |
| `MOISTURE_PIN` | **GPIO 34** | Input (Analog) | N/A | Connects to soil moisture sensor analog output. |
| `RELAY_PIN` | **GPIO 5** | Output (Digital) | `HIGH` (OFF) | Connects to active-low relay module signal line. |

---

## ⚙️ System Configuration & Variables

### 1. Thresholds & Timing Intervals
* `dryThreshold = 85`: Soil moisture threshold percentage.
* `intervalWeather = 600000`: Fetch weather every 10 minutes (600,000 ms).
* `intervalPump = 5000`: Maximum continuous pump run duration (5,000 ms).
* `intervalSendData = 300000`: Post data to ThingSpeak every 5 minutes (300,000 ms).

### 2. Network & API Credentials
* `ssid`: Wi-Fi Access Point Name (`"Agarwals@77"`).
* `password`: Wi-Fi Password (`"Punish@2124"`).
* `serverName`: ThingSpeak Endpoint (`"http://api.thingspeak.com/update"`).
* `apiKey`: ThingSpeak Write API Key (`"WEFE9LC69OIDI6XP"`).
* `openWeatherApiKey`: OpenWeatherMap API Key (`"83d17aa919ff191766c5b1cfeca32041"`).
* `city`: Location query (`"Nairobi,KE"`).

---

## 🔍 Core Functions & Logic Breakdown

### 1. `Read_Soil_Moisture()`
Reads raw voltage from GPIO 34 and maps it to a calibrated 0–100% moisture range.
```cpp
int Read_Soil_Moisture(){
  int rawMoisture = analogRead(MOISTURE_PIN);
  int Percent = map(rawMoisture, 4100, 1000, 0, 100);
  Percent = constrain(Percent, 0, 100);
  return Percent;
}
```
* **Calibration Range:**
  * `4100` raw ADC units maps to **0% Moisture**.
  * `1000` raw ADC units maps to **100% Moisture**.
* **`constrain()`:** Guarantees returned moisture values remain strictly between `0` and `100`.

---

### 2. `setup()`
Initializes serial communication, hardware pin modes, and Wi-Fi connection.
```cpp
void setup() {
  Serial.begin(115200);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH); // Active-Low Relay: HIGH keeps pump OFF at boot

  WiFi.begin(ssid, password);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}
```
* **Safety Isolation:** Sets `RELAY_PIN` to `HIGH` immediately during setup to keep the relay deactivated when powered on.

---

### 3. `loop()`
Executes non-blocking timing logic using `millis()` to manage weather updates, irrigation decisions, and cloud uploads.

#### A. Weather Fetch Routine
Every 10 minutes (`intervalWeather = 600000`), the ESP32 issues an HTTP GET request to OpenWeatherMap.
Parses JSON response:
* `apiTemp`: Outside temperature in Celsius (`doc["main"]["temp"]`).
* `apiHumidity`: Relative humidity (`doc["main"]["humidity"]`).
* `apiCondition`: Main weather condition string (`doc["weather"][0]["main"]`).

#### B. Pump Control & Rain Bypass Logic
* If `apiCondition` is `"Rain"`, `"Drizzle"`, or `"Thunderstorm"`, the pump remains `OFF` (`digitalWrite(RELAY_PIN, HIGH)`) regardless of soil moisture.
* If conditions are clear and `moisturePercent < dryThreshold` (85%), the pump is activated (`digitalWrite(RELAY_PIN, LOW)`).
* The pump runs until `moisturePercent >= dryThreshold` or the 5-second runtime limit (`intervalPump = 5000`) expires.

#### C. ThingSpeak Data Publishing
Every 5 minutes (`intervalSendData = 300000`), sensor data is transmitted to ThingSpeak:
* **Field 1:** `moisturePercent`
* **Field 2:** `apiTemp`

---

## 🛠️ Configuration & Upload Instructions

1. Open `Source/irrigation.ino` in the **Arduino IDE**.
2. Select **ESP32 Dev Module** under `Tools -> Board -> ESP32 Arduino`.
3. Verify network parameters (`ssid`, `password`) and API keys in the sketch.
4. Connect the ESP32 board via USB and click **Upload**.
5. Open the **Serial Monitor** at `115200` baud to view real-time diagnostic output.
