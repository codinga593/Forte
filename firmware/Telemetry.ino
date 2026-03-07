#include <Arduino.h>
#include <math.h>
#include <SPI.h>
#include <Wire.h>

#include "baro_ms5611.h"
#include "config_pins.h"
#include "data_types.h"
#include "flash_w25q128.h"
#include "gps_parser.h"
#include "imu_lsm6dso32.h"
#include "lora_sx1262.h"

#if __has_include(<SD.h>)
#include <SD.h>
#define HAS_SD_LIB 1
#else
#define HAS_SD_LIB 0
#endif

namespace {
LSM6DSO32 imu(IMU_I2C_BUS, IMU_I2C_ADDR);
MS5611 baro(BARO_I2C_BUS, MS5611_I2C_ADDR);
W25Q128 flashLog(FLASH_SPI_BUS, FLASH_CS_PIN);
Sx1262Radio lora(
  LORA_SPI_BUS,
  LORA_NSS_PIN,
  LORA_BUSY_PIN,
  LORA_DIO1_PIN,
  LORA_RESET_PIN
);
NmeaParser gps;

FlightSample sample {};
uint32_t flightStartMs = 0;
uint32_t lastLoopMs = 0;
uint32_t lastLoraTxMs = 0;
bool armed = false;
bool inFlight = false;
bool landed = false;
bool sdDumpDone = false;

float seaLevelPressureMbar = 1013.25f;
float prevAltitudeM = 0.0f;
float verticalSpeedMps = 0.0f;
float verticalAccelMps2 = 0.0f;
float prevVerticalSpeedMps = 0.0f;

uint16_t checksum16(const uint8_t *data, size_t len) {
  uint32_t sum = 0;
  for (size_t i = 0; i < len; i++) {
    sum += data[i];
  }
  return static_cast<uint16_t>(sum & 0xFFFFu);
}

float pressureToAltitudeM(float pressureMbar) {
  if (pressureMbar <= 0.0f) {
    return 0.0f;
  }
  return 44330.0f * (1.0f - powf(pressureMbar / seaLevelPressureMbar, 0.19029495f));
}

void readGpsStream() {
  while (GPS_UART.available()) {
    const char c = static_cast<char>(GPS_UART.read());
    gps.feed(c);
  }
}

bool detectLiftOff(const FlightSample &s) {
  const bool accelTrigger = fabsf(s.ax_g) > 2.2f || fabsf(s.ay_g) > 2.2f || fabsf(s.az_g) > 2.2f;
  const bool climbTrigger = s.altitude_m > 15.0f && verticalSpeedMps > 5.0f;
  return accelTrigger || climbTrigger;
}

bool detectLanding(const FlightSample &s, uint32_t nowMs) {
  if (!inFlight) {
    return false;
  }

  const bool nearGround = s.altitude_m < 8.0f;
  const bool lowVSpeed = fabsf(verticalSpeedMps) < 0.6f;
  const bool lowAccel = fabsf(s.ax_g) < 0.35f && fabsf(s.ay_g) < 0.35f && fabsf(s.az_g - 1.0f) < 0.35f;
  const bool minFlightTime = (nowMs - flightStartMs) > MIN_FLIGHT_TIME_MS;

  return minFlightTime && nearGround && lowVSpeed && lowAccel;
}

void fillSample(uint32_t nowMs) {
  sample.magic = SAMPLE_MAGIC;
  sample.ms = nowMs;
  sample.state = landed ? FlightState::LANDED : (inFlight ? FlightState::ASCENT_DESCENT : FlightState::ARMED_IDLE);

  imu.readAccelGyro(
    sample.ax_g, sample.ay_g, sample.az_g,
    sample.gx_dps, sample.gy_dps, sample.gz_dps
  );

  sample.temperature_c = baro.readTemperatureC();
  sample.pressure_mbar = baro.readPressureMbar();
  sample.altitude_m = pressureToAltitudeM(sample.pressure_mbar);

  const float dt = (lastLoopMs == 0) ? (LOOP_PERIOD_MS / 1000.0f) : (nowMs - lastLoopMs) / 1000.0f;
  verticalSpeedMps = (dt > 0.001f) ? ((sample.altitude_m - prevAltitudeM) / dt) : 0.0f;
  verticalAccelMps2 = (dt > 0.001f) ? ((verticalSpeedMps - prevVerticalSpeedMps) / dt) : 0.0f;
  prevAltitudeM = sample.altitude_m;
  prevVerticalSpeedMps = verticalSpeedMps;

  sample.vspeed_mps = verticalSpeedMps;
  sample.vaccel_mps2 = verticalAccelMps2;

  gps.copyTo(sample.gps);

  sample.checksum = checksum16(
    reinterpret_cast<const uint8_t *>(&sample),
    sizeof(FlightSample) - sizeof(sample.checksum)
  );
}

void sendLoraStatus(uint32_t nowMs) {
  if ((nowMs - lastLoraTxMs) < LORA_TX_INTERVAL_MS) {
    return;
  }
  lastLoraTxMs = nowMs;

  char packet[180];
  snprintf(
    packet,
    sizeof(packet),
    "$TLM,%lu,%u,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%.6f,%.6f,%.1f,%u",
    static_cast<unsigned long>(sample.ms),
    static_cast<unsigned int>(sample.state),
    sample.altitude_m,
    sample.vspeed_mps,
    sample.ax_g,
    sample.ay_g,
    sample.az_g,
    sample.gx_dps,
    sample.gy_dps,
    sample.gz_dps,
    sample.gps.fix,
    sample.gps.lat_deg,
    sample.gps.lon_deg,
    sample.gps.alt_m,
    sample.gps.sats
  );
  lora.transmit(packet);
}

void writeSampleToFlash() {
  flashLog.append(
    reinterpret_cast<const uint8_t *>(&sample),
    sizeof(sample)
  );
}

void dumpFlashToSdCard() {
  if (sdDumpDone) {
    return;
  }
#if HAS_SD_LIB
  if (!SD.begin()) {
    Serial.println("SD init failed, will retry");
    return;
  }

  File f = SD.open("/flight_log.csv", FILE_WRITE);
  if (!f) {
    Serial.println("SD open failed");
    return;
  }

  f.println(
    "ms,state,ax_g,ay_g,az_g,gx_dps,gy_dps,gz_dps,temp_c,pressure_mbar,alt_m,vspeed_mps,vaccel_mps2,"
    "gps_fix,lat,lon,gps_alt_m,gps_speed_mps,gps_course_deg,sats"
  );

  FlightSample r {};
  uint32_t addr = 0;
  const uint32_t end = flashLog.bytesWritten();
  while (addr + sizeof(FlightSample) <= end) {
    flashLog.read(addr, reinterpret_cast<uint8_t *>(&r), sizeof(r));
    addr += sizeof(r);
    const uint16_t cs = checksum16(
      reinterpret_cast<const uint8_t *>(&r),
      sizeof(r) - sizeof(r.checksum)
    );
    if (r.magic != SAMPLE_MAGIC || cs != r.checksum) {
      continue;
    }
    char line[240];
    snprintf(
      line, sizeof(line),
      "%lu,%u,%.4f,%.4f,%.4f,%.3f,%.3f,%.3f,%.2f,%.2f,%.2f,%.3f,%.3f,%d,%.7f,%.7f,%.2f,%.2f,%.2f,%u",
      static_cast<unsigned long>(r.ms),
      static_cast<unsigned int>(r.state),
      r.ax_g, r.ay_g, r.az_g,
      r.gx_dps, r.gy_dps, r.gz_dps,
      r.temperature_c, r.pressure_mbar, r.altitude_m,
      r.vspeed_mps, r.vaccel_mps2,
      r.gps.fix, r.gps.lat_deg, r.gps.lon_deg,
      r.gps.alt_m, r.gps.speed_mps, r.gps.course_deg,
      r.gps.sats
    );
    f.println(line);
  }

  f.flush();
  f.close();
  sdDumpDone = true;
  Serial.println("Flash->SD transfer complete");
#else
  Serial.println("SD library not found in core; cannot export yet");
#endif
}
}  // namespace

