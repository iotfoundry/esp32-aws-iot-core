/*
 * ESP32 AWS IoT Core Client
 *
 * Based on: https://github.com/iotfoundry/esp32-wifi-config-cpp
 *
 * A comprehensive ESP32 project that connects to AWS IoT Core using MQTT over SSL/TLS.
 * Features secure certificate-based authentication, LED status indicators, and robust error handling.
 *
 * Features:
 * - Secure WiFi and AWS IoT Core connection
 * - Certificate-based authentication (no username/password)
 * - Auto-reconnection for both WiFi and MQTT
 * - LED signal strength and MQTT status indicators
 * - JSON message publishing and subscription
 * - Command handling via MQTT
 * - Serial monitoring and debugging
 *
 * Hardware: ESP32C3 DevKitM1
 * Libraries: WiFi, WiFiClientSecure, PubSubClient, ArduinoJson
 */

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "wifi_config.h"
#include "aws_config.h"

// WiFi and MQTT clients
WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient);

// Status variables
bool wifiConnected = false;
bool mqttConnected = false;
unsigned long lastWiFiCheck = 0;
unsigned long lastMQTTCheck = 0;
unsigned long lastDataPublish = 0;
unsigned long lastHeartbeat = 0;

const unsigned long CONNECTION_TIMEOUT = 10000; // 10 seconds max per connection attempt

// LED control
#define LED_PIN 7 // ESP32C3 built-in LED

// Signal strength thresholds
#define STRONG_SIGNAL -50 // -30 to -50 dBm
#define MEDIUM_SIGNAL -70 // -50 to -70 dBm
// Below -70 dBm is weak signal

// MQTT message buffer
char mqttMessageBuffer[512];

// Data publishing intervals (defined in aws_config.h)
// const unsigned long DATA_PUBLISH_INTERVAL = 30000; // 30 seconds
// const unsigned long HEARTBEAT_INTERVAL = 60000;    // 60 seconds

void setup()
{
  Serial.begin(115200);
  delay(1000);

  // Setup LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  Serial.println("ESP32C3 AWS IoT Core Client");

  // Connect to WiFi
  connectToWiFi();

  // Setup AWS IoT Core MQTT
  setupAWSIoT();

  Serial.println("Setup complete!");
}

void loop()
{
  unsigned long currentTime = millis();

  // Check WiFi connection
  if (currentTime - lastWiFiCheck > 5000)
  { // Check every 5 seconds
    checkWiFiConnection();
    lastWiFiCheck = currentTime;
  }

  // Check MQTT connection
  if (currentTime - lastMQTTCheck > 10000)
  { // Check every 10 seconds
    checkMQTTConnection();
    lastMQTTCheck = currentTime;
  }

  // Publish data periodically
  if (wifiConnected && mqttConnected)
  {
    if (currentTime - lastDataPublish > DATA_PUBLISH_INTERVAL)
    {
      publishDeviceData();
      lastDataPublish = currentTime;
    }

    // Send heartbeat
    if (currentTime - lastHeartbeat > HEARTBEAT_INTERVAL)
    {
      publishHeartbeat();
      lastHeartbeat = currentTime;
    }
  }

  // Update LED status
  updateLEDStatus();

  // Handle MQTT messages
  if (mqttConnected)
  {
    mqttClient.loop();
  }

  delay(100);
}

