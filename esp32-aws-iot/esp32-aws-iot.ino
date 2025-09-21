/*
 * ESP32 AWS IoT Core Client - Simplified Version
 * Based on the working sample code
 */

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include "wifi_config.h"
#include "aws_config.h"
#include "device_config.h"

// WiFi and MQTT clients
WiFiClientSecure net = WiFiClientSecure();
PubSubClient client(net);

// Status variables
bool wifiConnected = false;
bool mqttConnected = false;

// Task timers (non-blocking)
unsigned long lastMQTTCheck = 0;
unsigned long lastDataPublish = 0;
unsigned long lastLEDUpdate = 0;
unsigned long lastHeartbeat = 0;

void setup()
{
  Serial.begin(115200);
  delay(1000);

  // Setup LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Setup sensors
  setupSensors();

  Serial.println("ESP32C3 AWS IoT Core Client - Simplified");

  // Connect to WiFi
  connectToWiFi();

  // Connect to AWS IoT
  connectToAWS();

  Serial.println("Setup complete!");
}

void loop()
{
  unsigned long currentTime = millis();

  // Handle MQTT messages (highest priority)
  client.loop();

  // Modular task execution (non-blocking) - with enable switches
  if (ENABLE_CONNECTION_MONITORING && currentTime - lastMQTTCheck > MQTT_CHECK_INTERVAL)
  {
    checkConnectionStatus();
    lastMQTTCheck = currentTime;
  }

  if (currentTime - lastDataPublish > DATA_PUBLISH_INTERVAL)
  {
    publishDeviceData();
    lastDataPublish = currentTime;
  }

  if (currentTime - lastLEDUpdate > LED_UPDATE_INTERVAL)
  {
    updateLEDStatus();
    lastLEDUpdate = currentTime;
  }

  if (ENABLE_HEARTBEAT_MESSAGES && currentTime - lastHeartbeat > HEARTBEAT_INTERVAL)
  {
    publishHeartbeat();
    lastHeartbeat = currentTime;
  }

  // Publish status (configurable interval)
  static unsigned long lastStatusPublish = 0;
  if (ENABLE_STATUS_MESSAGES && currentTime - lastStatusPublish > STATUS_PUBLISH_INTERVAL)
  {
    publishDeviceStatus();
    lastStatusPublish = currentTime;
  }

  // DNS check (once at startup, then only on connection issues)
  static bool dnsChecked = false;
  static unsigned long lastDNSCheck = 0;

  if (ENABLE_DNS_CHECKING)
  {
    if (!dnsChecked && wifiConnected)
    {
      checkDNSResolution();
      dnsChecked = true;
      lastDNSCheck = currentTime;
    }
    else if (!mqttConnected && (currentTime - lastDNSCheck > DNS_CHECK_INTERVAL))
    {
      // Re-check DNS if MQTT disconnected (configurable interval)
      checkDNSResolution();
      lastDNSCheck = currentTime;
    }
  }

  // Certificate validation (configurable interval)
  static unsigned long lastCertValidation = 0;
  if (ENABLE_CERTIFICATE_VALIDATION && currentTime - lastCertValidation > CERT_VALIDATION_INTERVAL)
  {
    validateCertificates();
    lastCertValidation = currentTime;
  }

  delay(100);
}

void setupSensors()
{
  Serial.println("🔧 Setting up sensors...");

  if (ENABLE_TEMPERATURE_SENSOR)
  {
    Serial.println("  ✅ Temperature sensor enabled (Built-in)");
  }

  Serial.println("🔧 Sensor setup complete!");
}

void readSensors(JsonDocument &doc)
{
  // Read built-in temperature sensor
  if (ENABLE_TEMPERATURE_SENSOR)
  {
    float temperature = temperatureRead();
    doc["temperature"] = temperature;
  }
}

void connectToWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  wifiConnected = true;
  Serial.println("\nWiFi Connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("RSSI: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
}

