#pragma once

#include <Arduino.h>
#include <SPI.h>

class W25Q128 {
public:
  W25Q128(SPIClass &spi, uint8_t csPin);
  bool begin();

  bool append(const uint8_t *data, size_t len);
  bool read(uint32_t address, uint8_t *buf, size_t len);

  uint32_t bytesWritten() const;

private:
  SPIClass &_spi;
  uint8_t _csPin;
  uint32_t _writeAddress = 0;
  uint32_t _lastErasedSector = 0xFFFFFFFFu;

  void select();
  void deselect();
  void writeEnable();
  void waitBusy(uint32_t timeoutMs = 2000);
  uint8_t readStatus1();
  bool eraseSector(uint32_t address);
  bool pageProgram(uint32_t address, const uint8_t *data, size_t len);
};

