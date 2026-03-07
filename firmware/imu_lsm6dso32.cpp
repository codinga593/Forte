#include "imu_lsm6dso32.h"

namespace {
constexpr uint8_t REG_WHO_AM_I = 0x0F;
constexpr uint8_t REG_CTRL1_XL = 0x10;
constexpr uint8_t REG_CTRL2_G = 0x11;
constexpr uint8_t REG_CTRL3_C = 0x12;
constexpr uint8_t REG_OUTX_L_G = 0x22;
constexpr uint8_t WHO_AM_I_VAL = 0x6C;

constexpr float ACCEL_32G_LSB = 32.0f / 32768.0f;
constexpr float GYRO_2000DPS_LSB = 2000.0f / 32768.0f;
}  // namespace

LSM6DSO32::LSM6DSO32(TwoWire &wire, uint8_t addr) : _wire(wire), _addr(addr) {}

bool LSM6DSO32::begin() {
  uint8_t who = 0;
  if (!readReg(REG_WHO_AM_I, who) || who != WHO_AM_I_VAL) {
    return false;
  }

  // Reboot + block data update + auto increment.
  if (!writeReg(REG_CTRL3_C, 0x44)) {
    return false;
  }
  delay(10);

  // Accel ODR 416 Hz, FS 32g.
  if (!writeReg(REG_CTRL1_XL, 0x7C)) {
    return false;
  }
  // Gyro ODR 416 Hz, FS 2000 dps.
  if (!writeReg(REG_CTRL2_G, 0x7C)) {
    return false;
  }
  return true;
}

bool LSM6DSO32::readAccelGyro(
  float &ax_g, float &ay_g, float &az_g,
  float &gx_dps, float &gy_dps, float &gz_dps
) {
  uint8_t raw[12] {};
  if (!readRegs(REG_OUTX_L_G, raw, sizeof(raw))) {
    return false;
  }

  const int16_t gx = static_cast<int16_t>((raw[1] << 8) | raw[0]);
  const int16_t gy = static_cast<int16_t>((raw[3] << 8) | raw[2]);
  const int16_t gz = static_cast<int16_t>((raw[5] << 8) | raw[4]);
  const int16_t ax = static_cast<int16_t>((raw[7] << 8) | raw[6]);
  const int16_t ay = static_cast<int16_t>((raw[9] << 8) | raw[8]);
  const int16_t az = static_cast<int16_t>((raw[11] << 8) | raw[10]);

  gx_dps = gx * GYRO_2000DPS_LSB;
  gy_dps = gy * GYRO_2000DPS_LSB;
  gz_dps = gz * GYRO_2000DPS_LSB;
  ax_g = ax * ACCEL_32G_LSB;
  ay_g = ay * ACCEL_32G_LSB;
  az_g = az * ACCEL_32G_LSB;
  return true;
}

bool LSM6DSO32::writeReg(uint8_t reg, uint8_t val) {
  _wire.beginTransmission(_addr);
  _wire.write(reg);
  _wire.write(val);
  return _wire.endTransmission() == 0;
}

bool LSM6DSO32::readReg(uint8_t reg, uint8_t &val) {
  if (!readRegs(reg, &val, 1)) {
    return false;
  }
  return true;
}

bool LSM6DSO32::readRegs(uint8_t reg, uint8_t *buf, size_t len) {
  _wire.beginTransmission(_addr);
  _wire.write(reg);
  if (_wire.endTransmission(false) != 0) {
    return false;
  }
  size_t r = _wire.requestFrom(_addr, static_cast<uint8_t>(len));
  if (r != len) {
    return false;
  }
  for (size_t i = 0; i < len; i++) {
    buf[i] = _wire.read();
  }
  return true;
}

