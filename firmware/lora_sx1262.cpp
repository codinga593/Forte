#include "lora_sx1262.h"
#include "config_pins.h"

#if __has_include(<RadioLib.h>)
#include <RadioLib.h>
#define HAS_RADIOLIB 1
#else
#define HAS_RADIOLIB 0
#endif

namespace {
#if HAS_RADIOLIB
SX1262 *gRadio = nullptr;
#endif
}  // namespace

Sx1262Radio::Sx1262Radio(SPIClass &spi, uint8_t nss, uint8_t busy, uint8_t dio1, uint8_t rst)
  : _spi(spi), _nss(nss), _busy(busy), _dio1(dio1), _rst(rst) {}

bool Sx1262Radio::begin() {
  pinMode(_nss, OUTPUT);
  pinMode(_busy, INPUT);
  pinMode(_dio1, INPUT);
  pinMode(_rst, OUTPUT);

  _spi.begin();

#if HAS_RADIOLIB
  static Module module(_nss, _dio1, _rst, _busy);
  static SX1262 radio(&module);
  gRadio = &radio;

  int16_t state = gRadio->begin();
  if (state == RADIOLIB_ERR_NONE) state = gRadio->setFrequency(LORA_FREQ_MHZ);
  if (state == RADIOLIB_ERR_NONE) state = gRadio->setBandwidth(125.0f);
  if (state == RADIOLIB_ERR_NONE) state = gRadio->setSpreadingFactor(9);
  if (state == RADIOLIB_ERR_NONE) state = gRadio->setCodingRate(7);
  if (state == RADIOLIB_ERR_NONE) state = gRadio->setSyncWord(0x12);
  if (state == RADIOLIB_ERR_NONE) state = gRadio->setOutputPower(20);

  _ready = state == RADIOLIB_ERR_NONE;
  return _ready;
#else
  // Build-time fallback if RadioLib is not installed.
  _ready = false;
  return false;
#endif
}

bool Sx1262Radio::transmit(const char *payload) {
  if (!_ready || !payload) {
    return false;
  }
#if HAS_RADIOLIB
  return gRadio->transmit(payload) == RADIOLIB_ERR_NONE;
#else
  (void)payload;
  return false;
#endif
}
