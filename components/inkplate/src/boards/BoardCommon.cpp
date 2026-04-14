#include "string.h"
#include "driver/gpio.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_rom_sys.h"
#include "nvs_flash.h"
#include "nvs.h"

// Include the active board's pins.h for WAKEUP_SET/CLEAR, CKV_SET, LE_SET, etc.
#if CONFIG_INKPLATE_BOARD_INKPLATE6
  #include "inkplate6/pins.h"
#elif CONFIG_INKPLATE_BOARD_INKPLATE6COLOR
  #include "inkplate6color/pins.h"
#elif CONFIG_INKPLATE_BOARD_INKPLATE10
  #include "inkplate10/pins.h"
#elif CONFIG_INKPLATE_BOARD_INKPLATE5
  #include "inkplate5/pins.h"
#elif CONFIG_INKPLATE_BOARD_INKPLATE4
  #include "inkplate4/pins.h"
#elif CONFIG_INKPLATE_BOARD_INKPLATE2
  #include "inkplate2/pins.h"
#endif

#include "BoardCommon.h"
#include "I2C.h"
#include "PCAL.h"
#include "TPS.h"
#include "SDCard.h"
#include "GraphicsDefs.h"

#define NVS_NAMESPACE "inkplate"
#define NVS_VCOM_KEY  "vcom"

static const char *TAG = "INKPLATE";

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE2
#define _swap_int16_t(a, b) { int16_t t = (a); (a) = (b); (b) = t; }

/**
 * ============================================================
 * Global peripheral instances — shared by all board implementations
 * ============================================================
 */

I2C     i2c;
PCAL    expander1(IO_INT_ADDR, i2c);
#if !defined(CONFIG_INKPLATE_BOARD_INKPLATE5) && !defined(CONFIG_INKPLATE_BOARD_INKPLATE6COLOR)
PCAL    expander2(IO_EXT_ADDR, i2c);
#endif
TPS     tps(i2c);
SDCard  sdCard(expander1);
#endif 

/**
 * ============================================================
 * Public functions
 * ============================================================
 */

/**
 * @brief  BoardCommon constructor.
 *
 * @param  uint16_t einkWidth
 *         Panel width in pixels.
 * @param  uint16_t einkHeight
 *         Panel height in pixels.
 * @param  uint8_t cleanCycles1
 *         Number of full-white cleaning passes in cleanBurnIn().
 * @param  uint8_t cleanCycles0
 *         Number of full-black cleaning passes in cleanBurnIn().
 */

BoardCommon::BoardCommon(uint16_t einkWidth, uint16_t einkHeight,
                         uint8_t cleanCycles1, uint8_t cleanCycles0)
  : m_einkWidth(einkWidth),
    m_einkHeight(einkHeight),
    m_cleanCycles1(cleanCycles1),
    m_cleanCycles0(cleanCycles0)
{
}

/**
 * @brief  Select the active display mode (black-and-white or grayscale).
 *
 * @param  displayMode_t mode
 *         BLACK_AND_WHITE or GRAYSCALE.
 */
void BoardCommon::setDisplayMode(displayMode_t mode)
{
  const char *name;
  if (mode == BLACK_AND_WHITE)
    name = "Black and white";
  else if (mode == GRAYSCALE)
    name = "Grayscale";
  else
    name = "Wrong display mode selected, defaulting to grayscale.";

  ESP_LOGI(TAG, "Selected display mode: %s", name);
  m_displayMode = mode;
}

/**
 * @brief  Fill the framebuffer with white (erase all content).
 */
void BoardCommon::clearDisplay()
{
  if (m_displayMode == BLACK_AND_WHITE)
    memset(m_newFramebuffer,   0x00, m_einkWidth * m_einkHeight / 8);
  else if (m_displayMode == GRAYSCALE)
    memset(m_framebufferColor, 0xFF, m_einkWidth * m_einkHeight / 2);

  ESP_LOGI(TAG, "Display cleared.");
}

/**
 * @brief  Fill the framebuffer with black (all pixels on).
 */
