#ifndef DEVICE_CONFIG_H
#define DEVICE_CONFIG_H

// LED configuration
const int LED_PIN = 7; // ESP32C3 built-in LED

// MQTT configuration
const int MQTT_QOS = 1;
const bool MQTT_RETAIN = false;
const int MQTT_RECONNECT_DELAY = 5000;
const int MQTT_KEEPALIVE_INTERVAL = 60;
const int MQTT_SOCKET_TIMEOUT_VALUE = 30;

// Timing intervals (milliseconds)
const unsigned long MQTT_CHECK_INTERVAL = 10000;      // 10 seconds - connection check
const unsigned long LED_UPDATE_INTERVAL = 1000;       // 1 second - LED update
const unsigned long DATA_PUBLISH_INTERVAL = 5000;     // 5 seconds - more responsive
const unsigned long STATUS_PUBLISH_INTERVAL = 15000;  // 15 seconds - device status
const unsigned long HEARTBEAT_INTERVAL = 30000;       // 30 seconds - reasonable heartbeat
const unsigned long DNS_CHECK_INTERVAL = 30000;       // 30 seconds - DNS check when MQTT disconnected
const unsigned long CERT_VALIDATION_INTERVAL = 60000; // 60 seconds - certificate validation

// Sensor configuration
const bool ENABLE_TEMPERATURE_SENSOR = true; // Enable built-in temperature sensor

// Diagnostic function switches
const bool ENABLE_CONNECTION_MONITORING = true;  // Enable connection status monitoring
const bool ENABLE_GRACEFUL_RECONNECTION = true;  // Enable automatic reconnection
const bool ENABLE_ERROR_DIAGNOSTICS = true;      // Enable error diagnostics logging
const bool ENABLE_DNS_CHECKING = true;           // Enable DNS resolution checking
const bool ENABLE_CERTIFICATE_VALIDATION = true; // Enable certificate validation
const bool ENABLE_HEARTBEAT_MESSAGES = true;     // Enable heartbeat publishing
const bool ENABLE_STATUS_MESSAGES = true;        // Enable status publishing

#endif
