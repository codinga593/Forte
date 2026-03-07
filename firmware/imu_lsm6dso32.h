#pragma once

#include <Arduino.h>
#include <Wire.h>

class LSM6DSO32 {
public:
  LSM6DSO32(TwoWire &wire, uint8_t addr);
  bool begin();
  bool readAccelGyro(float &ax_g, float &ay_g, float &az_g, float &gx_dps, float &gy_dps, float &gz_dps);

private:
  TwoWire &_wire;
  uint8_t _addr;

  bool writeReg(uint8_t reg, uint8_t val);
  bool readReg(uint8_t reg, uint8_t &val);
  bool readRegs(uint8_t reg, uint8_t *buf, size_t len);
};

