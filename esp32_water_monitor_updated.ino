#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include <HTTPClient.h>
#include <BlynkSimpleEsp32.h>
#include <time.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// 🔥 Blynk Template Setup
#define BLYNK_TEMPLATE_ID "TMPL41hL-G3VD"
#define BLYNK_TEMPLATE_NAME "Temperature"
#define BLYNK_AUTH_TOKEN "8VhXKKZgJdHXraJju_2bGKLy5TXTVdBY"

// 📶 WiFi credentials
const char* ssid = "Jerusha";
const char* password = "Sharon200";

// 🔥 Firebase details
const char* firebaseHost = "https://esp32temperaturewithhumidity-default-rtdb.asia-southeast1.firebasedatabase.app/";
const char* firebaseAuth = "xG1WjrOYCYXPzPFb1IfnKQJBhrio88DXYacrFp0s";

// 📏 Ultrasonic sensor pins
#define TRIG_PIN 5
#define ECHO_PIN 18

// 🔔 Buzzer pin
#define BUZZER_PIN 23

// 🧪 Turbidity sensor pin (analog)
#define TURBIDITY_AO 34

// 🌡 DS18B20 sensor pin
#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
float temperatureC = 0;

// 🌐 NTP config
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;  // UTC+5:30
const int daylightOffset_sec = 0;

// Web server for Android app
WebServer server(80);

float distance = 0;
int turbidityRaw = 0;
float turbidityVoltage = 0;
float ntu = 0;
String turbidityStatus = "Unknown";
String microbioStatus = "Unknown";

unsigned long lastBlynkUpdate = 0;
unsigned long lastFirebaseUpdate = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("-----------------------------------------------------");
  Serial.println("🧪 TURBIDITY SENSOR WATER QUALITY ANALYSIS REPORT 🧪");
  Serial.println("-----------------------------------------------------\n");

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  pinMode(TURBIDITY_AO, INPUT);

  sensors.begin();

  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  int retries = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (++retries > 40) {
      Serial.println("\nWiFi connection failed!");
      while (true);
    }
  }

  Serial.println("\nWiFi Connected Successfully!");

  // 🕒 NTP time sync
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Waiting for time synchronization...");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }

  Serial.println();
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.println(asctime(&timeinfo));

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, password);
  
  // Setup web server routes for Android app
  setupWebServer();
  server.begin();
  
  Serial.println("🚀 Distance, Turbidity, Temperature and Water Quality Monitor Started!");
  Serial.print("Web server IP: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  Blynk.run();
  server.handleClient();

  distance = measureDistance();
  readTurbidityAndAnalysis();
  readTemperatureSensor();

  if (distance > 0 && distance < 400) {
    Serial.print("📏 Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    // 🚨 Buzzer logic
    if (distance <= 10 || turbidityStatus == "🚨 Dirty Water" || turbidityStatus == "🚨 Very Dirty") {
      continuousBeep();
    } else if (distance >= 150) {
      beepBuzzerOnce();
    } else {
      digitalWrite(BUZZER_PIN, LOW);
    }

    // 📡 Send to Blynk
    if (millis() - lastBlynkUpdate >= 1000) {
      Blynk.virtualWrite(V0, distance);
      Blynk.virtualWrite(V1, turbidityVoltage);
      Blynk.virtualWrite(V2, turbidityStatus);
      Blynk.virtualWrite(V3, microbioStatus);
      Blynk.virtualWrite(V4, temperatureC);  // Temperature
      lastBlynkUpdate = millis();
    }

    // ☁ Send to Firebase
    if (millis() - lastFirebaseUpdate >= 2000) {
      sendToFirebase(distance, turbidityVoltage, turbidityStatus, microbioStatus, temperatureC);
      lastFirebaseUpdate = millis();
    }
  } else {
    Serial.println("❌ Failed to read from ultrasonic sensor!");
    digitalWrite(BUZZER_PIN, LOW);
  }

  delay(500);
}

void setupWebServer() {
  // API endpoint for Android app to get latest data
  server.on("/api/water-quality", HTTP_GET, []() {
    DynamicJsonDocument doc(1024);
    doc["distance"] = distance;
    doc["turbidity_voltage"] = turbidityVoltage;
    doc["ntu"] = ntu;
    doc["temperature"] = temperatureC;
    doc["status"] = turbidityStatus;
    doc["microbial_analysis"] = microbioStatus;
    doc["timestamp"] = getCurrentTime();
    doc["device_id"] = "ESP32_WATER_MONITOR";
    
    String response;
    serializeJson(doc, response);
    
    server.send(200, "application/json", response);
  });
  
  // API endpoint for device status
  server.on("/api/status", HTTP_GET, []() {
    DynamicJsonDocument doc(512);
    doc["status"] = "online";
    doc["device_id"] = "ESP32_WATER_MONITOR";
    doc["uptime"] = millis();
    doc["free_heap"] = ESP.getFreeHeap();
    doc["wifi_rssi"] = WiFi.RSSI();
    doc["distance"] = distance;
    doc["temperature"] = temperatureC;
    doc["turbidity_status"] = turbidityStatus;
    
    String response;
    serializeJson(doc, response);
    
    server.send(200, "application/json", response);
  });
  
  // API endpoint for historical data
  server.on("/api/water-quality/history", HTTP_GET, []() {
    DynamicJsonDocument doc(2048);
    JsonArray dataArray = doc.createNestedArray("data");
    
    // For simplicity, return current data 10 times
    // In a real implementation, you would store historical data
    for (int i = 0; i < 10; i++) {
      JsonObject data = dataArray.createNestedObject();
      data["distance"] = distance + random(-5, 5);
      data["turbidity_voltage"] = turbidityVoltage + random(-10, 10) / 100.0;
      data["ntu"] = ntu + random(-5, 5);
      data["temperature"] = temperatureC + random(-10, 10) / 100.0;
      data["status"] = turbidityStatus;
      data["microbial_analysis"] = microbioStatus;
      data["timestamp"] = getCurrentTime();
      data["device_id"] = "ESP32_WATER_MONITOR";
    }
    
    String response;
    serializeJson(doc, response);
    
    server.send(200, "application/json", response);
  });
  
  // Handle CORS
  server.enableCORS(true);
}

float measureDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return -1;

  float calculatedDistance = (duration * 0.0343) / 2;
  return round(calculatedDistance * 10.0) / 10.0;
}

