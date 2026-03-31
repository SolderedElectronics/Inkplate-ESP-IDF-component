#ifndef _I2S_H_
#define _I2S_H_

#include "esp_rom_lldesc.h"
#include "soc/i2s_struct.h"
#include "esp_attr.h"

class I2S
{
public:
  IRAM_ATTR I2S(uint8_t clockDivider = 5);

  void IRAM_ATTR      sendDataI2S();
  void IRAM_ATTR      setI2S1pin(uint32_t pin, uint32_t function, uint32_t inv);

protected:
  volatile uint8_t*   m_dmaLineBuffer;
  volatile lldesc_s*  m_dmaI2SDesc;

  // use only I2S1 (I2S0 is not compatible with 8 bit data).
  volatile i2s_dev_t* m_i2s;

};

#endif