void connectToAWS()
{
  if (!wifiConnected)
  {
    Serial.println("❌ Cannot connect to AWS - WiFi not connected");
    return;
  }

  // Configure WiFiClientSecure to use the AWS IoT device credentials
  net.setCACert(AWS_CERT_CA);
  net.setCertificate(AWS_CERT_CRT);
  net.setPrivateKey(AWS_CERT_PRIVATE);

  // Connect to the MQTT broker on the AWS endpoint
  client.setServer(AWS_IOT_ENDPOINT, AWS_IOT_PORT);

  // Create a message handler
  client.setCallback(messageHandler);

  Serial.println("Connecting to AWS IoT...");

  while (!client.connect(THING_NAME))
  {
    Serial.print(".");
    delay(100);
  }

  if (!client.connected())
  {
    Serial.println("AWS IoT Timeout!");
    mqttConnected = false;
    return;
  }

  mqttConnected = true;
  Serial.println("AWS IoT Connected!");

  // Subscribe to topics
  String subscribeTopic = String(THING_NAME) + "/sub";
  client.subscribe(subscribeTopic.c_str());
  Serial.println("Subscribed to: " + subscribeTopic);
}

void messageHandler(char *topic, byte *payload, unsigned int length)
{
  // Null terminate the payload
  payload[length] = '\0';

  Serial.print("Message received [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println((char *)payload);

  // Parse JSON message
  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, payload);

  if (error)
  {
    Serial.print("JSON parsing failed: ");
    Serial.println(error.c_str());
    return;
  }

  // Handle messages
  if (doc.containsKey("message"))
  {
    String message = doc["message"];
    Serial.println("Message: " + message);
  }

  if (doc.containsKey("led"))
  {
    String ledCommand = doc["led"];
    if (ledCommand == "on")
    {
      digitalWrite(LED_PIN, HIGH);
      Serial.println("LED turned ON via MQTT");
    }
    else if (ledCommand == "off")
    {
      digitalWrite(LED_PIN, LOW);
      Serial.println("LED turned OFF via MQTT");
    }
  }
}

void publishHeartbeat()
{
  if (!client.connected())
  {
    return;
  }

  StaticJsonDocument<200> doc;
  doc["device_id"] = THING_NAME;
  doc["timestamp"] = millis();
  doc["status"] = "alive";
  doc["wifi_connected"] = wifiConnected;
  doc["mqtt_connected"] = mqttConnected;
  doc["uptime"] = millis() / 1000;

  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);

  String publishTopic = String(THING_NAME) + "/heartbeat";
  if (client.publish(publishTopic.c_str(), jsonBuffer))
  {
    Serial.println("✓ Heartbeat published to: " + publishTopic);
  }
  else
  {
    Serial.println("✗ Failed to publish heartbeat");
    if (ENABLE_ERROR_DIAGNOSTICS)
    {
      logErrorDiagnostics("Heartbeat failed");
    }
  }
}

void publishDeviceStatus()
{
  if (!client.connected())
  {
    return;
  }

  StaticJsonDocument<200> doc;
  doc["device_id"] = THING_NAME;
  doc["timestamp"] = millis();
  doc["status"] = "online";
  doc["wifi_connected"] = wifiConnected;
  doc["mqtt_connected"] = mqttConnected;
  doc["wifi_rssi"] = WiFi.RSSI();
  doc["wifi_ssid"] = WiFi.SSID();
  doc["ip_address"] = WiFi.localIP().toString();
  doc["free_heap"] = ESP.getFreeHeap();
  doc["uptime"] = millis() / 1000;
  doc["led_state"] = digitalRead(LED_PIN) ? "on" : "off";

  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);

  String publishTopic = String(THING_NAME) + "/status";
  if (client.publish(publishTopic.c_str(), jsonBuffer))
  {
    Serial.println("✓ Status published to: " + publishTopic);
  }
  else
  {
    Serial.println("✗ Failed to publish status");
    if (ENABLE_ERROR_DIAGNOSTICS)
    {
      logErrorDiagnostics("Status publish failed");
    }
  }
}

