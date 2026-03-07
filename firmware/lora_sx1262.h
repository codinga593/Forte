#pragma once

#include <Arduino.h>
#include <SPI.h>

class Sx1262Radio {
public:
  Sx1262Radio(SPIClass &spi, uint8_t nss, uint8_t busy, uint8_t dio1, uint8_t rst);
  bool begin();
  bool transmit(const char *payload);

private:
  SPIClass &_spi;
  uint8_t _nss;
  uint8_t _busy;
  uint8_t _dio1;
  uint8_t _rst;
  bool _ready = false;
};

