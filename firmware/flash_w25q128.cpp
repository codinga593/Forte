#include "flash_w25q128.h"

namespace {
constexpr uint8_t CMD_JEDEC_ID = 0x9F;
constexpr uint8_t CMD_READ_STATUS1 = 0x05;
constexpr uint8_t CMD_WRITE_ENABLE = 0x06;
constexpr uint8_t CMD_READ_DATA = 0x03;
constexpr uint8_t CMD_PAGE_PROGRAM = 0x02;
constexpr uint8_t CMD_SECTOR_ERASE_4K = 0x20;

constexpr uint32_t FLASH_SIZE_BYTES = 16UL * 1024UL * 1024UL;
constexpr uint32_t SECTOR_SIZE = 4096UL;
constexpr uint32_t PAGE_SIZE = 256UL;
}  // namespace

W25Q128::W25Q128(SPIClass &spi, uint8_t csPin) : _spi(spi), _csPin(csPin) {}

bool W25Q128::begin() {
  pinMode(_csPin, OUTPUT);
  deselect();
  _spi.begin();

  select();
  _spi.transfer(CMD_JEDEC_ID);
  const uint8_t manufacturer = _spi.transfer(0x00);
  const uint8_t memType = _spi.transfer(0x00);
  const uint8_t capacity = _spi.transfer(0x00);
  deselect();

  // Winbond W25Q128JV -> EF 40 18
  if (manufacturer != 0xEF || memType != 0x40 || capacity != 0x18) {
    return false;
  }

  _writeAddress = 0;
  _lastErasedSector = 0xFFFFFFFFu;
  return true;
}

bool W25Q128::append(const uint8_t *data, size_t len) {
  if (!data || len == 0) {
    return true;
  }
  if ((_writeAddress + len) > FLASH_SIZE_BYTES) {
    return false;
  }

  size_t offset = 0;
  while (offset < len) {
    const uint32_t sector = _writeAddress / SECTOR_SIZE;
    if (sector != _lastErasedSector) {
      if (!eraseSector(sector * SECTOR_SIZE)) {
        return false;
      }
      _lastErasedSector = sector;
    }

    const uint32_t pageOffset = _writeAddress % PAGE_SIZE;
    const size_t chunk = min(static_cast<size_t>(PAGE_SIZE - pageOffset), len - offset);
    if (!pageProgram(_writeAddress, data + offset, chunk)) {
      return false;
    }
    _writeAddress += chunk;
    offset += chunk;
  }
  return true;
}

bool W25Q128::read(uint32_t address, uint8_t *buf, size_t len) {
  if (!buf || (address + len) > FLASH_SIZE_BYTES) {
    return false;
  }
  select();
  _spi.transfer(CMD_READ_DATA);
  _spi.transfer((address >> 16) & 0xFF);
  _spi.transfer((address >> 8) & 0xFF);
  _spi.transfer(address & 0xFF);
  for (size_t i = 0; i < len; i++) {
    buf[i] = _spi.transfer(0x00);
  }
  deselect();
  return true;
}

uint32_t W25Q128::bytesWritten() const {
  return _writeAddress;
}

void W25Q128::select() {
  digitalWrite(_csPin, LOW);
}

void W25Q128::deselect() {
  digitalWrite(_csPin, HIGH);
}

void W25Q128::writeEnable() {
  select();
  _spi.transfer(CMD_WRITE_ENABLE);
  deselect();
}

uint8_t W25Q128::readStatus1() {
  select();
  _spi.transfer(CMD_READ_STATUS1);
  uint8_t s = _spi.transfer(0x00);
  deselect();
  return s;
}

void W25Q128::waitBusy(uint32_t timeoutMs) {
  const uint32_t start = millis();
  while ((readStatus1() & 0x01) != 0) {
    if ((millis() - start) > timeoutMs) {
      break;
    }
    delay(1);
  }
}

bool W25Q128::eraseSector(uint32_t address) {
  writeEnable();
  select();
  _spi.transfer(CMD_SECTOR_ERASE_4K);
  _spi.transfer((address >> 16) & 0xFF);
  _spi.transfer((address >> 8) & 0xFF);
  _spi.transfer(address & 0xFF);
  deselect();
  waitBusy(450);
  return true;
}

bool W25Q128::pageProgram(uint32_t address, const uint8_t *data, size_t len) {
  if (len == 0 || len > PAGE_SIZE) {
    return false;
  }
  writeEnable();
  select();
  _spi.transfer(CMD_PAGE_PROGRAM);
  _spi.transfer((address >> 16) & 0xFF);
  _spi.transfer((address >> 8) & 0xFF);
  _spi.transfer(address & 0xFF);
  for (size_t i = 0; i < len; i++) {
    _spi.transfer(data[i]);
  }
  deselect();
  waitBusy(10);
  return true;
}

