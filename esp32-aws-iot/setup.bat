@echo off
REM ESP32 AWS IoT Core Setup Script
REM This script helps set up the configuration files for the ESP32 AWS IoT Core project

echo ESP32 AWS IoT Core Setup
echo ========================
echo.

REM Check if we're in the right directory
if not exist "esp32-aws-iot.ino" (
    echo Error: Please run this script from the esp32-aws-iot directory
    pause
    exit /b 1
)

REM Create WiFi config if it doesn't exist
if not exist "wifi_config.h" (
    echo Creating wifi_config.h from template...
    copy wifi_config_template.h wifi_config.h
    echo ✓ wifi_config.h created
    echo   Please edit wifi_config.h with your WiFi credentials
) else (
    echo ✓ wifi_config.h already exists
)

REM Check for AmazonRootCA1.pem
if not exist "AmazonRootCA1.pem" (
    echo ⚠️  AmazonRootCA1.pem not found
    echo   Please download AmazonRootCA1.pem from AWS and place it in this directory
    echo   Download from: https://www.amazontrust.com/repository/AmazonRootCA1.pem
    echo.
)

REM Generate AWS config from JSON
echo Generating aws_config.h from terraform/esp32_config.json...
powershell -ExecutionPolicy Bypass -File generate_aws_config.ps1
if errorlevel 1 (
    echo ✗ Failed to generate aws_config.h with PowerShell
    echo   Please ensure:
    echo   - terraform/esp32_config.json exists with your AWS IoT Core configuration
    echo   - AmazonRootCA1.pem is downloaded and placed in this directory
    echo.
    echo For manual configuration:
    echo 1. Copy aws_config_template.h to aws_config.h
    echo 2. Edit aws_config.h with your AWS IoT Core settings from terraform/esp32_config.json
    echo 3. Replace the placeholder values with actual certificates and endpoint
) else (
    echo ✓ aws_config.h generated successfully
)

echo.
echo Setup complete!
echo.
echo Next steps:
echo 1. Edit wifi_config.h with your WiFi credentials
echo 2. Open esp32-aws-iot.ino in Arduino IDE
echo 3. Upload to your ESP32
echo.
echo Note: aws_config.h is automatically generated from terraform/esp32_config.json
echo For detailed instructions, see README.md
pause
