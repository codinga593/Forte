#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>

// ===== Bus assignment =====
// STM32F405 typically exposes Wire (I2C1) and Wire1 (I2C2) in STM32Duino.
#if defined(WIRE_INTERFACES_COUNT) && (WIRE_INTERFACES_COUNT > 1)
#define IMU_I2C_BUS Wire1
#else
#define IMU_I2C_BUS Wire
#endif

#define BARO_I2C_BUS Wire

// GPS on UART4 RX path (module TX -> MCU RX). In STM32Duino, Serial4 maps to UART4.
#define GPS_UART Serial4

// SPI buses (adjust if your board variant names differ).
#define FLASH_SPI_BUS SPI
#define LORA_SPI_BUS SPI

// ===== Addresses =====
static constexpr uint8_t IMU_I2C_ADDR = 0x6A;
static constexpr uint8_t MS5611_I2C_ADDR = 0x77;

// ===== W25Q128 pins =====
// Replace these with your PCB net mapping.
static constexpr uint8_t FLASH_CS_PIN = PB0;

// ===== SX1262 pins =====
// Replace these with your PCB net mapping.
static constexpr uint8_t LORA_NSS_PIN = PB12;
static constexpr uint8_t LORA_BUSY_PIN = PB1;
static constexpr uint8_t LORA_DIO1_PIN = PB2;
static constexpr uint8_t LORA_RESET_PIN = PB10;

// ===== Runtime timing =====
static constexpr uint32_t DEBUG_BAUD = 115200;
static constexpr uint32_t GPS_BAUD = 9600;
static constexpr uint32_t LOOP_PERIOD_MS = 20;       // 50Hz sample loop
static constexpr uint32_t LORA_TX_INTERVAL_MS = 200; // 5Hz telemetry
static constexpr uint32_t MIN_FLIGHT_TIME_MS = 6000;
static constexpr float LORA_FREQ_MHZ = 915.0f;       // US915 default
