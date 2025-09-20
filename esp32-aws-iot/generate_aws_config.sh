#!/bin/bash

# Generate aws_config.h from terraform/esp32_config.json
echo "Generating aws_config.h from terraform/esp32_config.json..."

# Check if JSON file exists
if [ ! -f "../terraform/esp32_config.json" ]; then
    echo "Error: ../terraform/esp32_config.json not found!"
    exit 1
fi

# Extract simple values
ENDPOINT=$(grep -A 3 '"endpoint"' ../terraform/esp32_config.json | grep '"value"' | sed 's/.*"value": *"\([^"]*\)".*/\1/')
THING_NAME=$(grep -A 3 '"thing_name"' ../terraform/esp32_config.json | grep '"value"' | sed 's/.*"value": *"\([^"]*\)".*/\1/')

# Extract certificate - get everything between "value": " and the closing "
CERT_PEM=$(sed -n '/"certificate_pem"/,/^[[:space:]]*}/p' ../terraform/esp32_config.json | sed -n '/"value": "/,/^[[:space:]]*"$/p' | sed '1s/.*"value": *"//' | sed '$s/"$//' | sed 's/\\n/\n/g' | head -n -1)

# Extract private key - get everything between "value": " and the closing "
PRIVATE_KEY=$(sed -n '/"private_key"/,/^[[:space:]]*}/p' ../terraform/esp32_config.json | sed -n '/"value": "/,/^[[:space:]]*"$/p' | sed '1s/.*"value": *"//' | sed '$s/"$//' | sed 's/\\n/\n/g' | head -n -1)

# Debug output
echo "Debug - Extracted values:"
echo "  Endpoint: $ENDPOINT"
echo "  Thing Name: $THING_NAME"
echo "  Certificate length: ${#CERT_PEM} characters"
echo "  Private key length: ${#PRIVATE_KEY} characters"

# Validate required fields
if [ -z "$ENDPOINT" ] || [ -z "$THING_NAME" ] || [ -z "$CERT_PEM" ] || [ -z "$PRIVATE_KEY" ]; then
    echo "Error: Missing required fields in JSON configuration"
    exit 1
fi

# Generate the C header file
cat > aws_config.h << EOF
/*
 * AWS IoT Core Configuration
 * Generated from terraform/esp32_config.json
 */

#ifndef AWS_CONFIG_H
#define AWS_CONFIG_H

const char* AWS_IOT_ENDPOINT = "$ENDPOINT";
const int AWS_IOT_PORT = 8883;
const char* THING_NAME = "$THING_NAME";

const char* AWS_CERT_CRT = R"(
$CERT_PEM
)";

const char* AWS_CERT_PRIVATE = R"(
$PRIVATE_KEY
)";

const char* AWS_CERT_CA = R"(
$(cat AmazonRootCA1.pem)
)";

const char* MQTT_TOPIC_PREFIX = "\\\$aws/things/$THING_NAME";
const int MQTT_QOS = 1;
const bool MQTT_RETAIN = false;
const int MQTT_RECONNECT_DELAY = 5000;
const int MQTT_KEEPALIVE_INTERVAL = 60;
const int MQTT_SOCKET_TIMEOUT_VALUE = 15;
const unsigned long DATA_PUBLISH_INTERVAL = 30000;
const unsigned long HEARTBEAT_INTERVAL = 60000;

#endif
EOF

echo "✓ aws_config.h generated successfully"
echo "  - Endpoint: $ENDPOINT"
echo "  - Thing Name: $THING_NAME"