void BoardCommon::fillDisplay()
{
  if (m_displayMode == BLACK_AND_WHITE)
    memset(m_newFramebuffer,   0xFF, m_einkWidth * m_einkHeight / 8);
  else if (m_displayMode == GRAYSCALE)
    memset(m_framebufferColor, 0x00, m_einkWidth * m_einkHeight / 2);

  ESP_LOGI(TAG, "Display filled.");
}

/**
 * @brief  Write a single pixel into the framebuffer after applying display rotation.
 *
 * @param  int16_t x
 *         Logical X coordinate.
 * @param  int16_t y
 *         Logical Y coordinate.
 * @param  uint16_t color
 *         Pixel value (0–7 for grayscale; 0 or 1 for B&W).
 */
void BoardCommon::writePixelInternal(int16_t x, int16_t y, uint16_t color)
{
  int16_t x0 = x, y0 = y;

  uint8_t r = getRotation();
  int16_t logW = (r == 1 || r == 3) ? m_einkHeight : m_einkWidth;
  int16_t logH = (r == 1 || r == 3) ? m_einkWidth  : m_einkHeight;
  if (x0 < 0 || y0 < 0 || x0 >= logW || y0 >= logH)
    return;

  switch (r)
  {
  case 1:
    _swap_int16_t(x0, y0);
    x0 = m_einkWidth - x0 - 1;
    break;
  case 2:
    x0 = m_einkWidth  - x0 - 1;
    y0 = m_einkHeight - y0 - 1;
    break;
  case 3:
    _swap_int16_t(x0, y0);
    y0 = m_einkHeight - y0 - 1;
    break;
  default:
    break;
  }

  #ifndef CONFIG_INKPLATE_BOARD_INKPLATE6COLOR
  if (m_displayMode == BLACK_AND_WHITE)
  {
    int x1    = x0 >> 3;
    int x_sub = x0 & 7;
    uint8_t temp = *(m_newFramebuffer + (m_einkWidth / 8) * y0 + x1);
    *(m_newFramebuffer + (m_einkWidth / 8) * y0 + x1) =
      (~pixelMaskLUT[x_sub] & temp) | (color ? pixelMaskLUT[x_sub] : 0);
  }
  else if (m_displayMode == GRAYSCALE)
  {
    color &= 7;
    int x1    = x0 >> 1;
    int x_sub = x0 & 1;
    uint8_t temp = *(m_framebufferColor + (m_einkWidth / 2) * y0 + x1);
    *(m_framebufferColor + (m_einkWidth / 2) * y0 + x1) =
      (pixelMaskGLUT[x_sub] & temp) | (x_sub ? color : color << 4);
  }
  #else
    int x1 = x0 / 2;
    int xSub = x0 % 2;
    uint8_t temp = *(m_framebufferColor + m_einkWidth / 2 * y0 + x1);
    *(m_framebufferColor + m_einkWidth / 2 * y0 + x1) = (pixelMaskGLUT[xSub] & temp) | (xSub ? color : color << 4);
  #endif
}

/**
 * @brief  Push the framebuffer to the e-ink panel.
 *
 * @param  bool leaveOn
 *         If true, leave the panel powered on after the update.
 *
 * @return esp_err_t
 *         ESP_OK on success, or an error code from the active display driver.
 */
esp_err_t BoardCommon::display(bool leaveOn)
{
  esp_err_t ret = ESP_OK;
  if (m_displayMode == BLACK_AND_WHITE)
    ret = display1b(leaveOn);
  else if (m_displayMode == GRAYSCALE)
    ret = display3b(leaveOn);

  ESP_LOGI(TAG, "Content displayed.");
  return ret;
}

/**
 * @brief  Mark all PMIC control pins on expander1 as blocked so user code
 *         cannot accidentally modify them via the PCAL API.
 *
 * @note   Called automatically from each board constructor after gpioInit().
 */
void BoardCommon::blockGpioPins()
{
  #ifndef CONFIG_INKPLATE_BOARD_INKPLATE6COLOR
  expander1.blockPin(WAKEUP);
  expander1.blockPin(PWRUP);
  expander1.blockPin(VCOM);
  expander1.blockPin(OE);
  expander1.blockPin(GMOD);
  expander1.blockPin(SPV);
  #endif
}

