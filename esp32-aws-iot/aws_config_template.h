/*
 * AWS IoT Core Configuration Template
 *
 * Copy this file to aws_config.h and update with your AWS IoT Core settings.
 * aws_config.h is gitignored for security.
 *
 * To get these values:
 * 1. Go to AWS IoT Console
 * 2. Create a Thing
 * 3. Create and download certificates
 * 4. Create and attach a policy
 * 5. Get your AWS IoT endpoint
 */

#ifndef AWS_CONFIG_H
#define AWS_CONFIG_H

// AWS IoT Core Endpoint
// Get this from AWS IoT Console -> Settings -> Device data endpoint
const char *AWS_IOT_ENDPOINT = "your-endpoint-ats.iot.region.amazonaws.com";

// AWS IoT Core Port (always 8883 for MQTT over SSL/TLS)
const int AWS_IOT_PORT = 8883;

// Thing Name (must match the thing created in AWS IoT Console)
const char *THING_NAME = "ESP32_Thing";

// Certificate and Private Key
// These should be the actual certificate and private key content
// Replace the placeholder values with your actual certificates
const char *AWS_CERT_CA = R"(
-----BEGIN CERTIFICATE-----
Your Amazon Root CA 1 certificate content here
-----END CERTIFICATE-----
)";

const char *AWS_CERT_CRT = R"(
-----BEGIN CERTIFICATE-----
Your device certificate content here
-----END CERTIFICATE-----
)";

const char *AWS_CERT_PRIVATE = R"(
-----BEGIN RSA PRIVATE KEY-----
Your device private key content here
-----END RSA PRIVATE KEY-----
)";

// MQTT Topics (AWS IoT Core uses $aws/things/{thingName}/...)
const char *MQTT_TOPIC_PREFIX = "$aws/things/ESP32_Thing"; // Will be used as: $aws/things/ESP32_Thing/data, etc.

// MQTT Settings
const int MQTT_QOS = 1;         // Quality of Service level (0, 1, or 2)
const bool MQTT_RETAIN = false; // Retain messages

// Connection settings
const int MQTT_RECONNECT_DELAY = 5000;    // Delay between reconnection attempts (ms)
const int MQTT_KEEPALIVE_INTERVAL = 60;   // Keep alive interval (seconds)
const int MQTT_SOCKET_TIMEOUT_VALUE = 15; // Socket timeout (seconds)

// Message intervals
const unsigned long DATA_PUBLISH_INTERVAL = 30000; // 30 seconds
const unsigned long HEARTBEAT_INTERVAL = 60000;    // 60 seconds

#endif
