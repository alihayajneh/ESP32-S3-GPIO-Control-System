#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>

// ================================
// CONFIGURATION
// ================================
const char* WIFI_SSID = "***";
const char* WIFI_PASSWORD = "****";
const char* DEVICE_NAME = "ESP32-S3-API";
const char* VERSION = "2.0.0";

// ================================
// GLOBAL VARIABLES
// ================================
WebServer server(80);
unsigned long bootTime = 0;
unsigned long requestCount = 0;

// ESP32-S3 GPIO Configuration
const int USABLE_GPIOS[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48};
const int ANALOG_PINS[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20};
const int PROTECTED_PINS[] = {19, 20, 43, 44}; // USB and UART pins

// ================================
// CORS HELPER FUNCTIONS
// ================================
void setCORSHeaders() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS, HEAD");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type, Authorization, X-Requested-With, Accept, Origin");
  server.sendHeader("Access-Control-Max-Age", "86400");
  server.sendHeader("Cache-Control", "no-cache");
}

void handleOptionsRequest() {
  Serial.println("CORS Preflight: " + server.uri());
  setCORSHeaders();
  server.send(200, "text/plain", "OK");
}

void sendJSON(int code, DynamicJsonDocument& doc) {
  setCORSHeaders();
  server.sendHeader("Content-Type", "application/json");
  
  String response;
  serializeJson(doc, response);
  server.send(code, "application/json", response);
}

void sendError(int code, String message) {
  setCORSHeaders();
  server.sendHeader("Content-Type", "application/json");
  
  DynamicJsonDocument doc(256);
  doc["error"] = message;
  doc["success"] = false;
  doc["timestamp"] = millis();
  
  String response;
  serializeJson(doc, response);
  server.send(code, "application/json", response);
  
  Serial.println("Error " + String(code) + ": " + message);
}

// ================================
// UTILITY FUNCTIONS
// ================================
bool isProtectedPin(int pin) {
  for (int i = 0; i < sizeof(PROTECTED_PINS) / sizeof(PROTECTED_PINS[0]); i++) {
    if (PROTECTED_PINS[i] == pin) return true;
  }
  return false;
}

String formatTime(unsigned long seconds) {
  unsigned long hours = seconds / 3600;
  unsigned long minutes = (seconds % 3600) / 60;
  unsigned long secs = seconds % 60;
  return String(hours) + "h " + String(minutes) + "m " + String(secs) + "s";
}

String formatBytes(size_t bytes) {
  if (bytes < 1024) return String(bytes) + " B";
  else if (bytes < 1048576) return String(bytes / 1024.0, 1) + " KB";
  else return String(bytes / 1048576.0, 1) + " MB";
}

String getSignalStrength(int rssi) {
  if (rssi > -50) return "Excellent";
  if (rssi > -60) return "Good";
  if (rssi > -70) return "Fair";
  if (rssi > -80) return "Weak";
  return "Very Weak";
}

// ================================
// API HANDLERS
// ================================
void handleRoot() {
  requestCount++;
  Serial.println("GET /");
  
  DynamicJsonDocument doc(1024);
  doc["device"] = DEVICE_NAME;
  doc["version"] = VERSION;
  doc["status"] = "running";
  doc["cors_enabled"] = true;
  doc["ip_address"] = WiFi.localIP().toString();
  doc["uptime_seconds"] = (millis() - bootTime) / 1000;
  doc["requests_served"] = requestCount;
  
  JsonArray endpoints = doc.createNestedArray("endpoints");
  endpoints.add("GET / - Device info");
  endpoints.add("GET /api/status - System status");
  endpoints.add("GET /api/info - Hardware info");
  endpoints.add("GET /api/wifi - WiFi info");
  endpoints.add("GET /api/memory - Memory usage");
  endpoints.add("GET /api/gpio - GPIO status");
  endpoints.add("POST /api/gpio - Control GPIO");
  endpoints.add("POST /api/pwm - PWM control");
  endpoints.add("POST /api/bulk - Bulk GPIO control");
  endpoints.add("GET /api/sensors - Sensor readings");
  endpoints.add("POST /api/restart - Restart device");
  
  sendJSON(200, doc);
}

