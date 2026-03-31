#include "soc/i2s_struct.h"
#include "soc/gpio_sig_map.h"
#include "driver/gpio.h"
#include "esp_heap_caps.h"
#include "string.h"

#include "Inkplate6.h"

// static const char* TAG = "ESP_INKPLATE6";

// global instance, declared extern in pins.h
PCAL expander1(IO_INT_ADDR);
PCAL expander2(IO_EXT_ADDR, expander1.getBusHandle());

/**
 * ============================================================
 * Public functions
 * ============================================================
 */

/**
 * @brief  Inkplate6 constructor.
 *
 * @note   Allocates framebuffer and DMA buffers in PSRAM/DMA-capable DRAM
 *         and pre-computes the grayscale waveform LUTs.
 */
Inkplate6::Inkplate6()
{
  // framebuffer in PSRAM
  m_framebufferColor = (uint8_t*)heap_caps_malloc(E_INK_WIDTH * E_INK_HEIGHT / 2, MALLOC_CAP_SPIRAM);
  memset(m_framebufferColor, 0xFF, E_INK_WIDTH * E_INK_HEIGHT / 2);

  // DMA line buffer and descriptor in DMA-capable DRAM
  m_dmaLineBuffer = (volatile uint8_t*)heap_caps_malloc((E_INK_WIDTH / 4) + 16,  MALLOC_CAP_DMA);
  m_dmaI2SDesc    = (volatile lldesc_s*)heap_caps_malloc(sizeof(lldesc_s),       MALLOC_CAP_DMA);

  calculateLUTs();
}

/**
 * @brief  Initialize the Inkplate6 hardware.
 *
 * @note   Must be called once before any display operations.
 *         Initializes GPIO, IO expanders, and the PMIC.
 */
void Inkplate6::begin()
{
  gpioInit();
  pmicBegin();
}

void Inkplate6::writePixelInternal(int16_t x, int16_t y, uint16_t color)
{
  int16_t x0 = x;
  int16_t y0 = y;
  if (x0 > E_INK_WIDTH - 1 || y0 > E_INK_HEIGHT - 1 || x0 < 0 || y0 < 0)
    return;

  color &= 7;
  int x_ = x0 >> 1;
  int x_sub = x0 & 1;
  uint8_t temp;
  temp = *(m_framebufferColor+ 400 * y0 + x_);
  *(m_framebufferColor + 400 * y0 + x_) = (pixelMaskGLUT[x_sub] & temp) | (x_sub ? color : color << 4);
}

/**
 * @brief  Clear the framebuffer to white (0xFF).
 */
void Inkplate6::clearDisplay()
{
  memset(m_framebufferColor, 0xFF, E_INK_WIDTH * E_INK_HEIGHT / 2);
}

/**
 * @brief  Fill the framebuffer to black (0x00).
 */
void Inkplate6::fillDisplay()
{
  memset(m_framebufferColor, 0, E_INK_WIDTH * E_INK_HEIGHT / 2);
}


/**
 * @brief  Push the framebuffer to the display using 3-bit (8-level) grayscale.
 *
 * @param  bool leaveOn
 *         if true, leave the eink power supply on after the update
 */
void Inkplate6::display3b(bool leaveOn)
{
  if (!einkOn())
    return;

  clean(0, 1);
  clean(1, 18);
  clean(2, 1);
  clean(0, 18);
  clean(2, 1);
  clean(1, 18);
  clean(2, 1);
  clean(0, 18);
  clean(2, 1);

  for (int k = 0; k < 9; ++k)
  {
    uint8_t *dp = m_framebufferColor + E_INK_WIDTH * E_INK_HEIGHT / 2;

    vscanStart();
    for (int i = 0; i < E_INK_HEIGHT; ++i)
    {
       for (int j = 0; j < (E_INK_WIDTH / 4); j += 4)
       {
          uint8_t p0, p1;

          p0 = *(--dp); p1 = *(--dp);
          m_dmaLineBuffer[j + 2] = (m_glut2[k * 256 + p0] | m_glut[k * 256 + p1]);
          p0 = *(--dp); p1 = *(--dp);
          m_dmaLineBuffer[j + 3] = (m_glut2[k * 256 + p0] | m_glut[k * 256 + p1]);
          p0 = *(--dp); p1 = *(--dp);
          m_dmaLineBuffer[j]     = (m_glut2[k * 256 + p0] | m_glut[k * 256 + p1]);
          p0 = *(--dp); p1 = *(--dp);
          m_dmaLineBuffer[j + 1] = (m_glut2[k * 256 + p0] | m_glut[k * 256 + p1]);
        }
        sendDataI2S();
        vscanEnd();
    }
  esp_rom_delay_us(230);
  }

  clean(3, 1);
  vscanStart();

  // if (!leaveOn)
    // einkOff();
}