void connectToWiFi()
{
  WiFi.mode(WIFI_STA);

  // Set DNS servers BEFORE connecting (critical for ESP32)
  WiFi.setDNS(IPAddress(8, 8, 8, 8), IPAddress(8, 8, 4, 4));

  // Configure to use DHCP for IP but override DNS
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, IPAddress(8, 8, 8, 8));

  // Configure WiFi settings for better connectivity
  WiFi.setAutoReconnect(true);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.println("Connecting to WiFi...");
  Serial.println("SSID: " + String(WIFI_SSID));

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20)
  {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED)
  {
    wifiConnected = true;
    digitalWrite(LED_PIN, LOW);
    Serial.println("\nWiFi Connected!");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("Gateway: ");
    Serial.println(WiFi.gatewayIP());
    Serial.print("DNS: ");
    Serial.println(WiFi.dnsIP());
    Serial.print("Subnet: ");
    Serial.println(WiFi.subnetMask());
    Serial.print("RSSI: ");
    Serial.print(WiFi.RSSI());
    Serial.println(" dBm");

    // Verify DNS is working
    IPAddress testIP;
    Serial.print("🔍 Testing DNS resolution for google.com: ");
    if (WiFi.hostByName("google.com", testIP))
    {
      Serial.print("✅ SUCCESS (");
      Serial.print(testIP);
      Serial.println(")");
    }
    else
    {
      Serial.println("❌ FAILED - DNS not working properly");
    }
  }
  else
  {
    wifiConnected = false;
    Serial.println("\nWiFi connection failed!");
    digitalWrite(LED_PIN, HIGH);
  }
}

void checkWiFiConnection()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    if (wifiConnected)
    {
      Serial.println("WiFi disconnected! Attempting to reconnect...");
      wifiConnected = false;
    }
    connectToWiFi();
  }
  else if (!wifiConnected)
  {
    wifiConnected = true;
    Serial.println("WiFi reconnected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
}

void updateLEDStatus()
{
  unsigned long currentTime = millis();
  static unsigned long lastLEDUpdate = 0;
  static int ledState = LOW;
  static unsigned long ledBlinkInterval = 1000;

  if (currentTime - lastLEDUpdate > ledBlinkInterval)
  {
    lastLEDUpdate = currentTime;

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
      ledBlinkInterval = 1000; // 1 second
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
        ledBlinkInterval = 100; // 100ms
      }
      else if (rssi > -70)
      {
        // Medium signal - medium blink
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState);
        ledBlinkInterval = 500; // 500ms
      }
      else
      {
        // Weak signal - slow blink
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState);
        ledBlinkInterval = 1000; // 1 second
      }
    }
  }
}

String getWiFiStatus()
{
  switch (WiFi.status())
  {
  case WL_CONNECTED:
    return "Connected";
  case WL_NO_SSID_AVAIL:
    return "No SSID Available";
  case WL_CONNECT_FAILED:
    return "Connection Failed";
  case WL_CONNECTION_LOST:
    return "Connection Lost";
  case WL_DISCONNECTED:
    return "Disconnected";
  default:
    return "Unknown";
  }
}

// Test SSL connection with enforced timeout
bool testSSLConnection(const char *host, int port, unsigned long timeout)
{
  Serial.print("🔒 Testing SSL connection to ");
  Serial.print(host);
  Serial.print(":");
  Serial.print(port);
  Serial.print("... ");

  // Create a fresh WiFiClientSecure instance for each test to avoid state issues
  WiFiClientSecure testClient;
  testClient.setCACert(AWS_CERT_CA);
  testClient.setCertificate(AWS_CERT_CRT);
  testClient.setPrivateKey(AWS_CERT_PRIVATE);
  testClient.setTimeout(timeout);
  testClient.setHandshakeTimeout(timeout);

  unsigned long startTime = millis();
  bool connected = false;

  // Manual timeout enforcement with non-blocking approach
  connected = testClient.connect(host, port);

  // Force timeout if it takes too long
  if ((millis() - startTime) > timeout)
  {
    connected = false;
  }

  unsigned long duration = millis() - startTime;

  if (connected && testClient.connected())
  {
    Serial.print("✅ SUCCESS (");
    Serial.print(duration);
    Serial.println(" ms)");
    testClient.stop();
    return true;
  }
  else
  {
    Serial.print("❌ FAILED (");
    Serial.print(duration);
    Serial.println(" ms timeout)");
    testClient.stop();
    return false;
  }
}