/**
 * @brief  Set the number of partial updates allowed before a forced full refresh.
 *
 * @param  uint16_t numberOfPartialUpdates
 *         Maximum consecutive partial updates; 0 disables the limit.

 */
void BoardCommon::setFullUpdateThreshold(uint16_t numberOfPartialUpdates)
{
  m_partialUpdateLimiter = numberOfPartialUpdates;

  if (numberOfPartialUpdates != 0)
    m_blockPartial = true;
}

/**
 * @brief  Run multiple full-panel cleaning cycles to reduce burn-in.
 *
 * @param  uint8_t clearCycles
 *         Number of cleaning iterations.
 * @param  uint16_t cyclesDelay
 *         Delay in milliseconds between iterations.
 */
void BoardCommon::cleanBurnIn(uint8_t clearCycles, uint16_t cyclesDelay)
{
  einkOn();

  while (clearCycles)
  {
    clean(1, m_cleanCycles1);
    clean(2, 1);
    clean(0, m_cleanCycles0);
    clean(2, 1);
    clean(1, m_cleanCycles1);
    clean(2, 1);
    clean(0, m_cleanCycles0);
    clean(2, 1);

    esp_rom_delay_us(cyclesDelay * 1000);
    clearCycles--;
  }
}

/**
 * @brief  Read the LiPo battery voltage via the ESP32 ADC.
 *
 * @return double
 *         Battery voltage in volts (approximately 3.0–4.2 V when charged).
 */
double BoardCommon::readBattery()
{
  expander1.setLevel(IO_NUM_B1, 1, true);
  esp_rom_delay_us(5000);

  adc_oneshot_unit_handle_t adcHandle;
  adc_oneshot_unit_init_cfg_t initCfg = {};
  initCfg.unit_id = ADC_UNIT_1;
  adc_oneshot_new_unit(&initCfg, &adcHandle);

  adc_oneshot_chan_cfg_t chanCfg = {};
  chanCfg.atten    = ADC_ATTEN_DB_12;
  chanCfg.bitwidth = ADC_BITWIDTH_12;
  adc_oneshot_config_channel(adcHandle, ADC_CHANNEL_7, &chanCfg);

  adc_cali_handle_t caliHandle = NULL;
  adc_cali_line_fitting_config_t caliCfg = {};
  caliCfg.unit_id  = ADC_UNIT_1;
  caliCfg.atten    = ADC_ATTEN_DB_12;
  caliCfg.bitwidth = ADC_BITWIDTH_12;
  bool calibrated = (adc_cali_create_scheme_line_fitting(&caliCfg, &caliHandle) == ESP_OK);

  int raw = 0, mv = 0;
  adc_oneshot_read(adcHandle, ADC_CHANNEL_7, &raw);
  if (calibrated)
  {
    adc_cali_raw_to_voltage(caliHandle, raw, &mv);
    adc_cali_delete_scheme_line_fitting(caliHandle);
  }
  adc_oneshot_del_unit(adcHandle);

  expander1.setLevel(IO_NUM_B1, 0, true);

  return (double(mv) * 2.0 / 1000.0);
}

/**
 * @brief  Program a new VCOM voltage into the TPS65186 and persist it in NVS.
 *
 * @param  double vcom
 *         VCOM in volts; must be in the range -5.0 to 0.0.
 *
 * @return esp_err_t
 *         ESP_OK on success, ESP_ERR_INVALID_ARG if out of range,
 *         or an error from einkOn()/TPS.
 */
esp_err_t BoardCommon::setVCOM(double vcom)
{
  if (vcom < -5.0 || vcom > 0.0)
    return ESP_ERR_INVALID_ARG;

  esp_err_t ret = einkOn();
  if (ret != ESP_OK)
    return ret;

  ret = tps.writeVCOM(vcom, expander1);
  einkOff();

  if (ret != ESP_OK)
    return ret;

  nvs_handle_t nvs;
  if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs) == ESP_OK)
  {
    int32_t stored = (int32_t)(vcom * 100.0);
    nvs_set_i32(nvs, NVS_VCOM_KEY, stored);
    nvs_commit(nvs);
    nvs_close(nvs);
  }

  return ESP_OK;
}

