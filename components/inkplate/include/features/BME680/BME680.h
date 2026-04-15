#ifndef BME680_H
#define BME680_H

#include "libs/BME680Driver.h"
#include "driver/i2c_master.h"

class BME680 : public BME680Driver
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
