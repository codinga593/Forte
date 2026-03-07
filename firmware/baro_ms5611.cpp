#include "baro_ms5611.h"

namespace {
constexpr uint8_t CMD_RESET = 0x1E;
constexpr uint8_t CMD_ADC_READ = 0x00;
constexpr uint8_t CMD_CONV_D1_OSR4096 = 0x48;
constexpr uint8_t CMD_CONV_D2_OSR4096 = 0x58;
constexpr uint8_t CMD_PROM_READ_BASE = 0xA0;
}  // namespace

MS5611::MS5611(TwoWire &wire, uint8_t addr) : _wire(wire), _addr(addr) {}

bool MS5611::begin() {
  _wire.beginTransmission(_addr);
  _wire.write(CMD_RESET);
  if (_wire.endTransmission() != 0) {
    return false;
  }
  delay(10);
  if (!readProm()) {
    return false;
  }
  return compute();
}

float MS5611::readTemperatureC() {
  if (compute()) {
    return _lastTempCx100 / 100.0f;
  }
  return _lastTempCx100 / 100.0f;
}

float MS5611::readPressureMbar() {
  if (compute()) {
    return _lastPressPa / 100.0f;
  }
  return _lastPressPa / 100.0f;
}

bool MS5611::readProm() {
  for (uint8_t i = 1; i <= 6; i++) {
    _wire.beginTransmission(_addr);
    _wire.write(static_cast<uint8_t>(CMD_PROM_READ_BASE + (i * 2)));
    if (_wire.endTransmission(false) != 0) {
      return false;
    }
    if (_wire.requestFrom(_addr, static_cast<uint8_t>(2)) != 2) {
      return false;
    }
    _cal[i] = static_cast<uint16_t>((_wire.read() << 8) | _wire.read());
  }
  return true;
}

bool MS5611::readAdc(uint8_t cmd, uint32_t &value) {
  _wire.beginTransmission(_addr);
  _wire.write(cmd);
  if (_wire.endTransmission() != 0) {
    return false;
  }
  delay(10);

  _wire.beginTransmission(_addr);
  _wire.write(CMD_ADC_READ);
  if (_wire.endTransmission(false) != 0) {
    return false;
  }

  if (_wire.requestFrom(_addr, static_cast<uint8_t>(3)) != 3) {
    return false;
  }

  value = (static_cast<uint32_t>(_wire.read()) << 16);
  value |= (static_cast<uint32_t>(_wire.read()) << 8);
  value |= static_cast<uint32_t>(_wire.read());
  return true;
}

bool MS5611::compute() {
  uint32_t d1 = 0;
  uint32_t d2 = 0;
  if (!readAdc(CMD_CONV_D1_OSR4096, d1) || !readAdc(CMD_CONV_D2_OSR4096, d2)) {
    return false;
  }

  const int32_t dT = static_cast<int32_t>(d2) - (static_cast<int32_t>(_cal[5]) << 8);
  int64_t temp = 2000LL + ((static_cast<int64_t>(dT) * _cal[6]) >> 23);
  int64_t off = (static_cast<int64_t>(_cal[2]) << 16) + ((static_cast<int64_t>(_cal[4]) * dT) >> 7);
  int64_t sens = (static_cast<int64_t>(_cal[1]) << 15) + ((static_cast<int64_t>(_cal[3]) * dT) >> 8);

  // Second-order compensation for low temperature.
  if (temp < 2000) {
    const int64_t t2 = (static_cast<int64_t>(dT) * dT) >> 31;
    int64_t off2 = 5LL * (temp - 2000) * (temp - 2000) / 2;
    int64_t sens2 = 5LL * (temp - 2000) * (temp - 2000) / 4;
    if (temp < -1500) {
      off2 += 7LL * (temp + 1500) * (temp + 1500);
      sens2 += 11LL * (temp + 1500) * (temp + 1500) / 2;
    }
    temp -= t2;
    off -= off2;
    sens -= sens2;
  }

  const int64_t p = (((static_cast<int64_t>(d1) * sens) >> 21) - off) >> 15;
  _lastTempCx100 = static_cast<int32_t>(temp);
  _lastPressPa = static_cast<int32_t>(p);
  return true;
}

