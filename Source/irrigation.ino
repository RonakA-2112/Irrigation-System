#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h> 

//Define pins for components
#define MOISTURE_PIN 34 //soil moisture
#define RELAY_PIN 5     //pump relay

//Minimum Water Moisture level
const int dryThreshold = 85; // Changed 
unsigned long previousMillisWeather = 0;
unsigned long previousMillisPump = 0;
unsigned long previousData = 0;
unsigned long previousDataSend = 0;


const unsigned long intervalWeather = 600000;
const unsigned long intervalPump = 5000;
const unsigned long intervalData = 15000;
const unsigned long intervalSendData = 300000; // Send data every 5 minutes

int currentMillis;

int moisturePercent = 0;

//WiFi and ThingSpeak Credentials
const char* ssid = "YOUR_WIFI_NAME";  
const char* password = "YOUR_WIFI_PASSWORD";  
String apiKey = "YOUR_THINGSPEAK_API_KEY";  
String openWeatherApiKey = "YOUR_OPENWEATHER_API_KEY";  
String city = "YourCity,CountryCode";

float apiTemp = 0.0;
float apiHumidity = 0.0;
String apiCondition = "";

int Read_Soil_Moisture(){
  int rawMoisture = analogRead(MOISTURE_PIN);
    
  Serial.println("Raw Moisture: ");
  Serial.println(rawMoisture);
  int Percent = map(rawMoisture, 4100, 1000, 0, 100);
  Percent = constrain(Percent, 0, 100);
    
  Serial.print("Soil Moisture: ");
  Serial.print(Percent);
  Serial.println("%");

  return Percent;
}

void setup() {
  Serial.begin(115200);

  //Setup relay pin as an output and make sure the pump is OFF in the start
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);

  //Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  
  //Wait until connected
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nSuccessfully connected to WiFi!");
  Serial.print("ESP32 IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Check if WiFi is still connected
  if(WiFi.status() == WL_CONNECTED){

    // Read soil moisture sensor
    moisturePercent = Read_Soil_Moisture();

    // Only fetch every 10 minutes to avoid hitting API rate limits
    if ((millis() - previousMillisWeather) > intervalWeather || previousMillisWeather == 0) {
      String weatherUrl = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "&appid=" + openWeatherApiKey + "&units=metric";
      HTTPClient weatherHttp;
      weatherHttp.begin(weatherUrl.c_str());
      int httpCode = weatherHttp.GET();

      if (httpCode > 0) {
        String payload = weatherHttp.getString();
        
        JsonDocument doc; 
        DeserializationError error = deserializeJson(doc, payload);

        if (!error) {
          apiTemp = doc["main"]["temp"];
          apiHumidity = doc["main"]["humidity"];
          apiCondition = doc["weather"][0]["main"].as<String>();
          
          Serial.println("\n--- Weather API Update ---");
          Serial.print("Outside Temp: "); Serial.print(apiTemp); Serial.println("C");
          Serial.print("Outside Condition: "); Serial.println(apiCondition);
          Serial.println("--------------------------\n");
          
          previousMillisWeather = millis();
        } else {
          Serial.println("Failed to read weather data JSON.");
        }
      }
      weatherHttp.end();
    }

    // --- Compare & Control Pump ---
    int currentMillis = millis();
    Serial.print("API Temp (Outside): "); Serial.print(apiTemp); Serial.println("C");

    // Logic to control the water pump
    if (apiCondition == "Rain" || apiCondition == "Drizzle" || apiCondition == "Thunderstorm") {
      Serial.println("It's raining outside! Skipping the pump to save water.");
      digitalWrite(RELAY_PIN, HIGH);
    }else if (moisturePercent < dryThreshold) {
      Serial.println("Soil is dry and clear skies! Turning PUMP ON.");
      digitalWrite(RELAY_PIN, LOW);
      
      unsigned long pumpStartTime = millis();
      
      while (true) {
        moisturePercent = Read_Soil_Moisture();
        
        if(moisturePercent >= dryThreshold || (millis() - pumpStartTime) >= intervalPump){
          digitalWrite(RELAY_PIN, HIGH); // Turn pump OFF
          Serial.println("Pump cycle finished.");
          break;
        }
      
        delay(100);
      }
    } else {
      Serial.println("Soil is good. Turning PUMP OFF.");
      digitalWrite(RELAY_PIN, HIGH);
    }

    // Build the URL for ThingSpeak
    if ((millis() - previousDataSend) >= intervalSendData){
      String serverPath = serverName + "?api_key=" + apiKey + "&field1=" + String(moisturePercent) + "&field2=" + String(apiTemp);
    
      // Start the HTTP client
      HTTPClient http;
      http.begin(serverPath.c_str());
      
      // Send the GET request
      int httpResponseCode = http.GET();
      
      if (httpResponseCode > 0) {
        Serial.print("Data sent to ThingSpeak! HTTP Response code: ");
        Serial.println(httpResponseCode);
      } else {
        Serial.print("Error sending data. Code: ");
        Serial.println(httpResponseCode);
      }
      
      http.end();
      previousDataSend = millis();
    }
    
  } else {
    Serial.println("WiFi Disconnected. Reconnecting...");
    WiFi.reconnect();
  }
  
  // Wait 15 seconds before taking the next reading
  delay(15000); 
}