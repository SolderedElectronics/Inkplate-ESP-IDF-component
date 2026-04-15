#ifndef BQ27441_H
#define BQ27441_H

#include "driver/i2c_master.h"
#include "libs/BQ27441Driver.h"

class BQ27441 : public BQ27441Driver
{
  public:
    bool begin(i2c_master_bus_handle_t bus_handle, uint32_t speed = 100000);
};

#endif
