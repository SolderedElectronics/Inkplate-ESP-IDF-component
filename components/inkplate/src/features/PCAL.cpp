#include "driver/i2c_master.h"
#include "esp_log.h"
#include "esp_check.h"

#include "PCAL.h"

static const char* TAG = "ESP_PCAL";

/**
 * ============================================================
 * Public functions
 * ============================================================
 */

/**
 * @brief  PCAL constructor.
 *
 * @note   Sets I2C port properties.
 */
PCAL::PCAL(uint8_t addr, i2c_master_bus_handle_t busHandle)
{
  if (busHandle == NULL)
  {
    i2c_master_bus_config_t bus_config = {};
    bus_config.i2c_port = I2C_NUM_0;
    bus_config.sda_io_num = I2C_SDA;
    bus_config.scl_io_num = I2C_SCL;
    bus_config.clk_source = I2C_CLK_SRC_DEFAULT;
    bus_config.glitch_ignore_cnt = 7;
    bus_config.flags.enable_internal_pullup = true;

    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &m_busHandle));
  }
  else
  {
    m_busHandle = busHandle;
  }

  i2c_device_config_t dev_config = {};
  dev_config.dev_addr_length = I2C_ADDR_BIT_LEN_7;
  dev_config.device_address = addr;
  dev_config.scl_speed_hz = 400000;

  ESP_ERROR_CHECK(i2c_master_bus_add_device(m_busHandle, &dev_config, &m_devHandle));

  m_blockedPins = 0;

  ESP_LOGI(TAG, "I2C initilization finished!");
}

/**
 * @brief  Set output level of a pin.
 *
 * @param  IOPin_t pin
 *         pin to set
 *
 * @param  uint8_t level
 *         0 for low, non-zero for high
 *
 * @return esp_err_t
 *         ESP_OK if no error occured
 *         ESP_ERR_INVALID_ARG if pin is blocked
 *         ESP_ERR_INVALID_STATE if pin is configured as input
 */
esp_err_t PCAL::setLevel(IOPin_t pin, uint8_t level)
{
  if (checkBlockedPins(pin))
    return ESP_ERR_INVALID_ARG;

  // check if pin is set as input
  uint8_t cfgReg, cfgBit;
  pinToRegBit(pin, PCAL6416A_CFGPORT0, cfgReg, cfgBit);
  if ((readPin(cfgReg) >> cfgBit) & 1)
    return ESP_ERR_INVALID_STATE;

  uint8_t reg, bit;
  pinToRegBit(pin, PCAL6416A_OUTPORT0, reg, bit);

  uint8_t val = readPin(reg);
  if (level)
    val |= (1 << bit);
  else
    val &= ~(1 << bit);

  return writePin(reg, val);
}

/**
 * @brief  Get the output level of a pin.
 *
 * @param  IOPin_t pin
 *         pin to get
 *
 * @return int
 *         pin level
 *         ESP_ERR_INVALID_ARG if pin is blocked
 */
int PCAL::getLevel(IOPin_t pin)
{
  if (checkBlockedPins(pin))
    return ESP_ERR_INVALID_ARG;

  uint8_t reg, bit;
  pinToRegBit(pin, PCAL6416A_INPORT0, reg, bit);

  return (readPin(reg) >> bit) & 1;
}

/**
 * @brief  Set output level of a port.
 *
 * @param  IOPort_t port
 *         port to set
 *
 * @param  uint8_t value
 *         value to write to port
 *
 * @return esp_err_t
 *         ESP_OK if no error occured
 */
esp_err_t PCAL::setPort(IOPort_t port, uint8_t value)
{
  uint8_t reg = (port == IO_PORT_0) ? PCAL6416A_OUTPORT0 : PCAL6416A_OUTPORT1;
  return writePin(reg, value);
}

esp_err_t PCAL::setPortDirection(IOPort_t port, uint8_t mask)
{
  uint8_t reg = (port == IO_PORT_0) ? PCAL6416A_CFGPORT0 : PCAL6416A_CFGPORT1;
  return writePin(reg, mask);
}

/**
 * @brief  Get input level of a port.
 *
 * @param  IOPort_t port
 *         port to get
 *
 * @return int
 *         port value
 */
int PCAL::getPort(IOPort_t port)
{
  uint8_t reg = (port == IO_PORT_0) ? PCAL6416A_INPORT0 : PCAL6416A_INPORT1;
  return readPin(reg);
}

/**
 * @brief  Set direction (input/output) of a pin.
 *
 * @param  IOPin_t pin
 *         pin to configure
 *
 * @param  IOMode_t mode
 *         IO_MODE_INPUT or IO_MODE_OUTPUT
 *
 * @return esp_err_t
 *         ESP_OK if no error occured
 *         ESP_ERR_INVALID_ARG if pin is blocked
 */
