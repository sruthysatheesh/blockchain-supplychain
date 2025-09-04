#include <DHT.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>

// Sensor Pins
#define DHTPIN 2         // GPIO2 (D4)
#define RAIN_PIN 5       // Changed to GPIO5 (D1) - safer than GPIO0
#define MOISTURE_PIN A0  // Only analog pin
#define DHTTYPE DHT11    // DHT11 or DHT22

// WiFi Credentials
const char* ssid = "beep beep boop beep";
const char* password = "potatoslur";

// Google Script
const char* googleScriptURL = "https://script.google.com/macros/s/AKfycbxm1V-sNB2PiwhlPsaVZLIDE3BYkAHdkBbwIr3hiYi26FZ5TGtTnZRohnWmMmhgc1vK/exec";

// Calibration
const int dryValue = 620;
const int wetValue = 310;

// Global Variables
DHT dht(DHTPIN, DHTTYPE);
volatile int rainCount = 0;
unsigned long lastRainTime = 0;

void IRAM_ATTR countRain() {
  if (millis() - lastRainTime > 500) { // 500ms debounce
    rainCount++;
    lastRainTime = millis();
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  pinMode(MOISTURE_PIN, INPUT);
  pinMode(RAIN_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RAIN_PIN), countRain, FALLING);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
}

void loop() {
  // Read sensors
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  int soil = map(analogRead(MOISTURE_PIN), dryValue, wetValue, 0, 100);
  soil = constrain(soil, 0, 100);
  float rain = rainCount * 0.2794;

  // Print to Serial
  Serial.printf("Temp: %.1f°C | Hum: %.1f%% | Soil: %d%% | Rain: %.2fmm\n", 
               t, h, soil, rain);

  // Send to Google Sheets
  sendToGoogleSheets(t, h, soil, rain);

  // Reset rain counter hourly
  if (millis() % 3600000 < 2000) rainCount = 0;
  
  delay(3000); // 30s delay
}

void sendToGoogleSheets(float temp, float hum, int soil, float rain) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi Disconnected");
    WiFi.reconnect();
    delay(2000);
    return;
  }

  WiFiClientSecure client;
  client.setInsecure(); // Bypass SSL verification
  client.setTimeout(20); // 20 second timeout
  
  HTTPClient http;
  http.setReuse(false);
  http.setTimeout(20000); // 20 second timeout
  http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

  // Add cache buster to URL
  String url = String(googleScriptURL) + 
               "?temperature=" + String(temp,1) +
               "&humidity=" + String(hum,1) +
               "&soil=" + String(soil) +
               "&rain=" + String(rain,2) +
               "&t=" + millis(); // Cache buster

  Serial.println("Trying URL: " + url);

  if (http.begin(client, url)) {
    int httpCode = http.GET();
    
    if (httpCode > 0) {
      Serial.printf("HTTP Code: %d\n", httpCode);
      String payload = http.getString();
      Serial.println("Response: " + payload);
      
      // Check for HTML error page
      if (payload.indexOf("<html") >= 0) {
        Serial.println("Received HTML error page!");
        parseGoogleError(payload);
      }
    } else {
      Serial.printf("HTTP Error %d: %s\n", httpCode, http.errorToString(httpCode).c_str());
    }
    http.end();
  } else {
    Serial.println("Connection failed");
  }
}

void parseGoogleError(String html) {
  // Extract error message from Google's HTML response
  int start = html.indexOf("<div style=");
  if (start >= 0) {
    int end = html.indexOf("</div>", start);
    String error = html.substring(start, end);
    error.replace("<div style=\"text-align:center;font-family:monospace; margin:50px auto 0;max-width:600px\">", "");
    Serial.println("Google Error: " + error);
  }
}