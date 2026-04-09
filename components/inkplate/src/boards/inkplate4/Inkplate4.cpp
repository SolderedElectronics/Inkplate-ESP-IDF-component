#include "driver/gpio.h"
#include "esp_heap_caps.h"
#include "string.h"
#include "esp_log.h"
#include "driver/i2c_master.h"

#include "I2C.h"
#include "Inkplate4.h"
#include "TPS.h"

// Peripherals defined in BoardCommon.cpp
extern PCAL expander1;
extern PCAL expander2;
extern TPS  tps;
extern I2C  i2c;

static const char *TAG = "INKPLATE4";

static const uint8_t waveform3Bit[8][9] = {};

/**
 * ============================================================
 * Public functions
 * ============================================================
 */

/**
 * @brief  Inkplate4 constructor.
 *
 * @note   Allocates framebuffers in PSRAM, pre-computes grayscale waveform LUTs,
 *         initialises GPIO and the PMIC.
 */
Inkplate4::Inkplate4() : BoardCommon(E_INK_WIDTH, E_INK_HEIGHT, 0, 0)
{
  ESP_ERROR_CHECK(initBuffers());
  // calculateLUTs();
  gpioInit();
  // blockGpioPins();
  ESP_ERROR_CHECK(pmicBegin());

  i2c_master_bus_handle_t bus = i2c.getBusHandle();
  apds.begin(bus);
  bq.begin(bus);
  lsm.begin(bus);
  bme.begin(bus);

  ESP_LOGI(TAG, "Initialization finished!");
}

/**
 * @brief  Send only the changed pixels to the display (1-bit mode only).
 *
 * @param  bool forced
 *         If true, bypasses the partial update block flag
 * @param  bool leaveOn
 *         If true, leaves the e-ink panel powered on after the update
 *
 * @return uint32_t
 *         Number of pixels that changed; 0 if a full update was performed instead
 */
uint32_t Inkplate4::partialUpdate(bool forced, bool leaveOn)
{
  return 0;
}

/**
 * @brief  Power on the e-ink panel and assert all required control signals.
 *
 * @return esp_err_t
 *         ESP_OK on success, ESP_ERR_TIMEOUT if the PMIC does not reach
 *         power-good within 250 ms.
 */
esp_err_t Inkplate4::einkOn()
{
  return ESP_OK;
}

/**
 * @brief  Power off the e-ink panel and tri-state all data lines.
 *
 * @return esp_err_t
 *         ESP_OK on success, or a TPS driver error code.
 */
esp_err_t Inkplate4::einkOff()
{
  return ESP_OK;
}

/**
 * ============================================================
 * Private functions
 * ============================================================
 */

/**
 * @brief  Allocate framebuffers in PSRAM.
 *
 * @return esp_err_t
 *         ESP_OK on success, ESP_ERR_NO_MEM if allocation fails.
 */
