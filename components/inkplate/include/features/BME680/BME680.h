/**
 **************************************************
 *
 * @file        BME680-SOLDERED.h
 * @brief       Header file for BME680-SOLDERED board (ESP-IDF)
 *
 *
 * @copyright GNU General Public License v3.0
 * @authors     Zvonimir Haramustek for Soldered.com
 ***************************************************/

#ifndef __BME680_SOLDERED__
#define __BME680_SOLDERED__

#include "libs/ZanshinBME680.h"
#include "driver/i2c_master.h"

class BME680 : public BME680_Class
{
  public:
    bool begin(i2c_master_bus_handle_t bus_handle);

    float readTemperature();
    float readPressure();
    float readHumidity();
    float readAltitude();
    float readGasResistance();
    void  readSensorData(float &temp, float &humidity, float &pressure, float &gas);

    float calculateAltitude(float pressure);
};

#endif