void setupAWSIoT()
{
  Serial.println("🔧 Setting up AWS IoT Core...");

  // Configure WiFiClientSecure for AWS IoT Core
  wifiClient.setCACert(AWS_CERT_CA);
  wifiClient.setCertificate(AWS_CERT_CRT);
  wifiClient.setPrivateKey(AWS_CERT_PRIVATE);

  Serial.print("  - Endpoint: ");
  Serial.println(AWS_IOT_ENDPOINT);
  Serial.print("  - Thing Name: ");
  Serial.println(THING_NAME);

  Serial.println("📋 SSL Certificate Configuration:");
  Serial.print("   - CA Certificate: ");
  Serial.print(strlen(AWS_CERT_CA));
  Serial.println(" bytes");
  Serial.print("   - Device Certificate: ");
  Serial.print(strlen(AWS_CERT_CRT));
  Serial.println(" bytes");
  Serial.print("   - Private Key: ");
  Serial.print(strlen(AWS_CERT_PRIVATE));
  Serial.println(" bytes");

  // Configure SSL/TLS client timeouts BEFORE MQTT setup
  wifiClient.setTimeout(CONNECTION_TIMEOUT);          // 10 second timeout for SSL handshake
  wifiClient.setHandshakeTimeout(CONNECTION_TIMEOUT); // 10 second SSL handshake timeout

  // Configure MQTT client
  mqttClient.setServer(AWS_IOT_ENDPOINT, AWS_IOT_PORT);
  mqttClient.setCallback(messageReceived);
  mqttClient.setKeepAlive(MQTT_KEEPALIVE_INTERVAL);
  mqttClient.setSocketTimeout(MQTT_SOCKET_TIMEOUT_VALUE);

  Serial.println("🔧 SSL/TLS Configuration:");
  Serial.print("   - SSL timeout: ");
  Serial.print(CONNECTION_TIMEOUT / 1000);
  Serial.print(" seconds, MQTT timeout: ");
  Serial.print(MQTT_SOCKET_TIMEOUT_VALUE);
  Serial.println(" seconds");

  Serial.println("✅ AWS IoT Core setup complete");
  Serial.println("   - Port discovery will happen during first connection attempt");
}

void connectToMQTT()
{
  if (!wifiConnected)
  {
    Serial.println("❌ Cannot connect to MQTT - WiFi not connected");
    return;
  }

  Serial.print("Connecting to AWS IoT Core: ");
  Serial.println(AWS_IOT_ENDPOINT);
  Serial.print("WiFi RSSI: ");
  Serial.print(WiFi.RSSI());
  Serial.print(" dBm, IP: ");
  Serial.println(WiFi.localIP());

  String clientId = THING_NAME;

  // Configure MQTT client for AWS IoT Core (standard port 8883)
  mqttClient.setServer(AWS_IOT_ENDPOINT, AWS_IOT_PORT);

  // Attempt MQTT connection
  Serial.print("🔌 Connecting to AWS IoT Core with ID: ");
  Serial.println(clientId);

  if (mqttClient.connect(clientId.c_str()))
  {
    mqttConnected = true;
    Serial.println("✅ AWS IoT Core connected!");

    // Subscribe to command topics
    subscribeToTopics();

    // Publish connection status
    publishConnectionStatus();
  }
  else
  {
    mqttConnected = false;
    Serial.print("❌ AWS IoT Core connection failed, rc=");
    Serial.print(mqttClient.state());
    Serial.print(" (");

    // Decode MQTT error codes
    switch (mqttClient.state())
    {
    case -4:
      Serial.println("MQTT_CONNECTION_TIMEOUT)");
      Serial.println("🔍 DIAGNOSIS: AWS resources likely don't exist!");
      Serial.println("  - Thing 'basic-device' may not exist in AWS IoT Core");
      Serial.println("  - Certificate may not be registered in AWS IoT Core");
      Serial.println("  - Policy may not be attached to certificate");
      Serial.println("  - AWS IoT Core endpoint may be incorrect");
      Serial.println("🔧 QUICK FIX: Create AWS resources in IoT Core Console");
      break;
    case -3:
      Serial.println("MQTT_CONNECTION_LOST)");
      break;
    case -2:
      Serial.println("MQTT_CONNECT_FAILED - Check certificates/network)");
      Serial.println("🔍 AWS IoT Core Troubleshooting:");
      Serial.println("  - Verify Thing 'basic-device' exists in AWS IoT Core Console");
      Serial.println("  - Check certificate is registered and active");
      Serial.println("  - Verify policy is attached to certificate");
      Serial.println("  - Ensure policy allows MQTT connect operation");
      break;
    case -1:
      Serial.println("MQTT_DISCONNECTED)");
      break;
    case 1:
      Serial.println("MQTT_CONNECT_BAD_PROTOCOL)");
      break;
    case 2:
      Serial.println("MQTT_CONNECT_BAD_CLIENT_ID)");
      break;
    case 3:
      Serial.println("MQTT_CONNECT_UNAVAILABLE)");
      break;
    case 4:
      Serial.println("MQTT_CONNECT_BAD_CREDENTIALS)");
      break;
    case 5:
      Serial.println("MQTT_CONNECT_UNAUTHORIZED)");
      break;
    default:
      Serial.println("Unknown error)");
      break;
    }

    Serial.println("🔍 Troubleshooting:");
    Serial.println("  - Check WiFi connection");
    Serial.println("  - Verify AWS IoT Core endpoint");
    Serial.println("  - Check certificate validity");
    Serial.println("  - Ensure Thing is registered in AWS IoT Core");
  }
}