void handleStatus() {
  requestCount++;
  Serial.println("GET /api/status");
  
  DynamicJsonDocument doc(1024);
  doc["device"] = DEVICE_NAME;
  doc["version"] = VERSION;
  doc["status"] = "running";
  doc["uptime_ms"] = millis() - bootTime;
  doc["uptime_seconds"] = (millis() - bootTime) / 1000;
  doc["uptime_formatted"] = formatTime((millis() - bootTime) / 1000);
  doc["free_heap"] = ESP.getFreeHeap();
  doc["total_heap"] = ESP.getHeapSize();
  doc["heap_usage_percent"] = ((ESP.getHeapSize() - ESP.getFreeHeap()) * 100.0) / ESP.getHeapSize();
  doc["cpu_freq_mhz"] = ESP.getCpuFreqMHz();
  doc["requests_served"] = requestCount;
  doc["wifi_connected"] = WiFi.status() == WL_CONNECTED;
  doc["cors_working"] = true;
  
  sendJSON(200, doc);
}

void handleDeviceInfo() {
  requestCount++;
  Serial.println("GET /api/info");
  
  DynamicJsonDocument doc(1024);
  doc["device_name"] = DEVICE_NAME;
  doc["chip_model"] = ESP.getChipModel();
  doc["chip_revision"] = ESP.getChipRevision();
  doc["chip_cores"] = ESP.getChipCores();
  doc["cpu_freq_mhz"] = ESP.getCpuFreqMHz();
  doc["flash_size"] = ESP.getFlashChipSize();
  doc["flash_speed"] = ESP.getFlashChipSpeed();
  doc["sketch_size"] = ESP.getSketchSize();
  doc["free_sketch_space"] = ESP.getFreeSketchSpace();
  doc["sdk_version"] = ESP.getSdkVersion();
  doc["mac_address"] = WiFi.macAddress();
  
  sendJSON(200, doc);
}

void handleWiFiInfo() {
  requestCount++;
  Serial.println("GET /api/wifi");
  
  DynamicJsonDocument doc(512);
  doc["ssid"] = WiFi.SSID();
  doc["ip_address"] = WiFi.localIP().toString();
  doc["gateway"] = WiFi.gatewayIP().toString();
  doc["subnet_mask"] = WiFi.subnetMask().toString();
  doc["dns"] = WiFi.dnsIP().toString();
  doc["rssi"] = WiFi.RSSI();
  doc["signal_strength"] = getSignalStrength(WiFi.RSSI());
  doc["mac_address"] = WiFi.macAddress();
  doc["channel"] = WiFi.channel();
  doc["connected"] = WiFi.status() == WL_CONNECTED;
  
  sendJSON(200, doc);
}

void handleMemoryInfo() {
  requestCount++;
  Serial.println("GET /api/memory");
  
  DynamicJsonDocument doc(512);
  doc["total_heap"] = ESP.getHeapSize();
  doc["free_heap"] = ESP.getFreeHeap();
  doc["used_heap"] = ESP.getHeapSize() - ESP.getFreeHeap();
  doc["heap_usage_percent"] = ((ESP.getHeapSize() - ESP.getFreeHeap()) * 100.0) / ESP.getHeapSize();
  doc["largest_free_block"] = ESP.getMaxAllocHeap();
  doc["min_free_heap"] = ESP.getMinFreeHeap();
  doc["psram_size"] = ESP.getPsramSize();
  doc["free_psram"] = ESP.getFreePsram();
  
  // Format sizes
  doc["total_heap_formatted"] = formatBytes(ESP.getHeapSize());
  doc["free_heap_formatted"] = formatBytes(ESP.getFreeHeap());
  doc["psram_size_formatted"] = formatBytes(ESP.getPsramSize());
  
  sendJSON(200, doc);
}

void handleGPIOStatus() {
  requestCount++;
  Serial.println("GET /api/gpio");
  
  DynamicJsonDocument doc(4096); // Increased size for GPIO data
  
  // GPIO pins array
  JsonArray gpios = doc.createNestedArray("gpio_pins");
  int gpioCount = sizeof(USABLE_GPIOS) / sizeof(USABLE_GPIOS[0]);
  
  for (int i = 0; i < gpioCount; i++) {
    int pin = USABLE_GPIOS[i];
    JsonObject gpio = gpios.createNestedObject();
    gpio["pin"] = pin;
    gpio["digital_value"] = digitalRead(pin);
    gpio["is_protected"] = isProtectedPin(pin);
    
    // Pin function description
    if (pin == 0) gpio["function"] = "Boot Button";
    else if (pin == 43 || pin == 44) gpio["function"] = "UART0 (USB)";
    else if (pin == 19 || pin == 20) gpio["function"] = "USB D+/D-";
    else if (pin >= 26 && pin <= 32) gpio["function"] = "SPI Flash";
    else gpio["function"] = "General Purpose";
  }
  
  // Analog pins array
  JsonArray analogs = doc.createNestedArray("analog_pins");
  int analogCount = sizeof(ANALOG_PINS) / sizeof(ANALOG_PINS[0]);
  
  for (int i = 0; i < analogCount; i++) {
    int pin = ANALOG_PINS[i];
    JsonObject analog = analogs.createNestedObject();
    analog["pin"] = pin;
    analog["raw_value"] = analogRead(pin);
    analog["voltage"] = (analogRead(pin) * 3.3) / 4095.0;
    analog["percentage"] = (analogRead(pin) * 100.0) / 4095.0;
  }
  
  doc["total_gpio_pins"] = gpioCount;
  doc["total_analog_pins"] = analogCount;
  doc["timestamp"] = millis();
  
  sendJSON(200, doc);
}

