#include "Frontlight.h"
#include "esp_err.h"
#include "esp_log.h"

static const char *TAG = "FRONTLIGHT";

/**
 * ============================================================
 * Public functions
 * ============================================================
 */

/**
 * @brief  Register the Frontlight on the I2C bus.
 *
 */
esp_err_t Frontlight::begin(I2C &i2c, PCAL &expander)
{
  m_expander = &expander;
  return i2c.addDevice(FRONTLIGHT_I2C_ADDR, &m_devHandle);
}

esp_err_t Frontlight::setBrightness(uint8_t value)
{
  uint8_t buf[2] = {0, (uint8_t)(63 - (value & 0x3F))};
  return i2c_master_transmit(m_devHandle, buf, sizeof(buf), -1);
}



void Frontlight::setState(bool enable)
{
  ESP_LOGI(TAG, "Set state");
  m_expander->setLevel(FRONTLIGHT_EN_PIN, enable ? 1 : 0, true);
}

