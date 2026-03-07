#include "gps_parser.h"

#include <stdlib.h>
#include <string.h>

void NmeaParser::feed(char c) {
  if (c == '\r') {
    return;
  }
  if (c == '\n') {
    _line[_idx] = '\0';
    if (_idx > 6 && _line[0] == '$') {
      parseSentence(_line);
    }
    _idx = 0;
    return;
  }
  if (_idx < (sizeof(_line) - 1)) {
    _line[_idx++] = c;
  } else {
    _idx = 0;
  }
}

void NmeaParser::copyTo(GpsSnapshot &out) const {
  out = _latest;
}

float NmeaParser::toDecimalDegrees(const char *val, const char hemi) {
  if (!val || !*val) {
    return 0.0f;
  }
  const float raw = static_cast<float>(atof(val));
  const int deg = static_cast<int>(raw / 100.0f);
  const float minutes = raw - (deg * 100.0f);
  float dec = deg + (minutes / 60.0f);
  if (hemi == 'S' || hemi == 'W') {
    dec = -dec;
  }
  return dec;
}

bool NmeaParser::parseFloatField(const char *s, float &out) {
  if (!s || !*s) {
    return false;
  }
  out = static_cast<float>(atof(s));
  return true;
}

bool NmeaParser::parseUintField(const char *s, uint8_t &out) {
  if (!s || !*s) {
    return false;
  }
  out = static_cast<uint8_t>(atoi(s));
  return true;
}

void NmeaParser::parseSentence(char *line) {
  char *star = strchr(line, '*');
  if (star) {
    *star = '\0';
  }

  char *fields[20] {};
  int n = 0;
  char *ctx = nullptr;
  char *tok = strtok_r(line, ",", &ctx);
  while (tok && n < 20) {
    fields[n++] = tok;
    tok = strtok_r(nullptr, ",", &ctx);
  }
  if (n < 1) {
    return;
  }

  if (strstr(fields[0], "GGA")) {
    parseGga(fields, n);
  } else if (strstr(fields[0], "RMC")) {
    parseRmc(fields, n);
  }
}

void NmeaParser::parseGga(char **f, int n) {
  if (n < 10) {
    return;
  }

  uint8_t fix = 0;
  if (parseUintField(f[6], fix)) {
    _latest.fix = fix > 0 ? 1 : 0;
  }
  parseUintField(f[7], _latest.sats);
  _latest.lat_deg = toDecimalDegrees(f[2], f[3] ? f[3][0] : 'N');
  _latest.lon_deg = toDecimalDegrees(f[4], f[5] ? f[5][0] : 'E');
  parseFloatField(f[9], _latest.alt_m);
}

void NmeaParser::parseRmc(char **f, int n) {
  if (n < 9) {
    return;
  }

  _latest.fix = (f[2] && f[2][0] == 'A') ? 1 : 0;
  _latest.lat_deg = toDecimalDegrees(f[3], f[4] ? f[4][0] : 'N');
  _latest.lon_deg = toDecimalDegrees(f[5], f[6] ? f[6][0] : 'E');

  float speedKnots = 0.0f;
  if (parseFloatField(f[7], speedKnots)) {
    _latest.speed_mps = speedKnots * 0.514444f;
  }
  parseFloatField(f[8], _latest.course_deg);
}

