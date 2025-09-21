# Basic IoT Thing Example
# Creates a single IoT thing with certificate and inline policy

terraform {
  required_version = ">= 1.0.0"
  required_providers {
    aws = {
      source  = "hashicorp/aws"
      version = ">= 6.0.0"
    }
    local = {
      source  = "hashicorp/local"
      version = ">= 2.0.0"
    }
    http = {
      source  = "hashicorp/http"
      version = ">= 3.0.0"
    }
  }
}

provider "aws" {
  region = "ap-southeast-2"
}

# Local values
locals {
  thing_name = "basic-device"
}

# Data sources
data "aws_region" "current" {}
data "aws_caller_identity" "current" {}

# Download Amazon Root CA certificate from AWS
resource "local_file" "amazon_root_ca" {
  filename = "AmazonRootCA1.pem"
  content  = data.http.amazon_root_ca.response_body
}

data "http" "amazon_root_ca" {
  url = "https://www.amazontrust.com/repository/AmazonRootCA1.pem"

  request_headers = {
    Accept = "application/x-pem-file"
  }
}

module "iot" {
  source = "tfstack/iot-core/aws"

  thing_names = [local.thing_name]

  # Thing attributes for better organization
  thing_attributes = {
    deviceType  = "basic-device"
    version     = "1.0.0"
    environment = "dev"
  }

  # Basic IoT policy for MQTT communication
  policy_json = jsonencode({
    Version = "2012-10-17"
    Statement = [
      {
        Sid    = "AllowIoTConnect"
        Effect = "Allow"
        Action = [
          "iot:Connect"
        ]
        Resource = "arn:aws:iot:${data.aws_region.current.id}:${data.aws_caller_identity.current.account_id}:client/${local.thing_name}"
      },
      {
        Sid    = "AllowIoTSubscribe"
        Effect = "Allow"
        Action = [
          "iot:Subscribe"
        ]
        Resource = [
          "arn:aws:iot:${data.aws_region.current.id}:${data.aws_caller_identity.current.account_id}:topicfilter/${local.thing_name}/*"
        ]
      },
      {
        Sid    = "AllowIoTReceive"
        Effect = "Allow"
        Action = [
          "iot:Receive"
        ]
        Resource = [
          "arn:aws:iot:${data.aws_region.current.id}:${data.aws_caller_identity.current.account_id}:topic/esp32/sub"
        ]
      },
      {
        Sid    = "AllowIoTPublish"
        Effect = "Allow"
        Action = [
          "iot:Publish"
        ]
        Resource = [
          "arn:aws:iot:${data.aws_region.current.id}:${data.aws_caller_identity.current.account_id}:topic/${local.thing_name}/*"
        ]
      },
      {
        Sid    = "AllowIoTPublishRetain"
        Effect = "Allow"
        Action = [
          "iot:PublishRetain"
        ]
        Resource = [
          "arn:aws:iot:${data.aws_region.current.id}:${data.aws_caller_identity.current.account_id}:topic/${local.thing_name}/*"
        ]
      }
    ]
  })

  policy_name = "${local.thing_name}-policy"

  tags = {
    Environment = "dev"
    Project     = "iot-basic-example"
    DeviceType  = "basic-device"
  }
}

# Essential outputs for ESP32 configuration
output "thing_name" {
  description = "Name of the created IoT thing"
  value       = module.iot.thing_names[0]
}

output "certificate_pem" {
  description = "IoT certificate PEM (sensitive)"
  value       = module.iot.certificate_pems[0]
  sensitive   = true
}

output "private_key" {
  description = "IoT certificate private key (sensitive)"
  value       = module.iot.certificate_private_keys[0]
  sensitive   = true
}

output "endpoint" {
  description = "IoT endpoint"
  value       = module.iot.endpoint
}

# Generate ESP32 config file automatically
resource "local_file" "esp32_config" {
  filename = "esp32_config.json"
  content  = <<-EOT
{
  "certificate_pem": {
    "sensitive": true,
    "type": "string",
    "value": "${replace(module.iot.certificate_pems[0], "\n", "\\n")}"
  },
  "endpoint": {
    "sensitive": false,
    "type": "string",
    "value": "${module.iot.endpoint}"
  },
  "private_key": {
    "sensitive": true,
    "type": "string",
    "value": "${replace(module.iot.certificate_private_keys[0], "\n", "\\n")}"
  },
  "thing_name": {
    "sensitive": false,
    "type": "string",
    "value": "${local.thing_name}"
  }
}
EOT
}