void handleGPIOControl() {
  requestCount++;
  Serial.println("POST /api/gpio");
  
  String body = server.arg("plain");
  Serial.println("Request body: " + body);
  
  if (body.length() == 0) {
    sendError(400, "Empty request body");
    return;
  }
  
  DynamicJsonDocument request(512);
  DeserializationError error = deserializeJson(request, body);
  
  if (error) {
    sendError(400, "Invalid JSON: " + String(error.c_str()));
    return;
  }
  
  if (!request.containsKey("pin")) {
    sendError(400, "Missing 'pin' parameter");
    return;
  }
  
  int pin = request["pin"];
  
  if (isProtectedPin(pin)) {
    sendError(403, "Pin " + String(pin) + " is protected and cannot be controlled");
    return;
  }
  
  DynamicJsonDocument response(512);
  response["success"] = true;
  response["pin"] = pin;
  
  if (request.containsKey("state")) {
    // Digital control
    bool state = request["state"];
    pinMode(pin, OUTPUT);
    digitalWrite(pin, state);
    
    response["action"] = "digital_write";
    response["state"] = state;
    response["message"] = "GPIO " + String(pin) + " set to " + (state ? "HIGH" : "LOW");
    
    Serial.println("GPIO " + String(pin) + " -> " + (state ? "HIGH" : "LOW"));
    
  } else if (request.containsKey("mode")) {
    // Pin mode control
    String mode = request["mode"];
    
    if (mode == "INPUT") {
      pinMode(pin, INPUT);
    } else if (mode == "INPUT_PULLUP") {
      pinMode(pin, INPUT_PULLUP);
    } else if (mode == "OUTPUT") {
      pinMode(pin, OUTPUT);
    } else {
      sendError(400, "Invalid mode. Use INPUT, INPUT_PULLUP, or OUTPUT");
      return;
    }
    
    response["action"] = "set_mode";
    response["mode"] = mode;
    response["message"] = "GPIO " + String(pin) + " mode set to " + mode;
    
    Serial.println("GPIO " + String(pin) + " mode -> " + mode);
    
  } else {
    sendError(400, "Missing 'state' or 'mode' parameter");
    return;
  }
  
  sendJSON(200, response);
}

void handlePWMControl() {
  requestCount++;
  Serial.println("POST /api/pwm");
  
  String body = server.arg("plain");
  
  if (body.length() == 0) {
    sendError(400, "Empty request body");
    return;
  }
  
  DynamicJsonDocument request(512);
  DeserializationError error = deserializeJson(request, body);
  
  if (error) {
    sendError(400, "Invalid JSON: " + String(error.c_str()));
    return;
  }
  
  if (!request.containsKey("pin") || !request.containsKey("value")) {
    sendError(400, "Missing 'pin' or 'value' parameter");
    return;
  }
  
  int pin = request["pin"];
  int value = request["value"];
  int frequency = request.containsKey("frequency") ? request["frequency"].as<int>() : 1000;
  int channel = request.containsKey("channel") ? request["channel"].as<int>() : 0;
  
  if (isProtectedPin(pin)) {
    sendError(403, "Pin " + String(pin) + " is protected");
    return;
  }
  
  if (value < 0 || value > 255) {
    sendError(400, "PWM value must be between 0 and 255");
    return;
  }
  
  if (channel < 0 || channel > 15) {
    sendError(400, "PWM channel must be between 0 and 15");
    return;
  }
  
  // Configure and set PWM
  ledcSetup(channel, frequency, 8); // 8-bit resolution
  ledcAttachPin(pin, channel);
  ledcWrite(channel, value);
  
  DynamicJsonDocument response(512);
  response["success"] = true;
  response["pin"] = pin;
  response["pwm_value"] = value;
  response["frequency"] = frequency;
  response["channel"] = channel;
  response["duty_cycle_percent"] = (value * 100.0) / 255.0;
  response["message"] = "PWM set on GPIO " + String(pin);
  
  Serial.println("PWM GPIO " + String(pin) + " -> " + String(value) + " (" + String((value * 100) / 255) + "%)");
  
  sendJSON(200, response);
}

