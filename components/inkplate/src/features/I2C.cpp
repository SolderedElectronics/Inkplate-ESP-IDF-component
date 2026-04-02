#include "driver/i2c_master.h"
#include "esp_log.h"

#include "I2C.h"

static const char *TAG = "I2C";

I2C::I2C()
{
  i2c_master_bus_config_t bus_config  = {};
  bus_config.i2c_port                 = I2C_NUM_0;
  bus_config.sda_io_num               = I2C_SDA;
  bus_config.scl_io_num               = I2C_SCL;
  bus_config.clk_source               = I2C_CLK_SRC_DEFAULT;
  bus_config.glitch_ignore_cnt        = 7;
  bus_config.flags.enable_internal_pullup = true;

  ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &m_busHandle));
  ESP_LOGI(TAG, "I2C bus initialized");
}
