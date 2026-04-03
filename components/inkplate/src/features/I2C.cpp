#include "driver/i2c_master.h"
#include "esp_log.h"

#include "I2C.h"

static const char *TAG = "I2C";

/**
 * ============================================================
 * Public functions
 * ============================================================
 */

/**
 * @brief  I2C constructor.
 *
 * @note   Sets up the I2C bus.
 */
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

esp_err_t I2C::addDevice(uint8_t addr, i2c_master_dev_handle_t *handle)
{
  i2c_device_config_t dev_config  = {};
  dev_config.dev_addr_length      = I2C_ADDR_BIT_LEN_7;
  dev_config.device_address       = addr;
  dev_config.scl_speed_hz         = 400000;

  return i2c_master_bus_add_device(m_busHandle, &dev_config, handle);
}