/**
 * @brief  Turn on epaper power supply (TPS65186).
 *
 * @return 1 on success, 0 on timeout waiting for power good.
 *
 * @note   Power-on order matters — wrong order can damage the display.
 */
int Inkplate6::einkOn()
{
  if (getPanelState())
    return 1;

  WAKEUP_SET;
  vTaskDelay(pdMS_TO_TICKS(5));

  uint8_t buf[2];
  // enable all rails
  buf[0] = 0x01;
  buf[1] = 0b00100000;
  i2c_master_transmit(m_tpsHandle, buf, sizeof(buf), -1);
  // modify power up sequence
  buf[0] = 0x09;
  buf[1] = 0b11100100;
  i2c_master_transmit(m_tpsHandle, buf, sizeof(buf), -1);
  // modify power down sequence (VEE and VNEG swapped)
  buf[0] = 0x0B;
  buf[1] = 0b00011011;
  i2c_master_transmit(m_tpsHandle, buf, sizeof(buf), -1);

  pinsAsOutputs();
  LE_CLEAR;
  SPH_SET;
  GMOD_SET;
  SPV_SET;
  CKV_CLEAR;
  OE_CLEAR;
  PWRUP_SET;
  setPanelState(true);

  if (!waitPowerGood(true))
  {
    einkOff();
    return 0;
  }

  VCOM_SET;
  OE_SET;
  return 1;
}

/**
 * @brief  Turn off epaper power supply and put all IO pins in high-Z state.
 */
void Inkplate6::einkOff()
{
  if (!getPanelState())
    return;

  VCOM_CLEAR;
  OE_CLEAR;
  GMOD_CLEAR;
  LE_CLEAR;
  CKV_CLEAR;
  SPH_CLEAR;
  SPV_CLEAR;
  PWRUP_CLEAR;

  waitPowerGood(false);

  WAKEUP_CLEAR;

  uint8_t buf[2] = {0x01, 0b00000000};
  i2c_master_transmit(m_tpsHandle, buf, sizeof(buf), -1);

  pinsZstate();
  setPanelState(false);
}

/**
 * @brief  Set the panel power state.
 *
 * @param  bool state
 *         true if the panel is powered on, false if powered off
 */
void Inkplate6::setPanelState(bool state)
{
  m_panelState = state;
}

/**
 * @brief  Get the panel power state.
 *
 * @return bool
 *         true if the panel is powered on, false if powered off
 */
bool Inkplate6::getPanelState()
{
  return m_panelState;
}

/**
 * ============================================================
 * Private functions
 * ============================================================
 */

/**
 * @brief  Pre-compute m_glut and m_glut2 waveform lookup tables.
 *
 * @note   Each entry maps a packed pixel byte to the I2S output byte
 *         for a given waveform phase. m_glut2 is the same but shifted
 *         into the high nibble for interleaved output.
 */
void Inkplate6::calculateLUTs()
{
  for (int j = 0; j < 9; ++j)
  {
    for (int i = 0; i < 256; ++i)
    {
      m_glut [j * 256 + i]  = (waveform3Bit[i & 0x07][j] << 2) | (waveform3Bit[(i >> 4) & 0x07][j]);
      m_glut2[j * 256 + i] = ((waveform3Bit[i & 0x07][j] << 2) | (waveform3Bit[(i >> 4) & 0x07][j])) << 4;
    }
  }
}


/**
 * @brief  Configure all GPIO, IO expander pins, and the pixel-to-GPIO LUT.
 *
 * @note   Sets SPI pins as inputs to reduce deep-sleep current, configures
 *         expander1 EPD control signals and expander2 data outputs, then
 *         routes I2S data pins via pinsAsOutputs().
 */
