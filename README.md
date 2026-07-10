# **🌿 ESP32 Smart Irrigation System**

This is an automated, internet-connected plant watering system built with an ESP32. The system monitors soil moisture and controls a water pump via a relay. It also integrates with the OpenWeatherMap API to check local weather conditions, skipping scheduled watering if it is currently raining. Data is logged to ThingSpeak for remote monitoring.

## **✨ Features**

* 💧 **Automated Watering:** Triggers a water pump based on analog soil moisture readings.  
* 🌦️ **Weather API Integration:** Checks OpenWeatherMap to prevent the pump from running during rain, drizzle, or thunderstorms.  
* 📊 **Cloud Data Logging:** Pushes local soil moisture and outdoor weather data to a ThingSpeak dashboard.  
* 🔋 **Isolated Power:** Uses separate power supplies for the ESP32 and the water pump to ensure system stability.

## **🛠️ Hardware Requirements**

* ESP32 Development Board  
* Analog Soil Moisture Sensor  
* Relay Module (Active-Low)  
* Mini DC Water Pump (5V or 12V depending on your setup)  
* Power supply for ESP32 (e.g., 3x AA battery pack / 4.5V)  
* Separate power supply for the water pump  
* Breadboard and jumper wires  
* Tubing for the pump

## **🔌 Wiring Guide**

| Component | Pin | ESP32 Connection |
| :---- | :---- | :---- |
| **Soil Sensor** | VCC | 3.3V |
|  | GND | GND |
|  | A0 (Data) | GPIO 34 |
| **Relay Module** | VCC | 3.3V (or 5V if required) |
|  | GND | GND |
|  | IN (Signal) | GPIO 5 |

*(Note: The water pump should be wired in series with the normally open (NO) terminals of the relay, powered by its own battery pack).*

## **💻 Software Setup**

1. Install the [Arduino IDE](https://www.arduino.cc/en/software) and add the ESP32 board package in the Board Manager.  
2. Install the ArduinoJson library (by Benoit Blanchon) via the Library Manager.  
3. Open ESP32\_ThingSpeak.ino.  
4. Update the configuration variables at the top of the file with your local network and API details:  
   const char\* ssid \= "YOUR\_WIFI\_NAME";  
   const char\* password \= "YOUR\_WIFI\_PASSWORD";  
   String apiKey \= "YOUR\_THINGSPEAK\_API\_KEY";  
   String openWeatherApiKey \= "YOUR\_OPENWEATHER\_API\_KEY";  
   String city \= "YourCity,CountryCode"; // e.g., "Nairobi,KE"

5. Upload the sketch to your ESP32.

## **⚙️ How It Works**

* The ESP32 fetches local weather data from OpenWeatherMap every 10 minutes.  
* It reads the analog soil moisture sensor and maps the raw value to a percentage (0-100%).  
* If the soil moisture is below the configured dryThreshold (default 30%) and the API does not report rain, the relay turns the pump on.  
* If the weather API reports Rain, Drizzle, or Thunderstorms, the pump remains off to save water, regardless of the soil moisture level.  
* The system pushes the updated sensor and API data to ThingSpeak.

## **🚀 Planned Improvements**

* Integrate a DHT11/DHT22 sensor to track local greenhouse temperature and humidity.  
* Implement Deep Sleep functionality to extend battery life.  
* Switch the ESP32 power source to a solar charging circuit.
