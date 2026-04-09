#ifndef __LSM6DS3_SOLDERED_H__
#define __LSM6DS3_SOLDERED_H__

#include "libs/SparkFunLSM6DS3.h"
#include "driver/i2c_master.h"
#include "esp_err.h"

class Soldered_LSM6DS3 : public LSM6DS3
{
  public:
    Soldered_LSM6DS3() = default;

    // Initializes the I2C bus and device, then configures the sensor.
    // Call once before reading data.
    status_t begin(i2c_master_bus_handle_t bus_handle, uint8_t addr = 0x6B);

  private:
    i2c_master_dev_handle_t dev_handle;
};

#endif
