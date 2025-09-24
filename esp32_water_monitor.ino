#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>

// WiFi credentials
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Sensor pins
const int PH_PIN = 36;        // pH sensor analog pin
const int TEMP_PIN = 39;      // Temperature sensor analog pin
const int TURBIDITY_PIN = 34; // Turbidity sensor analog pin
const int DO_PIN = 35;        // Dissolved Oxygen sensor analog pin
const int CONDUCTIVITY_PIN = 32; // Conductivity sensor analog pin

// Calibration values
float phOffset = 0.00;
float tempOffset = 0.00;
float turbidityOffset = 0.00;
float doOffset = 0.00;
float conductivityOffset = 0.00;

WebServer server(80);

// Water quality data structure
struct WaterQualityData {
  float ph;
  float temperature;
  float turbidity;
  float dissolvedOxygen;
  float conductivity;
  unsigned long timestamp;
};

WaterQualityData currentData;

void setup() {
  Serial.begin(115200);
  
  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  // Setup server routes
  setupServerRoutes();
  
  // Start server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  
  // Read sensors every 30 seconds
  static unsigned long lastRead = 0;
  if (millis() - lastRead >= 30000) {
    readSensors();
    lastRead = millis();
  }
}

void setupServerRoutes() {
  // Get current water quality data
  server.on("/api/water-quality", HTTP_GET, []() {
    readSensors();
    
    DynamicJsonDocument doc(1024);
    doc["status"] = "success";
    doc["message"] = "Data retrieved successfully";
    
    JsonObject data = doc.createNestedObject("data");
    data["ph"] = currentData.ph;
    data["temperature"] = currentData.temperature;
    data["turbidity"] = currentData.turbidity;
    data["dissolved_oxygen"] = currentData.dissolvedOxygen;
    data["conductivity"] = currentData.conductivity;
    data["timestamp"] = currentData.timestamp;
    data["device_id"] = "ESP32_WATER_MONITOR";
    
    String response;
    serializeJson(doc, response);
    
    server.send(200, "application/json", response);
  });
  
  // Get device status
  server.on("/api/status", HTTP_GET, []() {
    DynamicJsonDocument doc(512);
    doc["status"] = "online";
    doc["device_id"] = "ESP32_WATER_MONITOR";
    doc["uptime"] = millis();
    doc["free_heap"] = ESP.getFreeHeap();
    doc["wifi_rssi"] = WiFi.RSSI();
    
    String response;
    serializeJson(doc, response);
    
    server.send(200, "application/json", response);
  });
  
  // Get historical data (simplified - returns last 10 readings)
  server.on("/api/water-quality/history", HTTP_GET, []() {
    DynamicJsonDocument doc(2048);
    JsonArray dataArray = doc.createNestedArray("data");
    
    // For simplicity, return current data 10 times
    // In a real implementation, you would store historical data
    for (int i = 0; i < 10; i++) {
      JsonObject data = dataArray.createNestedObject();
      data["ph"] = currentData.ph + random(-5, 5) / 100.0;
      data["temperature"] = currentData.temperature + random(-10, 10) / 100.0;
      data["turbidity"] = currentData.turbidity + random(-5, 5) / 100.0;
      data["dissolved_oxygen"] = currentData.dissolvedOxygen + random(-10, 10) / 100.0;
      data["conductivity"] = currentData.conductivity + random(-50, 50);
      data["timestamp"] = currentData.timestamp - (i * 30000); // 30 seconds apart
      data["device_id"] = "ESP32_WATER_MONITOR";
    }
    
    String response;
    serializeJson(doc, response);
    
    server.send(200, "application/json", response);
  });
  
  // Handle CORS
  server.enableCORS(true);
}

void readSensors() {
  // Read pH sensor
  int phRaw = analogRead(PH_PIN);
  currentData.ph = map(phRaw, 0, 4095, 0, 14) + phOffset;
  
  // Read temperature sensor (assuming DS18B20 or similar)
  int tempRaw = analogRead(TEMP_PIN);
  currentData.temperature = map(tempRaw, 0, 4095, 0, 50) + tempOffset;
  
  // Read turbidity sensor
  int turbidityRaw = analogRead(TURBIDITY_PIN);
  currentData.turbidity = map(turbidityRaw, 0, 4095, 0, 100) + turbidityOffset;
  
  // Read dissolved oxygen sensor
  int doRaw = analogRead(DO_PIN);
  currentData.dissolvedOxygen = map(doRaw, 0, 4095, 0, 20) + doOffset;
  
  // Read conductivity sensor
  int conductivityRaw = analogRead(CONDUCTIVITY_PIN);
  currentData.conductivity = map(conductivityRaw, 0, 4095, 0, 2000) + conductivityOffset;
  
  currentData.timestamp = millis();
  
  // Print to serial for debugging
  Serial.println("Water Quality Data:");
  Serial.print("pH: "); Serial.println(currentData.ph);
  Serial.print("Temperature: "); Serial.println(currentData.temperature);
  Serial.print("Turbidity: "); Serial.println(currentData.turbidity);
  Serial.print("Dissolved Oxygen: "); Serial.println(currentData.dissolvedOxygen);
  Serial.print("Conductivity: "); Serial.println(currentData.conductivity);
  Serial.println("------------------------");
}

// Helper function to map values
float map(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
} 