void checkDNSResolution()
{
  // Safe DNS check for network diagnostics
  if (!wifiConnected)
  {
    return; // Skip if WiFi not connected
  }

  Serial.print("🔍 DNS Test: ");
  IPAddress testIP;
  if (WiFi.hostByName("google.com", testIP))
  {
    Serial.print("✅ OK (");
    Serial.print(testIP);
    Serial.println(")");
  }
  else
  {
    Serial.println("❌ FAILED - DNS not working");
    if (ENABLE_ERROR_DIAGNOSTICS)
    {
      logErrorDiagnostics("DNS resolution failed");
    }
  }
}

void validateCertificates()
{
  // Safe certificate validation for diagnostics
  Serial.println("🔒 Certificate Validation:");

  // Check CA Certificate
  Serial.print("  - CA Certificate: ");
  if (strlen(AWS_CERT_CA) > 100)
  {
    Serial.print("✅ OK (");
    Serial.print(strlen(AWS_CERT_CA));
    Serial.println(" bytes)");
  }
  else
  {
    Serial.println("❌ INVALID - Too short or empty");
    if (ENABLE_ERROR_DIAGNOSTICS)
    {
      logErrorDiagnostics("CA Certificate validation failed");
    }
  }

  // Check Device Certificate
  Serial.print("  - Device Certificate: ");
  if (strlen(AWS_CERT_CRT) > 100)
  {
    Serial.print("✅ OK (");
    Serial.print(strlen(AWS_CERT_CRT));
    Serial.println(" bytes)");
  }
  else
  {
    Serial.println("❌ INVALID - Too short or empty");
    if (ENABLE_ERROR_DIAGNOSTICS)
    {
      logErrorDiagnostics("Device Certificate validation failed");
    }
  }

  // Check Private Key
  Serial.print("  - Private Key: ");
  if (strlen(AWS_CERT_PRIVATE) > 100)
  {
    Serial.print("✅ OK (");
    Serial.print(strlen(AWS_CERT_PRIVATE));
    Serial.println(" bytes)");
  }
  else
  {
    Serial.println("❌ INVALID - Too short or empty");
    if (ENABLE_ERROR_DIAGNOSTICS)
    {
      logErrorDiagnostics("Private Key validation failed");
    }
  }

  // Check for proper certificate format
  Serial.print("  - Certificate Format: ");
  if (strstr(AWS_CERT_CA, "BEGIN CERTIFICATE") &&
      strstr(AWS_CERT_CRT, "BEGIN CERTIFICATE") &&
      strstr(AWS_CERT_PRIVATE, "BEGIN RSA PRIVATE KEY"))
  {
    Serial.println("✅ OK - Proper PEM format");
  }
  else
  {
    Serial.println("❌ INVALID - Missing PEM headers");
    if (ENABLE_ERROR_DIAGNOSTICS)
    {
      logErrorDiagnostics("Certificate format validation failed");
    }
  }
}

