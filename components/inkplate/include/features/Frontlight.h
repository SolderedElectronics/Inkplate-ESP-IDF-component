#ifndef FRONTLIGHT_H
#define FRONTLIGHT_H

#include "esp_err.h"
#include <stdint.h>

#include "I2C.h"
#include "PCAL.h"

#define FRONTLIGHT_I2C_ADDR  0x5C >> 1

class Frontlight
{
public:
  Frontlight() = default;
  esp_err_t begin(I2C &i2c, PCAL &expander, IOPin_t pin);

  esp_err_t setBrightness(uint8_t value);
  void      setState(bool enable);

private:
  PCAL                   *m_expander  = nullptr;
  IOPin_t                 m_pin;
  i2c_master_dev_handle_t m_devHandle = NULL;
};

#endif
