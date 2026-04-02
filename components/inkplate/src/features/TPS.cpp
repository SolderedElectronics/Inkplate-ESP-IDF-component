#include "TPS.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"

/**
 * @brief  Register the TPS65186 on the I2C bus.
 *
 * @param  i2c_master_bus_handle_t busHandle
 *         Handle to an already-initialised I2C master bus.
 */
TPS::TPS(I2C &i2c)
{
  ESP_ERROR_CHECK(i2c.addDevice(TPS_I2C_ADDR, 100000, &m_handle));
}

/**
 * @brief  Write startup power-up/down sequences to the PMIC.
 *
 * @note   Should be called once, with WAKEUP asserted.
 *
 * @return esp_err_t
 *         ESP_OK on success, or an I2C driver error code.
 */
esp_err_t TPS::initSequences()
{
  uint8_t buf[5] = {0x09, 0b00011011, 0b00000000, 0b00011011, 0b00000000};
  return i2c_master_transmit(m_handle, buf, sizeof(buf), -1);
}

/**
 * @brief  Enable all power rails (register 0x01, bit 5).
 *
 * @return esp_err_t
 *         ESP_OK on success, or an I2C driver error code.
 */
esp_err_t TPS::enableRails()
{
  return writeReg(0x01, 0b00100000);
}

/**
 * @brief  Disable all power rails (register 0x01 = 0x00).
 *
 * @return esp_err_t
 *         ESP_OK on success, or an I2C driver error code.
 */
esp_err_t TPS::disableRails()
{
  return writeReg(0x01, 0b00000000);
}

/**
 * @brief  Set the power-up sequence register (UPSEQ0, 0x09).
 *
 * @param  uint8_t seq
 *         Sequence byte value.
 *
 * @return esp_err_t
 *         ESP_OK on success, or an I2C driver error code.
 */
esp_err_t TPS::setPowerUpSequence(uint8_t seq)
{
  return writeReg(0x09, seq);
}

/**
 * @brief  Set the power-down sequence register (DWNSEQ0, 0x0B).
 *
 * @param  uint8_t seq
 *         Sequence byte value.
 *
 * @return esp_err_t
 *         ESP_OK on success, or an I2C driver error code.
 */
esp_err_t TPS::setPowerDownSequence(uint8_t seq)
{
  return writeReg(0x0B, seq);
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

/**
 * @brief  Writes to register using I2C.
 *
 * @param  uint8_t reg
 *         register to write to
 *
 * @param  uint8_t val
 *         value to write
 *
 * @return esp_err_t
 *         ESP_OK on success, or an I2C driver error code.
 */
esp_err_t TPS::writeReg(uint8_t reg, uint8_t val)
{
  uint8_t buf[2] = {reg, val};
  return i2c_master_transmit(m_handle, buf, sizeof(buf), -1);
}

/**
 * @brief  Reads from register using I2C.
 *
 * @param  uint8_t reg
 *         register to read from
 *
 * @return uint8_t
 *         read value
 */
uint8_t TPS::readReg(uint8_t reg)
{
  uint8_t val = 0;
  i2c_master_transmit_receive(m_handle, &reg, 1, &val, 1, -1);
  return val;
}