esp_err_t Inkplate4::initBuffers()
{
  m_framebufferColor = (uint8_t*)heap_caps_malloc(E_INK_WIDTH * E_INK_HEIGHT / 2, MALLOC_CAP_SPIRAM);
  if (!m_framebufferColor) return ESP_ERR_NO_MEM;
  memset(m_framebufferColor, 0xFF, E_INK_WIDTH * E_INK_HEIGHT / 2);

  m_framebuffer = (uint8_t*)heap_caps_malloc(E_INK_WIDTH * E_INK_HEIGHT / 8, MALLOC_CAP_SPIRAM);
  if (!m_framebuffer) return ESP_ERR_NO_MEM;
  memset(m_framebuffer, 0x00, E_INK_WIDTH * E_INK_HEIGHT / 8);

  m_newFramebuffer = (uint8_t*)heap_caps_malloc(E_INK_WIDTH * E_INK_HEIGHT / 8, MALLOC_CAP_SPIRAM);
  if (!m_newFramebuffer) return ESP_ERR_NO_MEM;
  memset(m_newFramebuffer, 0x00, E_INK_WIDTH * E_INK_HEIGHT / 8);

  m_waveformBuffer = (uint8_t*)heap_caps_malloc(E_INK_WIDTH * E_INK_HEIGHT / 4, MALLOC_CAP_SPIRAM);
  if (!m_waveformBuffer) return ESP_ERR_NO_MEM;
  memset(m_waveformBuffer, 0x00, E_INK_WIDTH * E_INK_HEIGHT / 4);

  m_glut = (uint32_t*)heap_caps_malloc(9 * 256 * sizeof(uint32_t), MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
  if (!m_glut) return ESP_ERR_NO_MEM;

  m_glut2 = (uint32_t*)heap_caps_malloc(9 * 256 * sizeof(uint32_t), MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
  if (!m_glut2) return ESP_ERR_NO_MEM;

  m_pinLUT = (uint32_t*)heap_caps_malloc(256 * sizeof(uint32_t), MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);
  if (!m_pinLUT) return ESP_ERR_NO_MEM;

  return ESP_OK;
}
/**
 * @brief  Pre-compute the grayscale waveform lookup tables.
 */
void Inkplate4::calculateLUTs()
{
}

/**
 * @brief  Drive the panel using 3-bit (8-level grayscale) waveform.
 *
 * @param  bool leaveOn
 *         If true, leave the panel powered on after the update.
 *
 * @return esp_err_t
 *         ESP_OK on success.
 */
esp_err_t Inkplate4::display3b(bool leaveOn)
{
  return ESP_OK;
}

/**
 * @brief  Drive the panel using 1-bit (black and white) waveform.
 *
 * @param  bool leaveOn
 *         If true, leave the panel powered on after the update.
 *
 * @return esp_err_t
 *         ESP_OK on success.
 */
esp_err_t Inkplate4::display1b(bool leaveOn)
{
  return ESP_OK;
}

/**
 * @brief  Initialise all GPIO pins and IO expander directions.
 */
void Inkplate4::gpioInit()
{
  for (uint32_t i = 0; i < 256; ++i)
    m_pinLUT[i] = ((i & 0x03) << 4) | (((i & 0x0C) >> 2) << 18) |
                  (((i & 0x10) >> 4) << 23) | (((i & 0xE0) >> 5) << 25);

  gpio_set_direction(GPIO_NUM_12, GPIO_MODE_INPUT);
  gpio_set_direction(GPIO_NUM_13, GPIO_MODE_INPUT);
  gpio_set_direction(GPIO_NUM_14, GPIO_MODE_INPUT);
  gpio_set_direction(GPIO_NUM_15, GPIO_MODE_INPUT);

  expander1.setDirection(OE,     IO_MODE_OUTPUT);
  expander1.setDirection(GMOD,   IO_MODE_OUTPUT);
  expander1.setDirection(SPV,    IO_MODE_OUTPUT);
  expander1.setDirection(WAKEUP, IO_MODE_OUTPUT);
  expander1.setDirection(PWRUP,  IO_MODE_OUTPUT);
  expander1.setDirection(VCOM,   IO_MODE_OUTPUT);

  expander1.setDirection(GPIO0_ENABLE, IO_MODE_OUTPUT);
  expander1.setLevel(GPIO0_ENABLE, 1);

  expander1.setDirection(IO_NUM_B1, IO_MODE_OUTPUT);
  expander1.setLevel(IO_NUM_B1, 0);

  expander1.setDirection(IO_NUM_B6, IO_MODE_OUTPUT);
  expander1.setLevel(IO_NUM_B6, 0);
  expander1.setDirection(IO_NUM_B7, IO_MODE_OUTPUT);
  expander1.setLevel(IO_NUM_B7, 0);

  expander1.setDirection(SD_PMOS_PIN, IO_MODE_INPUT);

  expander2.setPort(IO_PORT_0, 0x00);
  expander2.setPort(IO_PORT_1, 0x00);
  expander2.setPortDirection(IO_PORT_0, 0x00);
  expander2.setPortDirection(IO_PORT_1, 0x00);

  pinsAsOutputs();
}

/**
 * @brief  Run a single cleaning pass on the panel.
 *
 * @param  uint8_t c
 *         Pixel value to write (0 = black, 1 = white, 2 = discharge, 3 = skip).
 * @param  uint8_t rep
 *         Number of times to repeat the pass.
 */
void Inkplate4::clean(uint8_t c, uint8_t rep)
{
}

/**
 * @brief  Set all e-ink data and control GPIOs to output mode.
 */
void Inkplate4::pinsAsOutputs()
{
  gpio_set_direction(GPIO_NUM_0,  GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NUM_2,  GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NUM_32, GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NUM_33, GPIO_MODE_OUTPUT);

  expander1.setDirection(IO_NUM_A0, IO_MODE_OUTPUT);
  expander1.setDirection(IO_NUM_A1, IO_MODE_OUTPUT);
  expander1.setDirection(IO_NUM_A2, IO_MODE_OUTPUT);

  gpio_set_direction(GPIO_NUM_4,  GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NUM_5,  GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NUM_18, GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NUM_19, GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NUM_23, GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NUM_25, GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NUM_26, GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NUM_27, GPIO_MODE_OUTPUT);
}

/**
 * @brief  Set all e-ink data and control GPIOs to high-impedance (input) mode.
 */
void Inkplate4::pinsZstate()
{
  m_i2s->conf1.tx_stop_en = 0;

  gpio_set_direction(GPIO_NUM_2,  GPIO_MODE_INPUT);
  gpio_set_direction(GPIO_NUM_32, GPIO_MODE_INPUT);
  gpio_set_direction(GPIO_NUM_33, GPIO_MODE_INPUT);

  expander1.setDirection(IO_NUM_A0, IO_MODE_INPUT);
  expander1.setDirection(IO_NUM_A1, IO_MODE_INPUT);
  expander1.setDirection(IO_NUM_A2, IO_MODE_INPUT);

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
