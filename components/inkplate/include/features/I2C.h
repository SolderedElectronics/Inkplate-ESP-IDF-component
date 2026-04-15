#ifndef I2C_H
#define I2C_H

#include "driver/i2c_master.h"

#define I2C_SDA GPIO_NUM_21
#define I2C_SCL GPIO_NUM_22

class I2C
{
public:
  I2C();

  esp_err_t               addDevice(uint8_t addr, i2c_master_dev_handle_t *handle);
  i2c_master_bus_handle_t getBusHandle() { return m_busHandle; }

private:
  i2c_master_bus_handle_t m_busHandle;
};

#endif