/**
 * @brief  Read the VCOM voltage stored in NVS (written by setVCOM()).
 *
 * @return double
 *         Stored VCOM in volts, or 0.0 if no value has been saved.
 */
double BoardCommon::getVCOM()
{
  nvs_handle_t nvs;
  if (nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs) != ESP_OK)
    return 0.0;

  int32_t stored = 0;
  esp_err_t ret = nvs_get_i32(nvs, NVS_VCOM_KEY, &stored);
  nvs_close(nvs);

  if (ret != ESP_OK)
    return 0.0;

  return stored / 100.0;
}

/**
 * @brief  Read the e-ink panel temperature from the TPS65186 thermistor.
 *
 * @return int8_t
 *         Temperature in degrees Celsius, or 0 if the panel could not be powered on.
 *
 * @note   Briefly powers the panel on to communicate with the TPS65186.
 */
int8_t BoardCommon::readTemperature()
{
  if (einkOn() != ESP_OK)
    return 0;

  int8_t temp = tps.readTemperature();
  einkOff();
  ESP_LOGI(TAG, "Temperature: %d", temp);
  return temp;
}

/**
 * @brief  Read the VCOM currently programmed into the TPS65186 hardware registers.
 *
 * @return double
 *         Live VCOM in volts, or 0.0 on error.
 *
 * @note   Briefly powers the panel on to read the TPS registers.
 */
double BoardCommon::getStoredVCOM()
{
  if (einkOn() != ESP_OK)
    return 0.0;

  double vcom = tps.readVCOM();
  einkOff();
  return vcom;
}

/**
 * @brief  Mount the SD card and make it accessible via the VFS.
 *
 * @return esp_err_t
 *         ESP_OK on success, or an SD driver error code.
 */
esp_err_t BoardCommon::sdCardInit()
{
  return sdCard.sdCardInit();
}

/**
 * @brief  Put the SD card into low-power sleep mode.
 *
 * @return esp_err_t
 *         ESP_OK on success, or an SD driver error code.
 */
esp_err_t BoardCommon::sdCardSleep()
{
  return sdCard.sdCardSleep();
}

/**
 * @brief  Return the VFS mount point for the SD card (e.g. "/sdcard").
 *
 * @return const char*
 *         Null-terminated mount point string.
 */
const char* BoardCommon::getMountPoint()
{
  return sdCard.getMountPoint();
}

/**
 * ============================================================
 * Protected functions
 * ============================================================
 */

/**
 * @brief  Assert the vertical scan start pulse (SPV strobed via CKV).
 *
 * @note   Timing values derived from ED060SC4 panel datasheet.
 */
void BoardCommon::vscanStart()
{
  #ifndef CONFIG_INKPLATE_BOARD_INKPLATE6COLOR
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
  #endif
}

/**
 * @brief  Latch the current scan line into the panel (CKV low, LE pulse).
 */
void BoardCommon::vscanEnd()
{
  #ifndef CONFIG_INKPLATE_BOARD_INKPLATE6COLOR
  CKV_CLEAR;
  LE_SET;
  LE_CLEAR;
  esp_rom_delay_us(0);
  #endif
}

/**
 * @brief  Set the cached panel power state.
 *
 * @param  bool state
 *         true = panel on, false = panel off.
 */
void BoardCommon::setPanelState(bool state)
{
  m_panelState = state;
}

/**
 * @brief  Return the cached panel power state.
 *
 * @return bool
 *         true = panel on, false = panel off.
 */
bool BoardCommon::getPanelState()
{
  return m_panelState;
}

/**
 * @brief  Initialise the TPS65186 PMIC with the default power sequences.
 *
 * @return esp_err_t
 *         ESP_OK on success, or an I2C error code.
 *
 * @note   Pulses WAKEUP briefly to allow I2C communication with the TPS65186.
 */
esp_err_t BoardCommon::pmicBegin()
{
  #ifndef CONFIG_INKPLATE_BOARD_INKPLATE6COLOR
  WAKEUP_SET;
  esp_rom_delay_us(1000);
  esp_err_t ret = tps.initSequences();
  esp_rom_delay_us(1000);
  WAKEUP_CLEAR;
  return ret;
  #else
  return ESP_OK;
  #endif
}