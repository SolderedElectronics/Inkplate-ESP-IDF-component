#include "TPS.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"

/**
 * @brief  Register the TPS65186 on the I2C bus.
 *
 * @param  i2c_master_bus_handle_t busHandle
 *         Handle to an already-initialised I2C master bus.
 *
 * @return esp_err_t
 *         ESP_OK on success
 *         ESP_ERR_INVALID_STATE if already registered or an I2C driver error code.
 */
esp_err_t TPS::begin(i2c_master_bus_handle_t busHandle)
{
  i2c_device_config_t cfg = {};
  cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
  cfg.device_address  = TPS_I2C_ADDR;
  cfg.scl_speed_hz    = 100000;
  return i2c_master_bus_add_device(busHandle, &cfg, &m_handle);
}

/**
 * @brief  Write startup power-up/down sequences to the PMIC (UPSEQ0/1, DWNSEQ0/1).
 *
 * @note   Should be called once after begin(), with WAKEUP asserted.
 */
void TPS::initSequences()
{
  uint8_t buf[5] = {0x09, 0b00011011, 0b00000000, 0b00011011, 0b00000000};
  i2c_master_transmit(m_handle, buf, sizeof(buf), -1);
}

/**
 * @brief  Enable all power rails (register 0x01, bit 5).
 */
void TPS::enableRails()
{
  writeReg(0x01, 0b00100000);
}

/**
 * @brief  Disable all power rails (register 0x01 = 0x00).
 */
void TPS::disableRails()
{
  writeReg(0x01, 0b00000000);
}

/**
 * @brief  Set the power-up sequence register (UPSEQ0, 0x09).
 *
 * @param  uint8_t seq  Sequence byte value.
 */
void TPS::setPowerUpSequence(uint8_t seq)
{
  writeReg(0x09, seq);
}

/**
 * @brief  Set the power-down sequence register (DWNSEQ0, 0x0B).
 *
 * @param  uint8_t seq  Sequence byte value.
 */
void TPS::setPowerDownSequence(uint8_t seq)
{
  writeReg(0x0B, seq);
}

/**
 * @brief  Read the PGSTAT register (0x0F).
 *
 * @return uint8_t
 *         raw power-good status bitmask
 */
uint8_t TPS::readPowerGood()
{
  return readReg(0x0F);
}

/**
 * @brief  Poll the PMIC until power-good state matches target or 250 ms timeout.
 *
 * @param  bool target
 *         true = wait for rails up, false = wait for rails down.
 *
 * @return bool
 *         true if target state reached, false if timeout elapsed.
 */
bool TPS::waitPowerGood(bool target)
{
  int64_t timer = esp_timer_get_time();
  do {
    esp_rom_delay_us(1000);
  } while ((readPowerGood() == TPS_PWR_GOOD) != target && (esp_timer_get_time() - timer) < 250000LL);
  return (esp_timer_get_time() - timer) < 250000LL;
}

void TPS::writeReg(uint8_t reg, uint8_t val)
{
  uint8_t buf[2] = {reg, val};
  i2c_master_transmit(m_handle, buf, sizeof(buf), -1);
}

uint8_t TPS::readReg(uint8_t reg)
{
  uint8_t val = 0;
  i2c_master_transmit_receive(m_handle, &reg, 1, &val, 1, -1);
  return val;
}