void Inkplate6::gpioInit()
{
  for (uint32_t i = 0; i < 256; ++i)
    m_pinLUT[i] = ((i & 0x03) << 4) | (((i & 0x0C) >> 2) << 18) | (((i & 0x10) >> 4) << 23) | (((i & 0xE0) >> 5) << 25);

  gpio_set_direction(GPIO_NUM_12, GPIO_MODE_INPUT);
  gpio_set_direction(GPIO_NUM_13, GPIO_MODE_INPUT);
  gpio_set_direction(GPIO_NUM_14, GPIO_MODE_INPUT);
  gpio_set_direction(GPIO_NUM_15, GPIO_MODE_INPUT);

  expander1.setDirection(IO_NUM_A0, IO_MODE_OUTPUT);  // OE
  expander1.setDirection(IO_NUM_A1, IO_MODE_OUTPUT);  // GMOD
  expander1.setDirection(IO_NUM_A2, IO_MODE_OUTPUT);  // SPV
  expander1.setDirection(IO_NUM_A3, IO_MODE_OUTPUT);  // WAKEUP
  expander1.setDirection(IO_NUM_A4, IO_MODE_OUTPUT);  // PWRUP
  expander1.setDirection(IO_NUM_A5, IO_MODE_OUTPUT);  // VCOM

  expander1.setDirection(GPIO0_ENABLE, IO_MODE_OUTPUT);
  expander1.setLevel(GPIO0_ENABLE, 1);

  expander1.setDirection(IO_NUM_B1, IO_MODE_OUTPUT);  // battery MOSFET, v2: LOW
  expander1.setLevel(IO_NUM_B1, 0);

  expander1.setDirection(IO_NUM_B6, IO_MODE_OUTPUT);  // unused, keep low
  expander1.setLevel(IO_NUM_B6, 0);
  expander1.setDirection(IO_NUM_B7, IO_MODE_OUTPUT);
  expander1.setLevel(IO_NUM_B7, 0);

  expander1.setDirection(SD_PMOS_PIN, IO_MODE_INPUT);

  // set all pins as outputs, LOW
  expander2.setPort(IO_PORT_0, 0x00);
  expander2.setPort(IO_PORT_1, 0x00);
  expander2.setPortDirection(IO_PORT_0, 0x00);
  expander2.setPortDirection(IO_PORT_1, 0x00);

  pinsAsOutputs();
}

/**
 * @brief  Send a solid waveform pattern to the display for cleaning.
 *
 * @param  uint8_t c
 *         pattern: 0 = discharge (0xAA), 1 = charge (0x55),
 *                  2 = blank (0x00),     3 = full (0xFF)
 *
 * @param  uint8_t rep
 *         number of times to repeat the pattern across the full frame
 */
void Inkplate6::clean(uint8_t c, uint8_t rep)
{
  einkOn();
  uint8_t data = 0;
  if (c == 0)
    data = 0b10101010;
  else if (c == 1)
    data = 0b01010101;
  else if (c == 2)
    data = 0b00000000;
  else if (c == 3)
    data = 0b11111111;

  for (int i = 0; i < (E_INK_WIDTH / 4); i++)
    m_dmaLineBuffer[i] = data;

  m_dmaI2SDesc->size          = (E_INK_WIDTH / 4) + 16;
  m_dmaI2SDesc->length        = (E_INK_WIDTH / 4) + 16;
  m_dmaI2SDesc->sosf          = 1;
  m_dmaI2SDesc->owner         = 1;
  m_dmaI2SDesc->qe.stqe_next  = 0;
  m_dmaI2SDesc->eof           = 1;
  m_dmaI2SDesc->buf           = m_dmaLineBuffer;
  m_dmaI2SDesc->offset        = 0;

  for (int k = 0; k < rep; ++k)
  {
    vscanStart();

    for (int i = 0; i < E_INK_HEIGHT; ++i)
    {
      sendDataI2S();
      vscanEnd();
    }

    esp_rom_delay_us(230);
  }
}


/**
 * @brief  Initialize the TPS65186 PMIC.
 *
 * @note   Registers the device on the I2C bus and programs the
 *         power-up/down rail sequences (UPSEQ0/1, DWNSEQ0/1).
 */
void Inkplate6::pmicBegin()
{
  i2c_device_config_t tps_cfg = {};
  tps_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
  tps_cfg.device_address  = 0x48;
  tps_cfg.scl_speed_hz    = 100000;
  i2c_master_bus_add_device(expander1.getBusHandle(), &tps_cfg, &m_tpsHandle);

  WAKEUP_SET;
  vTaskDelay(pdMS_TO_TICKS(1));

  uint8_t buf[5] = {0x09, 0b00011011, 0b00000000, 0b00011011, 0b00000000};
  i2c_master_transmit(m_tpsHandle, buf, sizeof(buf), -1);

  vTaskDelay(pdMS_TO_TICKS(1));
  WAKEUP_CLEAR;
}

/**
 * @brief  Read the TPS65186 PGSTAT register.
 *
 * @return uint8_t
 *         raw power-good status bitmask; compare against PWR_GOOD_OK
 */
uint8_t Inkplate6::readPowerGood()
{
  uint8_t reg = 0x0F;
  uint8_t val = 0;
  i2c_master_transmit_receive(m_tpsHandle, &reg, 1, &val, 1, -1);
  return val;
}