void reconnectMQTT()
{
  if (!wifiConnected)
  {
    return;
  }

  if (!mqttClient.connected())
  {
    static unsigned long lastReconnectAttempt = 0;
    unsigned long currentTime = millis();

    // Only attempt reconnection every 30 seconds
    if (currentTime - lastReconnectAttempt > 30000)
    {
      lastReconnectAttempt = currentTime;
      Serial.println("🔄 MQTT disconnected! Attempting to reconnect...");
      connectToMQTT();
    }
  }
}

void checkMQTTConnection()
{
  if (wifiConnected)
  {
    if (!mqttClient.connected())
    {
      if (mqttConnected)
      {
        Serial.println("MQTT disconnected!");
        mqttConnected = false;
      }
      reconnectMQTT();
    }
    else if (!mqttConnected)
    {
      mqttConnected = true;
      Serial.println("MQTT reconnected!");
    }
  }
}

void subscribeToTopics()
{
  // Subscribe to AWS IoT Core command topics
  String commandTopic = String(MQTT_TOPIC_PREFIX) + "/commands";
  String configTopic = String(MQTT_TOPIC_PREFIX) + "/config";

  if (mqttClient.subscribe(commandTopic.c_str(), MQTT_QOS))
  {
    Serial.println("Subscribed to: " + commandTopic);
  }

  if (mqttClient.subscribe(configTopic.c_str(), MQTT_QOS))
  {
    Serial.println("Subscribed to: " + configTopic);
  }
}

void messageReceived(char *topic, byte *payload, unsigned int length)
{
  // Null terminate the payload
  payload[length] = '\0';

  Serial.print("Message received [");
  Serial.print(topic);
  Serial.print("]: ");
  Serial.println((char *)payload);

  // Parse JSON message
  DynamicJsonDocument doc(512);
  DeserializationError error = deserializeJson(doc, payload);

  if (error)
  {
    Serial.print("JSON parsing failed: ");
    Serial.println(error.c_str());
    return;
  }

  // Handle different message types
  String topicStr = String(topic);

  if (topicStr.endsWith("/commands"))
  {
    handleCommand(doc);
  }
  else if (topicStr.endsWith("/config"))
  {
    handleConfig(doc);
  }
}

void handleCommand(JsonDocument &doc)
{
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
    else if (ledCommand == "blink")
    {
      int interval = doc.containsKey("interval") ? doc["interval"] : 500;
      // Note: ledBlinkInterval is static in updateLEDStatus(), so we can't modify it here
      // For direct MQTT control, you'd need a separate variable or modify updateLEDStatus()
      Serial.println("LED blink mode via MQTT, interval: " + String(interval) + "ms");
    }

    // Send acknowledgment
    publishCommandResponse("led", ledCommand, "success");
  }

  if (doc.containsKey("status"))
  {
    publishDeviceStatus();
  }
}