void handleBulkControl() {
  requestCount++;
  Serial.println("POST /api/bulk");
  
  String body = server.arg("plain");
  
  if (body.length() == 0) {
    sendError(400, "Empty request body");
    return;
  }
  
  DynamicJsonDocument request(2048);
  DeserializationError error = deserializeJson(request, body);
  
  if (error) {
    sendError(400, "Invalid JSON: " + String(error.c_str()));
    return;
  }
  
  if (!request.containsKey("pins")) {
    sendError(400, "Missing 'pins' array");
    return;
  }
  
  JsonArray pins = request["pins"];
  
  DynamicJsonDocument response(2048);
  response["success"] = true;
  JsonArray results = response.createNestedArray("results");
  
  int successCount = 0;
  int errorCount = 0;
  
  for (JsonVariant pinData : pins) {
    JsonObject result = results.createNestedObject();
    
    if (!pinData.containsKey("pin")) {
      result["error"] = "Missing pin number";
      errorCount++;
      continue;
    }
    
    int pin = pinData["pin"];
    result["pin"] = pin;
    
    if (isProtectedPin(pin)) {
      result["success"] = false;
      result["error"] = "Protected pin";
      errorCount++;
      continue;
    }
    
    if (pinData.containsKey("state")) {
      bool state = pinData["state"];
      pinMode(pin, OUTPUT);
      digitalWrite(pin, state);
      result["success"] = true;
      result["action"] = "digital_write";
      result["state"] = state;
      successCount++;
      
    } else if (pinData.containsKey("mode")) {
      String mode = pinData["mode"];
      if (mode == "INPUT") pinMode(pin, INPUT);
      else if (mode == "INPUT_PULLUP") pinMode(pin, INPUT_PULLUP);
      else if (mode == "OUTPUT") pinMode(pin, OUTPUT);
      else {
        result["success"] = false;
        result["error"] = "Invalid mode";
        errorCount++;
        continue;
      }
      result["success"] = true;
      result["action"] = "set_mode";
      result["mode"] = mode;
      successCount++;
      
    } else {
      result["success"] = false;
      result["error"] = "No valid action specified";
      errorCount++;
    }
  }
  
  response["summary"]["total"] = pins.size();
  response["summary"]["successful"] = successCount;
  response["summary"]["errors"] = errorCount;
  response["message"] = "Bulk operation completed: " + String(successCount) + " success, " + String(errorCount) + " errors";
  
  Serial.println("Bulk operation: " + String(successCount) + "/" + String(pins.size()) + " successful");
  
  sendJSON(200, response);
}

void handleSensors() {
  requestCount++;
  Serial.println("GET /api/sensors");
  
  DynamicJsonDocument doc(2048);
  
  // System sensors
  doc["internal_temperature_f"] = temperatureRead();
  doc["internal_temperature_c"] = (temperatureRead() - 32) * 5.0 / 9.0;
  doc["uptime_seconds"] = (millis() - bootTime) / 1000;
  doc["free_heap"] = ESP.getFreeHeap();
  
  // Analog readings
  JsonArray analogs = doc.createNestedArray("analog_readings");
  int analogCount = sizeof(ANALOG_PINS) / sizeof(ANALOG_PINS[0]);
  
  for (int i = 0; i < analogCount; i++) {
    int pin = ANALOG_PINS[i];
    JsonObject reading = analogs.createNestedObject();
    reading["pin"] = pin;
    reading["raw_value"] = analogRead(pin);
    reading["voltage"] = (analogRead(pin) * 3.3) / 4095.0;
    reading["percentage"] = (analogRead(pin) * 100.0) / 4095.0;
  }
  
  // Random test value for demonstration
  doc["random_demo_value"] = random(0, 1024);
  doc["timestamp"] = millis();
  
  sendJSON(200, doc);
}

void handleRestart() {
  requestCount++;
  Serial.println("POST /api/restart");
  
  DynamicJsonDocument doc(256);
  doc["success"] = true;
  doc["message"] = "Device will restart in 3 seconds";
  doc["restart_time"] = millis() + 3000;
  
  sendJSON(200, doc);
  
  Serial.println("Restart requested - restarting in 3 seconds...");
  delay(3000);
  ESP.restart();
}

void handleNotFound() {
  requestCount++;
  Serial.println("404: " + server.uri());
  
  DynamicJsonDocument doc(512);
  doc["error"] = "Endpoint not found";
  doc["path"] = server.uri();
  doc["method"] = server.method() == HTTP_GET ? "GET" : 
                 server.method() == HTTP_POST ? "POST" : 
                 server.method() == HTTP_OPTIONS ? "OPTIONS" : "OTHER";
  doc["available_endpoints"] = "Visit / for complete API documentation";
  doc["cors_enabled"] = true;
  
  sendJSON(404, doc);
}

