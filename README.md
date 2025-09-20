# ESP32 AWS IoT Core

Complete solution for ESP32 AWS IoT Core integration with Terraform infrastructure provisioning and MQTT communication.

## Overview

This project provides a complete setup for connecting ESP32 devices to AWS IoT Core, including:

- **Terraform Infrastructure**: Automated provisioning of AWS IoT Core resources
- **Device Configuration**: Secure certificate and endpoint management with automated generation
- **MQTT Communication**: Device-to-cloud and cloud-to-device messaging
- **Device Shadow**: State synchronization and management
- **ESP32 Arduino Code**: Complete implementation with error handling and diagnostics

## Quick Start

### 1. Deploy AWS Infrastructure

```bash
cd terraform
terraform init
terraform apply
```

### 2. Setup ESP32 Device

```bash
cd esp32-aws-iot
# Windows
setup.bat

# Linux/macOS
chmod +x setup.sh
./setup.sh
```

### 3. Configure and Upload

1. Edit `wifi_config.h` with your WiFi credentials
2. Generate AWS configuration from Terraform output
3. Upload `esp32-aws-iot.ino` to your ESP32 device

## Project Structure

```plaintext
esp32-aws-iot-core/
├── terraform/                    # AWS infrastructure as code
│   ├── main.tf                  # IoT Core resources
│   ├── esp32_config.json        # Generated device config (gitignored)
│   └── README.md                # Terraform documentation
└── esp32-aws-iot/               # ESP32 Arduino code
    ├── esp32-aws-iot.ino        # Main Arduino sketch
    ├── aws_config.h             # Generated AWS configuration
    ├── wifi_config.h            # WiFi configuration
    ├── AmazonRootCA1.pem        # AWS Root CA certificate
    ├── generate_aws_config.ps1  # Windows config generator
    ├── generate_aws_config.sh   # Linux/macOS config generator
    ├── setup.bat                # Windows setup script
    ├── setup.sh                 # Linux/macOS setup script
    └── README.md                # ESP32 documentation
```

## Key Features

- **🔐 Secure Connection**: Mutual TLS authentication with X.509 certificates
- **📡 MQTT Communication**: Publish sensor data and receive commands
- **🔄 Device Shadow**: Bidirectional state synchronization with AWS
- **🛠️ Error Handling**: Comprehensive diagnostics and troubleshooting
- **🔄 Auto-reconnection**: Automatic recovery from network issues
- **📊 Data Publishing**: Send sensor data, status, and heartbeat messages

## Documentation

- **[Terraform Setup](terraform/README.md)** - Complete infrastructure provisioning guide
- **[ESP32 Setup](esp32-aws-iot/README.md)** - Detailed device configuration and programming guide

## Requirements

- **Hardware**: ESP32 development board
- **Software**: Arduino IDE with ESP32 board support
- **Cloud**: AWS account with IoT Core service
- **Network**: WiFi connection for ESP32

## Security

- All sensitive data (certificates, private keys) is automatically excluded from version control
- Device-specific policies with least privilege access
- Secure MQTT communication with mutual TLS authentication
- Automatic certificate validation and error reporting

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
