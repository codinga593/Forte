#pragma once

#include <Arduino.h>

static constexpr uint32_t SAMPLE_MAGIC = 0x544C4D31UL; // TLM1

enum class FlightState : uint8_t {
  ARMED_IDLE = 0,
  ASCENT_DESCENT = 1,
  LANDED = 2
};

struct GpsSnapshot {
  uint8_t fix = 0;
  uint8_t sats = 0;
  float lat_deg = 0.0f;
  float lon_deg = 0.0f;
  float alt_m = 0.0f;
  float speed_mps = 0.0f;
  float course_deg = 0.0f;
};

struct FlightSample {
  uint32_t magic = SAMPLE_MAGIC;
  uint32_t ms = 0;
  FlightState state = FlightState::ARMED_IDLE;

  float ax_g = 0.0f;
  float ay_g = 0.0f;
  float az_g = 0.0f;
  float gx_dps = 0.0f;
  float gy_dps = 0.0f;
  float gz_dps = 0.0f;

  float temperature_c = 0.0f;
  float pressure_mbar = 0.0f;
  float altitude_m = 0.0f;
  float vspeed_mps = 0.0f;
  float vaccel_mps2 = 0.0f;

  GpsSnapshot gps {};

  uint16_t checksum = 0;
} __attribute__((packed));

