# ESP32 AWS IoT Core Configuration

This Terraform configuration creates AWS IoT Core resources for ESP32 device connectivity using the terraform-aws-iot-core module.

## What it creates

- 1 IoT Thing with attributes
- 1 IoT Certificate (with PEM, private key, and public key)
- 1 IoT Policy (comprehensive inline JSON)
- Policy attachment to certificate
- Certificate attachment to thing

## Usage

### 1. Initialize and Deploy

```bash
# Navigate to the terraform directory
cd terraform

# Initialize Terraform
terraform init

# Plan deployment
terraform plan

# Apply configuration
terraform apply
```

### 2. Generate ESP32 Configuration

```bash
# Generate configuration file for ESP32
terraform output -json > esp32_config.json
```

This creates:

- `esp32_config.json` - JSON format with all AWS IoT values (contains sensitive data - do not commit to git)

### 3. View Outputs

```bash
# View all outputs
terraform output

# View specific outputs (non-sensitive)
terraform output thing_name
terraform output endpoint

# View sensitive outputs (certificates)
terraform output certificate_pem
terraform output private_key
```

## Configuration

This configuration uses a single `main.tf` file with:

- **Local values** - `thing_name` is set as "basic-device"
- **AWS Provider** - Configured for ap-southeast-2 region
- **IoT Module** - Uses tfstack/iot-core/aws module
- **Essential outputs only** - Just what's needed for ESP32

## Outputs

| Name            | Description                   | Sensitive |
| --------------- | ----------------------------- | :-------: |
| thing_name      | Name of the created IoT thing |    no     |
| certificate_pem | IoT certificate PEM           |    yes    |
| private_key     | IoT certificate private key   |    yes    |
| endpoint        | IoT endpoint                  |    no     |

## Policy

The example creates a comprehensive IoT policy that allows the device to:

- **Connect** to IoT Core (with thing attachment condition)
- **Publish** to topics under `device/{thing_name}/*` and device shadow
- **Subscribe** to topics under `device/{thing_name}/*` and device shadow
- **Receive** messages from subscribed topics

### Policy Features

- **Security**: Uses `iot:Connection.Thing.IsAttached` condition for connect
- **Shadow Access**: Includes device shadow read/write permissions
- **Topic Isolation**: Scoped to device-specific topics
- **Least Privilege**: Only necessary permissions granted

## Thing Attributes

The IoT thing is created with the following attributes:

- `deviceType`: "basic-device"
- `version`: "1.0.0"
- `environment`: "dev"

## ESP32 Integration

### 1. Use Generated Configuration

The `esp32_config.json` file contains all the values you need for ESP32 configuration:

```json
{
  "thing_name": {
    "value": "basic-device"
  },
  "endpoint": {
    "value": "iot.ap-southeast-2.amazonaws.com"
  },
  "certificate_pem": {
    "value": "-----BEGIN CERTIFICATE-----\n...",
    "sensitive": true
  },
  "private_key": {
    "value": "-----BEGIN RSA PRIVATE KEY-----\n...",
    "sensitive": true
  }
}
```

### 2. Available Values

- `thing_name` - Your device name ("basic-device")
- `endpoint` - MQTT broker endpoint
- `certificate_pem` - Device certificate (PEM format)
- `private_key` - Device private key (PEM format)

### 3. Security Note

⚠️ **Important**: The `esp32_config.json` file contains sensitive certificate data and is automatically added to `.gitignore`. Never commit this file to version control.

## Testing

### 1. Test Device Connection

Use AWS IoT Core Console → Test → MQTT test client:

**Subscribe to**: `device/basic-device/data`
**Publish to**: `device/basic-device/commands`

### 2. Test Device Shadow

**Subscribe to**: `$aws/things/basic-device/shadow/update/accepted`
**Publish to**: `$aws/things/basic-device/shadow/update`

### 3. Verify Certificate

```bash
# List certificates
aws iot list-certificates

# Describe the created certificate
aws iot describe-certificate --certificate-id $(aws iot list-certificates --query 'certificates[0].certificateId' --output text)
```

## Cleanup

```bash
# Destroy resources
terraform destroy
```

## File Structure

```plaintext
terraform/
├── main.tf              # Complete configuration (provider, module, outputs)
├── esp32_config.json    # Generated configuration (contains secrets - gitignored)
├── .gitignore          # Git ignore rules including esp32_config.json
└── README.md           # This documentation
```

**Note**: This configuration uses a single `main.tf` file for simplicity. The `esp32_config.json` file is automatically generated and contains sensitive data, so it's excluded from version control.