/**
 * @brief  Poll the PMIC until power-good state matches target or timeout.
 *
 * @param  bool target
 *         true to wait until all rails are up, false to wait until they are down
 *
 * @return bool
 *         true if target state was reached, false if 250 ms timeout elapsed
 */
bool Inkplate6::waitPowerGood(bool target)
{
  int64_t timer = esp_timer_get_time();
  do {
    vTaskDelay(pdMS_TO_TICKS(1));
  } while ((readPowerGood() == PWR_GOOD_OK) != target && (esp_timer_get_time() - timer) < 250000LL);
  return (esp_timer_get_time() - timer) < 250000LL;
}


/**
 * @brief  Start a new frame by pulsing SPV/CKV to initialise the gate driver.
 */
void Inkplate6::vscanStart()
{
  CKV_SET;
  esp_rom_delay_us(7);
  SPV_CLEAR;
  esp_rom_delay_us(10);
  CKV_CLEAR;
  esp_rom_delay_us(0);
  CKV_SET;
  esp_rom_delay_us(8);
  SPV_SET;
  esp_rom_delay_us(10);
  CKV_CLEAR;
  esp_rom_delay_us(0);
  CKV_SET;
  esp_rom_delay_us(18);
  CKV_CLEAR;
  esp_rom_delay_us(0);
  CKV_SET;
  esp_rom_delay_us(18);
  CKV_CLEAR;
  esp_rom_delay_us(0);
  CKV_SET;
}

/**
 * @brief  End the current row by latching the data to the display.
 */
void Inkplate6::vscanEnd()
{
  CKV_CLEAR;
  LE_SET;
  LE_CLEAR;
  esp_rom_delay_us(0);
}

/**
 * @brief  Set EPD control and data pins as outputs and route I2S signals.
 */
void Inkplate6::pinsAsOutputs()
{
  gpio_set_direction(GPIO_NUM_0,  GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NUM_2,  GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NUM_32, GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NUM_33, GPIO_MODE_OUTPUT);

  expander1.setDirection(IO_NUM_A0, IO_MODE_OUTPUT);
  expander1.setDirection(IO_NUM_A1, IO_MODE_OUTPUT);
  expander1.setDirection(IO_NUM_A2, IO_MODE_OUTPUT);

  setI2S1pin(0,  I2S1O_BCK_OUT_IDX,    0);
  setI2S1pin(4,  I2S1O_DATA_OUT0_IDX,  0);
  setI2S1pin(5,  I2S1O_DATA_OUT1_IDX,  0);
  setI2S1pin(18, I2S1O_DATA_OUT2_IDX,  0);
  setI2S1pin(19, I2S1O_DATA_OUT3_IDX,  0);
  setI2S1pin(23, I2S1O_DATA_OUT4_IDX,  0);
  setI2S1pin(25, I2S1O_DATA_OUT5_IDX,  0);
  setI2S1pin(26, I2S1O_DATA_OUT6_IDX,  0);
  setI2S1pin(27, I2S1O_DATA_OUT7_IDX,  0);

  // start sending clock to the EPD
  m_i2s->conf1.tx_stop_en = 1;
}

/**
 * @brief  Release all EPD pins to high-Z (input) to save power.
 *
 * @note   Stops the I2S clock before releasing pins.
 */
void Inkplate6::pinsZstate()
{
  m_i2s->conf1.tx_stop_en = 0;

  gpio_set_direction(GPIO_NUM_2,  GPIO_MODE_INPUT);
  gpio_set_direction(GPIO_NUM_32, GPIO_MODE_INPUT);
  gpio_set_direction(GPIO_NUM_33, GPIO_MODE_INPUT);

  expander1.setDirection(IO_NUM_A0, IO_MODE_INPUT);
  expander1.setDirection(IO_NUM_A1, IO_MODE_INPUT);
  expander1.setDirection(IO_NUM_A2, IO_MODE_INPUT);

  // set up the EPD Data and CL pins for I2S
  gpio_set_direction(GPIO_NUM_0,  GPIO_MODE_INPUT);
  gpio_set_direction(GPIO_NUM_4,  GPIO_MODE_INPUT);
  gpio_set_direction(GPIO_NUM_5,  GPIO_MODE_INPUT);
  gpio_set_direction(GPIO_NUM_18, GPIO_MODE_INPUT);
  gpio_set_direction(GPIO_NUM_19, GPIO_MODE_INPUT);
  gpio_set_direction(GPIO_NUM_23, GPIO_MODE_INPUT);
  gpio_set_direction(GPIO_NUM_25, GPIO_MODE_INPUT);
  gpio_set_direction(GPIO_NUM_26, GPIO_MODE_INPUT);
  gpio_set_direction(GPIO_NUM_27, GPIO_MODE_INPUT);
}
