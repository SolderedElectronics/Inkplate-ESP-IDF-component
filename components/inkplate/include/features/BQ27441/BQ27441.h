/**
 **************************************************
 *
 * @file        BQ27441-G1-SOLDERED.h
 * @brief       Soldered BQ27441 Arduino Library.
 *
 *
 * @copyright   GNU General Public License v3.0
 * @authors     @ soldered.com, Robert Peric
 ***************************************************/

#ifndef _BQ27441_G1_H
#define _BQ27441_G1_H

#include "driver/i2c_master.h"
#include "libs/BQ27441Driver.h"

class BQ27441 : public BQ27441Driver
{
  public:
    bool begin(i2c_master_bus_handle_t bus_handle, uint32_t speed = 100000);
};

#endif