esp_err_t PCAL::setDirection(IOPin_t pin, IOMode_t mode)
{
  if (checkBlockedPins(pin))
    return ESP_ERR_INVALID_ARG;

  uint8_t reg, bit;
  pinToRegBit(pin, PCAL6416A_CFGPORT0, reg, bit);

  // 1 = input, 0 = output
  uint8_t val = readPin(reg);
  if (mode == IO_MODE_INPUT)
    val |= (1 << bit);
  else
    val &= ~(1 << bit);

  return writePin(reg, val);
}

/**
 * @brief  Set pull mode of pin (up/down).
 *
 * @param  IOPin_t pin
 *         pin to configure
 *
 * @param  IOPullMode_t pullMode
 *         IO_PULLUP or IO_PULLDOWN
 *
 * @return esp_err_t
 *         ESP_OK if no error occured
 *         ESP_ERR_INVALID_ARG if pin is blocked
 */
esp_err_t PCAL::setPullMode(IOPin_t pin, IOPullMode_t pullMode)
{
  if (checkBlockedPins(pin))
    return ESP_ERR_INVALID_ARG;

  uint8_t enReg, selReg, bit;
  pinToRegBit(pin, PCAL6416A_PUPDEN_REG0,  enReg,  bit);
  pinToRegBit(pin, PCAL6416A_PUPDSEL_REG0, selReg, bit);

  // enable the pull resistor for this pin
  uint8_t enVal = readPin(enReg);
  enVal |= (1 << bit);
  esp_err_t err = writePin(enReg, enVal);
  if (err != ESP_OK)
    return err;

  // select pull-up (1) or pull-down (0)
  uint8_t selVal = readPin(selReg);
  if (pullMode == IO_PULLUP)
    selVal |= (1 << bit);
  else
    selVal &= ~(1 << bit);

  return writePin(selReg, selVal);
}

/**
 * @brief  Set polarity inversion for a pin.
 *
 * @param  IOPin_t pin
 *         pin to configure
 *
 * @param  bool invert
 *         true to invert, false for normal
 *
 * @return esp_err_t
 *         ESP_OK if no error occured
 *         ESP_ERR_INVALID_ARG if pin is blocked
 */
esp_err_t PCAL::setPolarityInversion(IOPin_t pin, bool invert)
{
  if (checkBlockedPins(pin))
    return ESP_ERR_INVALID_ARG;

  uint8_t reg, bit;
  pinToRegBit(pin, PCAL6416A_POLINVPORT0, reg, bit);

  uint8_t val = readPin(reg);
  if (invert)
    val |= (1 << bit);
  else
    val &= ~(1 << bit);

  return writePin(reg, val);
}

/**
 * @brief  Set input latch for a pin.
 *
 * @param  IOPin_t pin
 *         pin to configure
 *
 * @param  bool latch
 *         true to enable latch, false to disable
 *
 * @note   When latched, the input state is held until read.
 *         Useful in combination with interrupts.
 *
 * @return esp_err_t
 *         ESP_OK if no error occured
 *         ESP_ERR_INVALID_ARG if pin is blocked
 */
esp_err_t PCAL::setInputLatch(IOPin_t pin, bool latch)
{
  if (checkBlockedPins(pin))
    return ESP_ERR_INVALID_ARG;

  uint8_t reg, bit;
  pinToRegBit(pin, PCAL6416A_INLAT_REG0, reg, bit);

  uint8_t val = readPin(reg);
  if (latch)
    val |= (1 << bit);
  else
    val &= ~(1 << bit);

  return writePin(reg, val);
}

/**
 * @brief  Set output mode (push-pull or open-drain) for a port.
 *
 * @param  IOPort_t port
 *         port to configure
 *
 * @param  IOOutputMode_t mode
 *         IO_PUSH_PULL or IO_OPEN_DRAIN
 *
 * @return esp_err_t
 *         ESP_OK if no error occured
 */
esp_err_t PCAL::setOutputMode(IOPort_t port, IOOutputMode_t mode)
{
  uint8_t bit = (port == IO_PORT_0) ? 0 : 1;

  uint8_t val = readPin(PCAL6416A_OUTPORT_CONF);
  if (mode == IO_OPEN_DRAIN)
    val |= (1 << bit);
  else
    val &= ~(1 << bit);

  return writePin(PCAL6416A_OUTPORT_CONF, val);
}

/**
 * @brief  Set output drive strength for a pin.
 *
 * @param  IOPin_t pin
 *         pin to configure
 *
 * @param  IODriveStrength_t strength
 *         IO_DRIVE_25, IO_DRIVE_50, IO_DRIVE_75, or IO_DRIVE_100
 *
 * @return esp_err_t
 *         ESP_OK if no error occured
 *         ESP_ERR_INVALID_ARG if pin is blocked
 *
 * @note   Each pin occupies 2 bits in one of four drive strength registers
 *         (REG00–REG11). Port 0 uses 0x40/0x41, port 1 uses 0x42/0x43.
 */
