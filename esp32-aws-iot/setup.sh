#!/bin/bash

# ESP32 AWS IoT Core Setup Script
# This script helps set up the configuration files for the ESP32 AWS IoT Core project

echo "ESP32 AWS IoT Core Setup"
echo "========================"
echo ""

# Check if we're in the right directory
if [ ! -f "esp32-aws-iot.ino" ]; then
    echo "Error: Please run this script from the esp32-aws-iot directory"
    exit 1
fi

# Create WiFi config if it doesn't exist
if [ ! -f "wifi_config.h" ]; then
    echo "Creating wifi_config.h from template..."
    cp wifi_config_template.h wifi_config.h
    echo "✓ wifi_config.h created"
    echo "  Please edit wifi_config.h with your WiFi credentials"
else
    echo "✓ wifi_config.h already exists"
fi

# Check for AmazonRootCA1.pem
if [ ! -f "AmazonRootCA1.pem" ]; then
    echo "⚠️  AmazonRootCA1.pem not found"
    echo "  Please download AmazonRootCA1.pem from AWS and place it in this directory"
    echo "  Download from: https://www.amazontrust.com/repository/AmazonRootCA1.pem"
    echo ""
fi

# Generate AWS config from JSON
echo "Generating aws_config.h from terraform/esp32_config.json..."

# Try shell script first, then fall back to PowerShell
if bash generate_aws_config.sh; then
    echo "✓ aws_config.h generated successfully (using shell script)"
elif command -v powershell &> /dev/null && powershell -ExecutionPolicy Bypass -File generate_aws_config.ps1; then
    echo "✓ aws_config.h generated successfully (using PowerShell)"
else
    echo "✗ Failed to generate aws_config.h"
    echo "  Please ensure:"
    echo "  - terraform/esp32_config.json exists with your AWS IoT Core configuration"
    echo "  - AmazonRootCA1.pem is downloaded and placed in this directory"
    echo "  You can also manually copy aws_config_template.h to aws_config.h and edit it"
    exit 1
fi

echo ""
echo "Setup complete!"
echo ""
echo "Next steps:"
echo "1. Edit wifi_config.h with your WiFi credentials"
echo "2. Open esp32-aws-iot.ino in Arduino IDE"
echo "3. Upload to your ESP32"
echo ""
echo "Note: aws_config.h is automatically generated from terraform/esp32_config.json"
echo "For detailed instructions, see README.md"