void readTurbidityAndAnalysis() {
  turbidityRaw = analogRead(TURBIDITY_AO);
  turbidityVoltage = turbidityRaw * (3.3 / 4095.0);
  ntu = -1120.4 * sq(turbidityVoltage) + 5742.3 * turbidityVoltage - 4352.9;
  if (ntu < 0) ntu = 0;

  if (ntu <= 50) {
    turbidityStatus = "✅ Clean Water";
    microbioStatus = "Safe - No Organic/Microbial Traces";
  } else if (ntu > 50 && ntu <= 100) {
    turbidityStatus = "⚠ Cloudy Water";
    microbioStatus = "Mild Organic Presence";
  } else if (ntu > 100 && ntu <= 300) {
    turbidityStatus = "🚨 Dirty Water";
    microbioStatus = "Likely Microbial/Algae Bloom";
  } else {
    turbidityStatus = "🚨 Very Dirty";
    microbioStatus = "High Risk! Organic + Microbial";
  }

  Serial.print("🧪 Raw: ");
  Serial.print(turbidityRaw);
  Serial.print(" | Voltage: ");
  Serial.print(turbidityVoltage, 2);
  Serial.print(" V | NTU: ");
  Serial.print(ntu, 2);
  Serial.print(" | Status: ");
  Serial.println(turbidityStatus);
  Serial.print("🔬 Microbial/Organic Condition: ");
  Serial.println(microbioStatus);
}

void readTemperatureSensor() {
  sensors.requestTemperatures();
  temperatureC = sensors.getTempCByIndex(0);
  if (temperatureC == DEVICE_DISCONNECTED_C) {
    Serial.println("❌ DS18B20 not connected!");
    temperatureC = 0;
  } else {
    Serial.print("🌡 Temperature: ");
    Serial.print(temperatureC);
    Serial.println(" °C");
  }
}

void sendToFirebase(float dist, float turbVolt, String turbStatus, String microbio, float temp) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String timestamp = getCurrentTime();
    String jsonData = "{\"distance\":" + String(dist) +
                      ",\"turbidity_voltage\":" + String(turbVolt, 2) +
                      ",\"status\":\"" + turbStatus +
                      "\",\"microbial_analysis\":\"" + microbio +
                      "\",\"temperature\":" + String(temp, 2) +
                      ",\"timestamp\":\"" + timestamp + "\"}";

    String url = String(firebaseHost) + "/SensorData.json?auth=" + firebaseAuth;
    http.begin(url);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.PUT(jsonData);

    if (httpResponseCode > 0) {
      Serial.print("✅ Firebase Response: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("❌ Firebase Error: ");
      Serial.println(httpResponseCode);
    }

    http.end();
  }
}

String getCurrentTime() {
  time_t now = time(nullptr);
  struct tm timeinfo;
  localtime_r(&now, &timeinfo);

  char buffer[25];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(buffer);
}

void beepBuzzerOnce() {
  digitalWrite(BUZZER_PIN, HIGH);
  delay(100);
  digitalWrite(BUZZER_PIN, LOW);
}

void continuousBeep() {
  digitalWrite(BUZZER_PIN, HIGH);
} 