esp_err_t PCAL::setDriveStrength(IOPin_t pin, IODriveStrength_t strength)
{
  if (checkBlockedPins(pin))
    return ESP_ERR_INVALID_ARG;

  // two registers per port (pins 0-3 in first, pins 4-7 in second)
  uint8_t pinIndex = pin % 8; // pin index within its port
  uint8_t baseReg  = (pin >= IO_NUM_B0) ? PCAL6416A_OUTDRVST_REG10 : PCAL6416A_OUTDRVST_REG00;
  uint8_t reg      = baseReg + (pinIndex / 4);
  uint8_t shift    = (pinIndex % 4) * 2;

  uint8_t val = readPin(reg);
  val &= ~(0x03 << shift);
  val |= ((uint8_t)strength << shift);

  return writePin(reg, val);
}

/**
 * @brief  Enable interrupt for a pin.
 *
 * @param  IOPin_t pin
 *         pin to enable interrupt on
 *
 * @return esp_err_t
 *         ESP_OK if no error occured
 *         ESP_ERR_INVALID_ARG if pin is blocked
 */
esp_err_t PCAL::interruptEnable(IOPin_t pin)
{
  if (checkBlockedPins(pin))
    return ESP_ERR_INVALID_ARG;

  uint8_t reg, bit;
  pinToRegBit(pin, PCAL6416A_INTMSK_REG0, reg, bit);

  // 0 = interrupt enabled, 1 = masked
  uint8_t val = readPin(reg);
  val &= ~(1 << bit);

  return writePin(reg, val);
}

/**
 * @brief  Disable interrupt for a pin.
 *
 * @param  IOPin_t pin
 *         pin to disable interrupt on
 *
 * @return esp_err_t
 *         ESP_OK if no error occured
 *         ESP_ERR_INVALID_ARG if pin is blocked
 */
esp_err_t PCAL::interruptDisable(IOPin_t pin)
{
  if (checkBlockedPins(pin))
    return ESP_ERR_INVALID_ARG;

  uint8_t reg, bit;
  pinToRegBit(pin, PCAL6416A_INTMSK_REG0, reg, bit);

  // 1 = masked (disabled)
  uint8_t val = readPin(reg);
  val |= (1 << bit);

  return writePin(reg, val);
}

/**
 * @brief  Check if an interrupt has been triggered on a pin.
 *
 * @param  IOPin_t pin
 *         pin to check
 *
 * @note   The interrupt status register clears automatically on read.
 *
 * @return bool
 *         true if interrupt triggered, false otherwise
 */
bool PCAL::getInterrupt(IOPin_t pin)
{
  uint8_t reg, bit;
  pinToRegBit(pin, PCAL6416A_INTSTAT_REG0, reg, bit);

  return (readPin(reg) >> bit) & 1;
}

/**
 * @brief  Block a pin from being modified.
 *
 * @param  IOPin_t pin
 *         pin to block
 *
 * @return esp_err_t
 *         ESP_OK always
 */
esp_err_t PCAL::blockPin(IOPin_t pin)
{
  m_blockedPins |= (1 << pin);
  return ESP_OK;
}

/**
 * @brief  Unblock a previously blocked pin.
 *
 * @param  IOPin_t pin
 *         pin to unblock
 *
 * @return esp_err_t
 *         ESP_OK always
 */
esp_err_t PCAL::unblockPin(IOPin_t pin)
{
  m_blockedPins &= ~(1 << pin);
  return ESP_OK;
}

/**
 * ============================================================
 * Private functions
 * ============================================================
 */

/**
 * @brief  Resolve register address and bit position for a pin.
 *
 * @param  IOPin_t pin
 *         pin to resolve
 *
 * @param  uint8_t baseReg
 *         base register
 *
 * @param  uint8_t &reg
 *         output: resolved register address
 *
 * @param  uint8_t &bit
 *         output: bit position within the register
 */
void PCAL::pinToRegBit(IOPin_t pin, uint8_t baseReg, uint8_t &reg, uint8_t &bit)
{
  reg = baseReg + (pin >= IO_NUM_B0 ? 1 : 0);
  bit = pin % 8;
}

/**
 * @brief  Check if a pin is blocked.
 *
 * @param  IOPin_t pin
 *         pin to check
 *
 * @return bool
 *         true if blocked, false otherwise
 */
bool PCAL::checkBlockedPins(IOPin_t pin)
{
  return (m_blockedPins >> pin) & 1;
}

/**
 * @brief  Internal function to write to a register.
 *
 * @return esp_err_t
 *         ESP_OK if no error occured
 */
esp_err_t PCAL::writePin(uint8_t reg, uint8_t val)
{
  uint8_t data[2] = { reg, val };
  return i2c_master_transmit(m_devHandle, data, sizeof(data), -1);
}

/**
 * @brief  Internal function to read a register.
 *
 * @return uint8_t
 *         register value
 */
uint8_t PCAL::readPin(uint8_t reg)
{
  uint8_t val = 0;
  ESP_ERROR_CHECK(i2c_master_transmit_receive(m_devHandle, &reg, 1, &val, 1, -1));

  return val;
}
