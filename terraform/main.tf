# Basic IoT Thing Example
# Creates a single IoT thing with certificate and inline policy

terraform {
  required_version = ">= 1.0.0"
  required_providers {
    aws = {
      source  = "hashicorp/aws"
      version = ">= 6.0.0"
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
        Effect = "Allow"
        Action = [
          "iot:Connect"
        ]
        Resource = "arn:aws:iot:${data.aws_region.current.id}:${data.aws_caller_identity.current.account_id}:client/${local.thing_name}"
        Condition = {
          Bool = {
            "iot:Connection.Thing.IsAttached" = "true"
          }
        }
      },
      {
        Effect = "Allow"
        Action = [
          "iot:Publish"
        ]
        Resource = [
          "arn:aws:iot:${data.aws_region.current.id}:${data.aws_caller_identity.current.account_id}:topic/device/${local.thing_name}/*",
          "arn:aws:iot:${data.aws_region.current.id}:${data.aws_caller_identity.current.account_id}:topic/$${aws}/things/${local.thing_name}/shadow/*"
        ]
      },
      {
        Effect = "Allow"
        Action = [
          "iot:Subscribe"
        ]
        Resource = [
          "arn:aws:iot:${data.aws_region.current.id}:${data.aws_caller_identity.current.account_id}:topicfilter/device/${local.thing_name}/*",
          "arn:aws:iot:${data.aws_region.current.id}:${data.aws_caller_identity.current.account_id}:topicfilter/$${aws}/things/${local.thing_name}/shadow/*"
        ]
      },
      {
        Effect = "Allow"
        Action = [
          "iot:Receive"
        ]
        Resource = [
          "arn:aws:iot:${data.aws_region.current.id}:${data.aws_caller_identity.current.account_id}:topic/device/${local.thing_name}/*",
          "arn:aws:iot:${data.aws_region.current.id}:${data.aws_caller_identity.current.account_id}:topic/$${aws}/things/${local.thing_name}/shadow/*"
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
