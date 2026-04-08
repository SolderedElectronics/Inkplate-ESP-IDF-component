#include "TPS.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_rom_sys.h"

static const char *TAG = "TPS";

/**
 * ============================================================
 * Public functions
 * ============================================================
 */

/**
 * @brief  Register the TPS65186 on the I2C bus.
 *
 * @param  i2c_master_bus_handle_t busHandle
 *         Handle to an already-initialised I2C master bus.
 */
TPS::TPS(I2C &i2c)
{
  ESP_ERROR_CHECK(i2c.addDevice(TPS_I2C_ADDR, &m_handle));
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
 * @brief  Program VCOM voltage into the TPS65186 internal EEPROM.
 *
 * @param  double vcom
 *         VCOM value in volts (must be in range -5.0 to 0.0).
 * @param  PCAL &expander
 *         IO expander instance with TPS_INT_PIN connected to TPS65186 INT.
 *
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG if out of range,
 *         ESP_FAIL if readback verification fails.
 *
 * @note   Call with eink power already on (einkOn()).
 */
esp_err_t TPS::writeVCOM(double vcom, PCAL &expander)
{
  if (vcom < -5.0 || vcom > 0.0)
    return ESP_ERR_INVALID_ARG;

  // Configure INT pin as input with pull-up
  expander.setDirection(TPS_INT_PIN, IO_MODE_INPUT);
  expander.setPullMode(TPS_INT_PIN, IO_PULLUP);

  // Convert to 9-bit raw value (abs * 100, e.g. -1.23V → 123)
  int     raw    = abs((int)(vcom * 100.0)) & 0x1FF;
  uint8_t vcomL  = (uint8_t)(raw & 0xFF);
  uint8_t vcomMSB = (uint8_t)((raw >> 8) & 0x01);

  // Write low 8 bits to REG_VCOM1 (0x03)
  writeReg(0x03, vcomL);

  // Read REG_VCOM2 (0x04), preserve reserved bits, set MSB, clear program bit
  uint8_t r4 = readReg(0x04);
  r4 &= ~((1 << 0) | (1 << 6));
  r4 |= vcomMSB;
  writeReg(0x04, r4);
  esp_rom_delay_us(1000);

  // Strobe EEPROM program bit (bit 6)
  writeReg(0x04, r4 | (1 << 6));

  // Wait for INT to go low (programming done), 1 s timeout
  int64_t deadline = esp_timer_get_time() + 1000000LL;
  while (expander.getLevel(TPS_INT_PIN) && esp_timer_get_time() < deadline)
    esp_rom_delay_us(1000);

  // Clear interrupt by reading INT1 register (0x07)
  (void)readReg(0x07);

  // Readback verification
  uint8_t rdL  = readReg(0x03);
  uint8_t rdH  = readReg(0x04) & 0x01;
  int     check = ((int)rdH << 8) | rdL;

  ESP_LOGI(TAG, "VCOM program: raw=%d readback=%d %s",
           raw, check, (check == raw) ? "OK" : "FAIL");

  return (check == raw) ? ESP_OK : ESP_FAIL;
}

/**
 * @brief  Read the VCOM voltage currently stored in TPS65186 registers.
 *
 * @return double
 *         VCOM in volts (negative, e.g. -1.23).
 *
 * @note   Call with eink power already on (einkOn()).
 */
double TPS::readVCOM()
{
  uint8_t vcomL = readReg(0x03);
  uint8_t vcomH = readReg(0x04) & 0x01;
  int raw = ((int)vcomH << 8) | vcomL;
  return -(raw / 100.0);
}

/**
 * @brief  Read the on-chip thermistor temperature.
 *
 * @return int8_t
 *         Temperature in degrees Celsius.
 *
 * @note   Call with eink power already on (einkOn()).
 */
int8_t TPS::readTemperature()
{
  writeReg(0x0D, 0x80);        // trigger thermistor ADC conversion (bit 7 = CONV)
  esp_rom_delay_us(5000);      // ~5 ms for conversion to complete
  return (int8_t)readReg(0x00); // TMST_VALUE register
}

/**
 * ============================================================
 * Private functions
 * ============================================================
 */

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
