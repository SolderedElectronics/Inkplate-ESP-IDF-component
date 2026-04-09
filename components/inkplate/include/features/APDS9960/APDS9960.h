#ifndef __APDS9960_SOLDERED__
#define __APDS9960_SOLDERED__

#include "libs/APDS9960Driver.h"

class APDS9960 : public APDS9960Driver
{
  public:
    APDS9960() : APDS9960Driver() {}
    bool begin(i2c_master_bus_handle_t bus_handle);
};

#endif