void checkConnectionStatus()
{
  // Safe connection monitoring with graceful reconnection
  static unsigned long lastCheck = 0;
  static bool lastConnectionState = true;
  static unsigned long lastReconnectAttempt = 0;

  // Check every 10 seconds to avoid being too aggressive
  if (millis() - lastCheck > 10000)
  {
    lastCheck = millis();

    bool currentConnection = client.connected();

    // Only log state changes to avoid spam
    if (currentConnection != lastConnectionState)
    {
      if (currentConnection)
      {
        Serial.println("✅ MQTT connection restored");
        mqttConnected = true;
      }
      else
      {
        Serial.println("❌ MQTT connection lost");
        mqttConnected = false;
      }
      lastConnectionState = currentConnection;
    }

    // Graceful reconnection - only if enabled and disconnected and enough time has passed
    if (ENABLE_GRACEFUL_RECONNECTION && !currentConnection && (millis() - lastReconnectAttempt > 30000)) // 30 second cooldown
    {
      Serial.println("🔄 Attempting graceful reconnection...");
      lastReconnectAttempt = millis();

      // Simple reconnection like the original working approach
      if (client.connect(THING_NAME))
      {
        Serial.println("✅ Graceful reconnection successful!");
        mqttConnected = true;
        lastConnectionState = true;

        // Re-subscribe to topics
        String subscribeTopic = String(THING_NAME) + "/sub";
        client.subscribe(subscribeTopic.c_str());
        Serial.println("Re-subscribed to: " + subscribeTopic);
      }
      else
      {
        Serial.println("❌ Graceful reconnection failed, will try again later");
      }
    }
  }
}

void publishDeviceData()
{
  if (!client.connected())
  {
    Serial.println("MQTT not connected, attempting to reconnect...");
    connectToAWS();
    return;
  }

  StaticJsonDocument<400> doc; // Increased size for additional system data
  doc["device_id"] = THING_NAME;
  doc["timestamp"] = millis();
  doc["wifi_rssi"] = WiFi.RSSI();
  doc["wifi_ssid"] = WiFi.SSID();
  doc["free_heap"] = ESP.getFreeHeap();
  doc["uptime"] = millis() / 1000;
  doc["mqtt_connected"] = mqttConnected;

  // System information
  doc["cpu_freq"] = ESP.getCpuFreqMHz();
  doc["flash_size"] = ESP.getFlashChipSize();
  doc["chip_model"] = ESP.getChipModel();
  doc["chip_revision"] = ESP.getChipRevision();

  // Read sensor data
  readSensors(doc);

  char jsonBuffer[512];
  serializeJson(doc, jsonBuffer);

  String publishTopic = String(THING_NAME) + "/pub";
  if (client.publish(publishTopic.c_str(), jsonBuffer))
  {
    Serial.println("✓ Data published to: " + publishTopic);
    Serial.println("  Payload: " + String(jsonBuffer));
  }
  else
  {
    Serial.println("✗ Failed to publish data");
    if (ENABLE_ERROR_DIAGNOSTICS)
    {
      logErrorDiagnostics("Publish failed");
    }
  }
}

void logErrorDiagnostics(const String &errorContext)
{
  // Basic error diagnostics - safe and non-intrusive
  Serial.println("🔍 Error Diagnostics for: " + errorContext);
  Serial.print("  - WiFi Status: ");
  Serial.println(WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");
  Serial.print("  - WiFi RSSI: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
  Serial.print("  - MQTT Connected: ");
  Serial.println(client.connected() ? "Yes" : "No");
  Serial.print("  - Free Heap: ");
  Serial.print(ESP.getFreeHeap());
  Serial.println(" bytes");
  Serial.print("  - Uptime: ");
  Serial.print(millis() / 1000);
  Serial.println(" seconds");
}

void updateLEDStatus()
{
  static unsigned long lastLEDUpdate = 0;
  static bool ledState = LOW;

  if (millis() - lastLEDUpdate > 1000)
  {
    lastLEDUpdate = millis();

    if (!wifiConnected)
    {
      // No WiFi - LED off
      digitalWrite(LED_PIN, LOW);
    }
    else if (!mqttConnected)
    {
      // WiFi connected, no MQTT - slow blink
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState);
    }
    else
    {
      // Both connected - show signal strength
      int rssi = WiFi.RSSI();
      if (rssi > -50)
      {
        // Strong signal - fast blink
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState);
      }
      else if (rssi > -70)
      {
        // Medium signal - medium blink
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState);
      }
      else
      {
        // Weak signal - slow blink
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState);
      }
    }
  }
}