void setup() {
  Serial.begin(DEBUG_BAUD);
  delay(250);
  Serial.println("Telemetry firmware boot");

  BARO_I2C_BUS.begin();
  IMU_I2C_BUS.begin();
  GPS_UART.begin(GPS_BAUD);

  if (!imu.begin()) {
    Serial.println("IMU init failed");
  }
  if (!baro.begin()) {
    Serial.println("MS5611 init failed");
  }
  if (!flashLog.begin()) {
    Serial.println("Flash init failed");
  }
  if (!lora.begin()) {
    Serial.println("LoRa init failed");
  }

  const float p0 = baro.readPressureMbar();
  if (p0 > 100.0f && p0 < 1200.0f) {
    seaLevelPressureMbar = p0;
  }
  Serial.print("Sea-level baseline mbar: ");
  Serial.println(seaLevelPressureMbar, 2);
}

void loop() {
  const uint32_t nowMs = millis();
  readGpsStream();

  if ((nowMs - lastLoopMs) < LOOP_PERIOD_MS) {
    return;
  }

  fillSample(nowMs);

  if (!armed && (nowMs > 3000 || sample.gps.fix)) {
    armed = true;
  }
  if (!inFlight && armed && detectLiftOff(sample)) {
    inFlight = true;
    flightStartMs = nowMs;
    Serial.println("Liftoff detected");
  }
  if (!landed && detectLanding(sample, nowMs)) {
    landed = true;
    Serial.println("Landing detected");
  }

  writeSampleToFlash();
  sendLoraStatus(nowMs);

  if (landed) {
    dumpFlashToSdCard();
  }

  lastLoopMs = nowMs;
}
