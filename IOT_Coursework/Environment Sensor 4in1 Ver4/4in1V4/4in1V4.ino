/*
 * Project: Environmental Sensor Data Collection & Upload System
 * Hardware: LoRa32 + DHT11 digital temperature and humidity sensor + Light Dependent Resistor Module 
 *           + HC-SR501 PIR Motion Sensor + DS1307 Real-Time Clock Module
 * Function: Collect environmental data every minute, cache locally, upload to Google Sheets
 */

// Library
#include <Wire.h>              
#include "RTClib.h"            
#include "DHT.h"               
#include <WiFi.h>              
#include <WiFiClientSecure.h>  
#include <HTTPClient.h>        
#include "SPIFFS.h"            

// WiFi
// WiFi network credentials (SSID and password)
const char* ssid = "太阳当空照";
const char* password = "hhhhhhhhhh";
// Google Apps Script Web App deployment URL (data receiving endpoint)
const char* serverName = "https://script.google.com/macros/s/AKfycbzYY3cWd7HkfGTWGH9yr_PGeMhzD_M8_uQZUf4EfNhMmfQYUR2pY-FhreU5irf6nP4iJg/exec";

// DHT
#define DHTPIN 34              
#define DHTTYPE DHT11          
DHT dht(DHTPIN, DHTTYPE);      

// LDR 
#define LDRPIN 1              

// PIR 
#define PIRPIN 26             

// Real-Time Clock Object
RTC_DS1307 rtc;                

// Timer Variables
unsigned long lastRead = 0;    
const unsigned long INTERVAL = 60000;  // Data collection interval: 60 seconds (1 minute)

// WiFi Connection Function
void connectWiFi() {

  if (WiFi.status() == WL_CONNECTED) return;

  Serial.println("Resetting WiFi...");

  WiFi.disconnect(true);     
  delay(1000);

  WiFi.mode(WIFI_STA);       
  WiFi.begin(ssid, password);

  unsigned long startAttempt = millis();

  while (WiFi.status() != WL_CONNECTED &&
         millis() - startAttempt < 15000) {

    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi Failed.");
  }
}

// Local Storage Function
void saveToLocal(String jsonData) {

  File file = SPIFFS.open("/data.txt", FILE_APPEND);
  if (!file) {
    Serial.println("File open failed!");
    return;
  }

  file.println(jsonData);
  file.close();
  Serial.println("Saved locally.");
}

// Upload Cache Function
void uploadStoredData() {

  if (WiFi.status() != WL_CONNECTED) return;

  File file = SPIFFS.open("/data.txt");
  if (!file) {
    Serial.println("No stored data.");
    return;
  }

  Serial.println("Uploading stored data...");

  WiFiClientSecure client;
  client.setInsecure();

  while (file.available()) {

    String line = file.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) continue;

    HTTPClient http;

    http.begin(client, serverName);
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);  // ⭐关键
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(line);

    Serial.print("Upload response: ");
    Serial.println(httpResponseCode);

    http.end();
    delay(800);

    if (httpResponseCode < 200 || httpResponseCode >= 300) {
      Serial.println("Upload failed, keep cache.");
      file.close();
      return;
    }
  }

  file.close();
  SPIFFS.remove("/data.txt");
  Serial.println("All data uploaded and cache cleared.");
}

// Setup
void setup() {

  Serial.begin(115200);
  delay(1000);

  Wire.begin(36, 35);

  if (!rtc.begin()) {
    Serial.println("RTC not found!");
  }

  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  dht.begin();

  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  pinMode(PIRPIN, INPUT);

  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
  }

  connectWiFi();
}

// Main Loop
void loop() {

  if (millis() - lastRead >= INTERVAL) {

    lastRead = millis();

    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
    int lightValue = analogRead(LDRPIN);
    float lightPercent = lightValue * 100.0 / 4095.0;

    int pirState = digitalRead(PIRPIN);

    // PIR Motion Detection Logic
    int pirCounter = 0;
    unsigned long startTime = millis();
    const unsigned long CHECK_DURATION = 5000;

    while (millis() - startTime < CHECK_DURATION) {
      if (digitalRead(PIRPIN) == HIGH) {
        pirCounter++;
        delay(200);
      }
      delay(50);
    }

    bool pirDetected = (pirCounter >= 3);

    DateTime now = rtc.now();
    char timestamp[25];
    sprintf(timestamp, "%04d/%02d/%02d %02d:%02d:%02d",
            now.year(), now.month(), now.day(),
            now.hour(), now.minute(), now.second());

    // Construct JSON Data Packet
    String jsonData = "{";
    jsonData += "\"timestamp\":\"" + String(timestamp) + "\",";
    jsonData += "\"temperature\":" + String(temperature) + ",";
    jsonData += "\"humidity\":" + String(humidity) + ",";
    jsonData += "\"lightPercent\":" + String(lightPercent) + ",";
    jsonData += "\"pirState\":" + String(pirState) + ",";
    jsonData += "\"pirDetected\":" + String(pirDetected ? 1 : 0);
    jsonData += "}";

    Serial.println("New Data:");
    Serial.println(jsonData);

    // Save to local first (ensure no data loss)
    saveToLocal(jsonData);

    // Attempt WiFi connection (if not connected)
    connectWiFi();

    // Upload all cached data (including just saved)
    uploadStoredData();
  }
}