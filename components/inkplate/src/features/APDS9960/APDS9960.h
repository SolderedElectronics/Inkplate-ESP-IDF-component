#ifndef __APDS9960_SOLDERED__
#define __APDS9960_SOLDERED__

#include "libs/SparkFunAPDS9960.h"

class APDS9960 : public SparkFun_APDS9960
{
  public:
    APDS9960() : SparkFun_APDS9960() {}
    bool begin(i2c_master_bus_handle_t bus_handle);
};

#endif
