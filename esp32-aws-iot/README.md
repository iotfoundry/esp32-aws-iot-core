# ESP32 AWS IoT Core Device Code

Complete Arduino implementation for ESP32 devices to connect to AWS IoT Core with comprehensive error handling, MQTT communication, and device shadow support.

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Hardware Requirements](#hardware-requirements)
- [Software Requirements](#software-requirements)
- [Installation](#installation)
- [Configuration](#configuration)
- [Usage](#usage)
- [MQTT Topics](#mqtt-topics)
- [Error Handling](#error-handling)
- [Troubleshooting](#troubleshooting)
- [API Reference](#api-reference)
- [Examples](#examples)

## Overview

This ESP32 Arduino project provides a complete solution for connecting ESP32 microcontrollers to AWS IoT Core. It includes automatic WiFi connection, secure MQTT communication, device shadow synchronization, and comprehensive error handling with detailed diagnostics.

### Key Capabilities

- **Secure AWS IoT Core Connection**: Mutual TLS authentication with X.509 certificates
- **Device Shadow Integration**: Bidirectional state synchronization with AWS
- **MQTT Communication**: Publish sensor data and receive commands
- **Auto-reconnection**: Automatic reconnection on network failures
- **Error Diagnostics**: Detailed error reporting and troubleshooting guidance
- **Modular Design**: Easy to extend and customize

## Features

### Core Features

- ✅ **WiFi Management**: Automatic connection with retry logic
- ✅ **AWS IoT Core Integration**: Secure MQTT with mutual TLS
- ✅ **Device Shadow Support**: Full shadow document synchronization
- ✅ **Command Processing**: Handle cloud-to-device commands
- ✅ **Data Publishing**: Send sensor data and device status
- ✅ **Heartbeat System**: Regular status updates
- ✅ **Error Recovery**: Automatic reconnection and error handling
- ✅ **Serial Debugging**: Comprehensive debug output

### Advanced Features

- **Certificate Validation**: Automatic certificate format checking
- **Connection Diagnostics**: Detailed connection status reporting
- **MQTT QoS Support**: Reliable message delivery
- **JSON Message Format**: Structured data exchange
- **Configurable Timeouts**: Adjustable connection and operation timeouts
- **Memory Management**: Efficient memory usage for ESP32

## Hardware Requirements

### Required Hardware

- **ESP32 Development Board**: Any ESP32 or ESP32-S series board
- **USB Cable**: For programming and power
- **WiFi Network**: 2.4GHz WiFi connection
- **Computer**: For Arduino IDE and programming

### Recommended Hardware

- **ESP32 DevKitC**: Official Espressif development board
- **ESP32-S3**: For enhanced performance and features
- **External Antenna**: For better WiFi range (if supported by board)

## Software Requirements

### Required Software

- **Arduino IDE**: Version 2.0 or later
- **ESP32 Board Package**: Espressif ESP32 Arduino Core
- **Required Libraries**:
  - `WiFi` (built-in)
  - `WiFiClientSecure` (built-in)
  - `PubSubClient` (by Nick O'Leary)
  - `ArduinoJson` (by Benoit Blanchon)

### Installation Steps

1. **Install Arduino IDE**
   - Download from <https://www.arduino.cc/en/software>

2. **Install ESP32 Board Package**
   - Open Arduino IDE
   - Go to File → Preferences
   - Add this URL to Additional Board Manager URLs:

     ```plaintext
     https://espressif.github.io/arduino-esp32/package_esp32_index.json
     ```

   - Go to Tools → Board → Boards Manager
   - Search for "ESP32" and install "ESP32 by Espressif Systems"

3. **Install Required Libraries**
   - Go to Tools → Manage Libraries
   - Install the following libraries:
     - `PubSubClient` by Nick O'Leary
     - `ArduinoJson` by Benoit Blanchon

## Installation

### Quick Setup

1. **Clone the Repository**

   ```bash
   git clone <repository-url>
   cd esp32-aws-iot-core/esp32-aws-iot
   ```

2. **Run Setup Script**

   **Windows:**

   ```cmd
   setup.bat
   ```

   **Linux/macOS:**

   ```bash
   chmod +x setup.sh
   ./setup.sh
   ```

3. **Download Root CA Certificate**

   ```bash
   curl -o AmazonRootCA1.pem https://www.amazontrust.com/repository/AmazonRootCA1.pem
   ```

4. **Generate AWS Configuration**

   **Windows:**

   ```cmd
   generate_aws_config.ps1
   ```

   **Linux/macOS:**

   ```bash
   chmod +x generate_aws_config.sh
   ./generate_aws_config.sh
   ```

### Manual Setup

If you prefer manual setup:

1. **Configure WiFi**
   - Copy `wifi_config_template.h` to `wifi_config.h`
   - Edit `wifi_config.h` with your WiFi credentials

2. **Configure AWS**
   - Ensure `esp32_config.json` exists in the parent `terraform/` directory
   - Run the appropriate configuration generator script
   - Verify `aws_config.h` is generated correctly

3. **Download Dependencies**
   - Download `AmazonRootCA1.pem` from AWS Trust Repository
   - Place it in the `esp32-aws-iot/` directory

## Configuration

### WiFi Configuration

Edit `wifi_config.h`:

```cpp
#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

// WiFi credentials
#define WIFI_SSID "your_wifi_ssid"
#define WIFI_PASSWORD "your_wifi_password"

// WiFi connection settings
#define WIFI_TIMEOUT_MS 20000
#define WIFI_RETRY_ATTEMPTS 3

#endif
```

### AWS Configuration

The `aws_config.h` file is automatically generated and contains:

```cpp
// AWS IoT Core endpoint
const char* AWS_IOT_ENDPOINT = "your-endpoint.iot.region.amazonaws.com";
const int AWS_IOT_PORT = 8883;

// Device information
const char* THING_NAME = "your-thing-name";

// Certificates (automatically populated)
const char* AWS_CERT_CRT = "-----BEGIN CERTIFICATE-----\n...";
const char* AWS_CERT_PRIVATE = "-----BEGIN RSA PRIVATE KEY-----\n...";
const char* AWS_CERT_CA = "-----BEGIN CERTIFICATE-----\n...";

// MQTT configuration
const char* MQTT_TOPIC_PREFIX = "$aws/things/your-thing-name";
```

## Usage

### Basic Usage

1. **Open in Arduino IDE**
   - Open `esp32-aws-iot.ino` in Arduino IDE
   - Select your ESP32 board from Tools → Board
   - Select the correct COM port from Tools → Port

2. **Configure Settings**
   - Edit `wifi_config.h` with your WiFi credentials
   - Ensure `aws_config.h` is properly generated

3. **Upload and Monitor**
   - Click Upload to compile and upload to ESP32
   - Open Serial Monitor (115200 baud) to view debug output

### Serial Monitor Output

The device provides detailed serial output:

```plaintext
ESP-ROM:esp32c3-api1-20210207
WiFi connecting to: YourWiFiNetwork
WiFi connected! IP: 192.168.1.100
Connecting to AWS IoT Core: your-endpoint.iot.region.amazonaws.com
🔌 Connecting to AWS IoT Core with ID: your-thing-name
✅ AWS IoT Core connected!
📡 Subscribing to topics...
📤 Publishing initial data...
```

## MQTT Topics

### Published Topics

| Topic | Description | Message Format |
|-------|-------------|----------------|
| `$aws/things/{thing_name}/data` | Sensor data | JSON with timestamp, values |
| `$aws/things/{thing_name}/status` | Device status | JSON with connection state |
| `$aws/things/{thing_name}/heartbeat` | Heartbeat | JSON with uptime, memory |
| `$aws/things/{thing_name}/response` | Command responses | JSON with result, data |

### Subscribed Topics

| Topic | Description | Message Format |
|-------|-------------|----------------|
| `$aws/things/{thing_name}/commands` | Device commands | JSON with command, parameters |
| `$aws/things/{thing_name}/config` | Configuration updates | JSON with settings |
| `$aws/things/{thing_name}/shadow/update/accepted` | Shadow update confirmations | JSON shadow document |
| `$aws/things/{thing_name}/shadow/update/rejected` | Shadow update rejections | JSON error details |

## Error Handling

### Error Codes and Diagnostics

The device provides detailed error reporting:

#### MQTT Connection Errors

- **MQTT_CONNECTION_TIMEOUT (-4)**
  - **Diagnosis**: AWS resources don't exist
  - **Solution**: Create AWS IoT Core resources using Terraform

- **MQTT_CONNECT_FAILED (-2)**
  - **Diagnosis**: Certificate or policy issues
  - **Solution**: Check certificate registration and policy attachment

- **MQTT_CONNECT_UNAUTHORIZED (5)**
  - **Diagnosis**: Policy permission issues
  - **Solution**: Verify policy allows MQTT connect operations

#### Network Errors

- **WiFi Connection Failed**
  - **Diagnosis**: Invalid credentials or network unavailable
  - **Solution**: Check WiFi credentials and network availability

- **DNS Resolution Failed**
  - **Diagnosis**: Cannot resolve AWS IoT Core endpoint
  - **Solution**: Check internet connectivity and endpoint URL

## Troubleshooting

### Common Issues

#### 1. WiFi Connection Problems

**Symptoms:**

- Device fails to connect to WiFi
- "WiFi connection failed" error

**Solutions:**

- Verify WiFi credentials in `wifi_config.h`
- Check WiFi network availability
- Ensure 2.4GHz network (ESP32 doesn't support 5GHz)
- Check WiFi signal strength

#### 2. AWS IoT Core Connection Issues

**Symptoms:**

- MQTT connection timeout
- Certificate errors
- Authentication failures

**Solutions:**

- Verify AWS resources exist in IoT Core Console
- Check certificate registration and activation
- Ensure policy is attached to certificate
- Verify policy permissions
- Check endpoint URL format

#### 3. Certificate Issues

**Symptoms:**

- Certificate format errors
- SSL handshake failures
- "Bad credentials" errors

**Solutions:**

- Regenerate `aws_config.h` from Terraform output
- Verify certificate format (PEM encoding)
- Check certificate expiration
- Ensure `AmazonRootCA1.pem` is downloaded

## API Reference

### Core Functions

#### `void setup()`

Initializes the device, connects to WiFi, and establishes AWS IoT Core connection.

#### `void loop()`

Main program loop that handles MQTT communication, data publishing, and error monitoring.

#### `void connectToWiFi()`

Connects to the configured WiFi network with retry logic.

#### `void connectToMQTT()`

Establishes secure MQTT connection to AWS IoT Core.

#### `void publishDeviceData()`

Publishes sensor data to the data topic.

#### `void publishHeartbeat()`

Publishes device heartbeat information.

#### `void handleCommand(String command, String payload)`

Processes incoming MQTT commands.

## Examples

### Basic Sensor Data Publishing

```cpp
void publishSensorData() {
  // Read sensor values
  float temperature = readTemperature();
  float humidity = readHumidity();

  // Create JSON message
  DynamicJsonDocument doc(1024);
  doc["timestamp"] = millis();
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["device_id"] = THING_NAME;

  // Publish to MQTT
  String topic = String(MQTT_TOPIC_PREFIX) + "/data";
  String payload;
  serializeJson(doc, payload);

  mqttClient.publish(topic.c_str(), payload.c_str());
}
```

### Command Processing

```cpp
void handleCommand(String command, String payload) {
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, payload);

  if (command == "set_led") {
    bool state = doc["state"];
    int brightness = doc["brightness"];

    // Control LED
    setLED(state, brightness);

    // Send response
    sendCommandResponse("set_led", "success", "LED updated");
  }
}
```

## License

This project is licensed under the MIT License - see the [LICENSE](../LICENSE) file for details.

## Support

For issues and questions:

1. Check the [Troubleshooting](#troubleshooting) section
2. Review the [Error Handling](#error-handling) guide
3. Check AWS IoT Core Console for resource status
4. Open an issue on GitHub with detailed error information