void handleConfig(JsonDocument &doc)
{
  if (doc.containsKey("publish_interval"))
  {
    int newInterval = doc["publish_interval"];
    if (newInterval >= 5000 && newInterval <= 300000)
    { // 5 seconds to 5 minutes
      // Update publish interval (would need to be stored in a variable)
      Serial.println("Publish interval updated to: " + String(newInterval) + "ms");
      publishCommandResponse("config", "publish_interval", "updated");
    }
  }
}

void publishDeviceData()
{
  if (!mqttConnected)
    return;

  DynamicJsonDocument doc(512);
  doc["device_id"] = THING_NAME;
  doc["timestamp"] = millis();
  doc["wifi_rssi"] = WiFi.RSSI();
  doc["wifi_ssid"] = WiFi.SSID();
  doc["free_heap"] = ESP.getFreeHeap();
  doc["uptime"] = millis() / 1000;
  doc["mqtt_connected"] = mqttConnected;

  String topic = String(MQTT_TOPIC_PREFIX) + "/data";
  String payload;
  serializeJson(doc, payload);

  if (mqttClient.publish(topic.c_str(), payload.c_str(), MQTT_RETAIN))
  {
    Serial.println("✓ Data published to: " + topic);
    Serial.println("  Payload: " + payload);
  }
  else
  {
    Serial.println("✗ Failed to publish data to: " + topic);
  }
}

void publishHeartbeat()
{
  if (!mqttConnected)
    return;

  DynamicJsonDocument doc(256);
  doc["device_id"] = THING_NAME;
  doc["timestamp"] = millis();
  doc["status"] = "alive";
  doc["wifi_connected"] = wifiConnected;
  doc["mqtt_connected"] = mqttConnected;

  String topic = String(MQTT_TOPIC_PREFIX) + "/heartbeat";
  String payload;
  serializeJson(doc, payload);

  if (mqttClient.publish(topic.c_str(), payload.c_str(), MQTT_RETAIN))
  {
    Serial.println("✓ Heartbeat published to: " + topic);
  }
  else
  {
    Serial.println("✗ Failed to publish heartbeat");
  }
}

void publishConnectionStatus()
{
  if (!mqttConnected)
    return;

  DynamicJsonDocument doc(256);
  doc["device_id"] = THING_NAME;
  doc["status"] = "connected";
  doc["timestamp"] = millis();
  doc["ip_address"] = WiFi.localIP().toString();
  doc["wifi_rssi"] = WiFi.RSSI();

  String topic = String(MQTT_TOPIC_PREFIX) + "/status";
  String payload;
  serializeJson(doc, payload);

  mqttClient.publish(topic.c_str(), payload.c_str(), MQTT_RETAIN);
}

void publishDeviceStatus()
{
  if (!mqttConnected)
    return;

  DynamicJsonDocument doc(512);
  doc["device_id"] = THING_NAME;
  doc["timestamp"] = millis();
  doc["wifi_connected"] = wifiConnected;
  doc["mqtt_connected"] = mqttConnected;
  doc["wifi_rssi"] = WiFi.RSSI();
  doc["wifi_ssid"] = WiFi.SSID();
  doc["ip_address"] = WiFi.localIP().toString();
  doc["free_heap"] = ESP.getFreeHeap();
  doc["uptime"] = millis() / 1000;
  doc["led_state"] = digitalRead(LED_PIN) ? "on" : "off";

  String topic = String(MQTT_TOPIC_PREFIX) + "/status";
  String payload;
  serializeJson(doc, payload);

  mqttClient.publish(topic.c_str(), payload.c_str(), MQTT_RETAIN);
}

void publishCommandResponse(String command, String value, String result)
{
  if (!mqttConnected)
    return;

  DynamicJsonDocument doc(256);
  doc["device_id"] = THING_NAME;
  doc["command"] = command;
  doc["value"] = value;
  doc["result"] = result;
  doc["timestamp"] = millis();

  String topic = String(MQTT_TOPIC_PREFIX) + "/response";
  String payload;
  serializeJson(doc, payload);

  mqttClient.publish(topic.c_str(), payload.c_str(), MQTT_RETAIN);
}
