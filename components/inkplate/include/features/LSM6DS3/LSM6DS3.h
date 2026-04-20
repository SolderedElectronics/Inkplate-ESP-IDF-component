#ifndef LSM6DS3_H
#define LSM6DS3_H

#include "libs/LSM6DS3Driver.h"
#include "driver/i2c_master.h"
#include "esp_err.h"

class LSM6DS3 : public LSM6DS3Driver
{
public:
  LSM6DS3() = default;

  status_t begin(i2c_master_bus_handle_t bus_handle, uint8_t addr = 0x6B);

private:
  i2c_master_dev_handle_t dev_handle;
};

#endif
