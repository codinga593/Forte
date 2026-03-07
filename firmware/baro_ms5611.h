#pragma once

#include <Arduino.h>
#include <Wire.h>

class MS5611 {
public:
  MS5611(TwoWire &wire, uint8_t addr);
  bool begin();

  float readTemperatureC();
  float readPressureMbar();

private:
  TwoWire &_wire;
  uint8_t _addr;
  uint16_t _cal[7] {};
  int32_t _lastTempCx100 = 0;
  int32_t _lastPressPa = 0;

  bool readProm();
  bool readAdc(uint8_t cmd, uint32_t &value);
  bool compute();
};