// ================================
// SETUP ROUTES
// ================================
void setupRoutes() {
  // Universal OPTIONS handler for CORS preflight
  server.onNotFound([]() {
    if (server.method() == HTTP_OPTIONS) {
      handleOptionsRequest();
    } else {
      handleNotFound();
    }
  });
  
  // Main API routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/", HTTP_OPTIONS, handleOptionsRequest);
  
  server.on("/api/status", HTTP_GET, handleStatus);
  server.on("/api/status", HTTP_OPTIONS, handleOptionsRequest);
  
  server.on("/api/info", HTTP_GET, handleDeviceInfo);
  server.on("/api/info", HTTP_OPTIONS, handleOptionsRequest);
  
  server.on("/api/wifi", HTTP_GET, handleWiFiInfo);
  server.on("/api/wifi", HTTP_OPTIONS, handleOptionsRequest);
  
  server.on("/api/memory", HTTP_GET, handleMemoryInfo);
  server.on("/api/memory", HTTP_OPTIONS, handleOptionsRequest);
  
  server.on("/api/gpio", HTTP_GET, handleGPIOStatus);
  server.on("/api/gpio", HTTP_POST, handleGPIOControl);
  server.on("/api/gpio", HTTP_OPTIONS, handleOptionsRequest);
  
  server.on("/api/pwm", HTTP_POST, handlePWMControl);
  server.on("/api/pwm", HTTP_OPTIONS, handleOptionsRequest);
  
  server.on("/api/bulk", HTTP_POST, handleBulkControl);
  server.on("/api/bulk", HTTP_OPTIONS, handleOptionsRequest);
  
  server.on("/api/sensors", HTTP_GET, handleSensors);
  server.on("/api/sensors", HTTP_OPTIONS, handleOptionsRequest);
  
  server.on("/api/restart", HTTP_POST, handleRestart);
  server.on("/api/restart", HTTP_OPTIONS, handleOptionsRequest);
}

// ================================
// MAIN SETUP AND LOOP
// ================================
void setup() {
  Serial.begin(115200);
  delay(2000);
  
  bootTime = millis();
  
  Serial.println();
  Serial.println("================================");
  Serial.println("ESP32-S3 API Server v" + String(VERSION));
  Serial.println("================================");
  
  // Initialize SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("⚠️  SPIFFS mount failed");
  } else {
    Serial.println("✅ SPIFFS initialized");
  }
  
  // Connect to WiFi
  Serial.println("🔗 Connecting to WiFi: " + String(WIFI_SSID));
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("✅ WiFi connected successfully!");
    Serial.println("📍 IP Address: " + WiFi.localIP().toString());
    Serial.println("📶 Signal: " + String(WiFi.RSSI()) + " dBm (" + getSignalStrength(WiFi.RSSI()) + ")");
    Serial.println("🏠 Gateway: " + WiFi.gatewayIP().toString());
    Serial.println("🆔 MAC: " + WiFi.macAddress());
  } else {
    Serial.println();
    Serial.println("❌ WiFi connection failed!");
    Serial.println("⚠️  Starting in AP mode...");
    WiFi.softAP("ESP32-S3-Setup", "12345678");
    Serial.println("📍 AP IP: " + WiFi.softAPIP().toString());
  }
  
  // Setup routes and start server
  setupRoutes();
  server.begin();
  
  Serial.println("================================");
  Serial.println("🚀 API Server started on port 80");
  Serial.println("🌐 CORS fully enabled");
  Serial.println("📋 Available endpoints:");
  Serial.println("   GET  /              - Device info");
  Serial.println("   GET  /api/status    - System status");
  Serial.println("   GET  /api/info      - Hardware info");
  Serial.println("   GET  /api/wifi      - WiFi details");
  Serial.println("   GET  /api/memory    - Memory usage");
  Serial.println("   GET  /api/gpio      - GPIO status");
  Serial.println("   POST /api/gpio      - Control GPIO");
  Serial.println("   POST /api/pwm       - PWM control");
  Serial.println("   POST /api/bulk      - Bulk operations");
  Serial.println("   GET  /api/sensors   - Sensor readings");
  Serial.println("   POST /api/restart   - Restart device");
  Serial.println("================================");
  Serial.println("✅ Ready for connections!");
  Serial.println();
}

void loop() {
  server.handleClient();
  delay(2);
}
