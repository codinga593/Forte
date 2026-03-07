#pragma once

#include <Arduino.h>

#include "data_types.h"

class NmeaParser {
public:
  void feed(char c);
  void copyTo(GpsSnapshot &out) const;

private:
  char _line[120] {};
  uint8_t _idx = 0;
  GpsSnapshot _latest {};

  static float toDecimalDegrees(const char *val, const char hemi);
  static bool parseFloatField(const char *s, float &out);
  static bool parseUintField(const char *s, uint8_t &out);
  void parseSentence(char *line);
  void parseGga(char **f, int n);
  void parseRmc(char **f, int n);
};

