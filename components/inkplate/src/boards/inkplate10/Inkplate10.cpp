#include "driver/gpio.h"
#include "esp_heap_caps.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "string.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"

#include "Inkplate10.h"
#include "I2C.h"

#define NVS_NAMESPACE "inkplate"
#define NVS_VCOM_KEY  "vcom"

static const char* TAG = "ESP_INKPLATE10";

I2C     i2c;
PCAL    expander1(IO_INT_ADDR, i2c);
PCAL    expander2(IO_EXT_ADDR, i2c);
TPS     tps(i2c);
SDCard  sdCard(expander1);


/**
 * ============================================================
 * Public functions
 * ============================================================
 */

/**
 * @brief  Inkplate6 constructor.
 *
 * @note   Allocates framebuffer and DMA buffers and pre-computes the grayscale waveform LUTs.
 */
Inkplate10::Inkplate10() : rtc(i2c)
{
  ESP_ERROR_CHECK(initBuffers());
  calculateLUTs();

  gpioInit();
  ESP_ERROR_CHECK(pmicBegin());

  ESP_LOGI(TAG, "Inkplate6 initilization finished!");
}

/**
 * @brief  Set the active display mode.
 *
 * @param  displayMode_t mode
 *         BLACK_AND_WHITE for 1-bit mode, GRAYSCALE for 3-bit mode.
 */
void Inkplate10::setDisplayMode(displayMode_t mode)
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
 * @brief  Write a single pixel into the framebuffer.
 *
 * @param  int16_t x
 *         Pixel X coordinate (0 = left).
 * @param  int16_t y
 *         Pixel Y coordinate (0 = top).
 * @param  uint16_t color
 *         In BLACK_AND_WHITE: 0 = white, non-zero = black.
 *         In GRAYSCALE: 0–7 grey level (0 = white, 7 = black).
 *
 * @note   Out-of-bounds coordinates are ignored.
 */
#define _swap_int16_t(a, b) { int16_t t = (a); (a) = (b); (b) = t; }

void Inkplate10::writePixelInternal(int16_t x, int16_t y, uint16_t color)
{
  int16_t x0 = x, y0 = y;

  // bounds check against logical (rotation-aware) dimensions
  uint8_t r = getRotation();
  int16_t logW = (r == 1 || r == 3) ? E_INK_HEIGHT : E_INK_WIDTH;
  int16_t logH = (r == 1 || r == 3) ? E_INK_WIDTH  : E_INK_HEIGHT;
  if (x0 < 0 || y0 < 0 || x0 >= logW || y0 >= logH)
  return;

  // transform logical to physical coordinates
  switch (r)
  {
  case 1: // 90° left
  _swap_int16_t(x0, y0);
  x0 = E_INK_WIDTH - x0 - 1;
  break;
  case 2: // 180°
  x0 = E_INK_WIDTH  - x0 - 1;
  y0 = E_INK_HEIGHT - y0 - 1;
  break;
  case 3: // 90° right
  _swap_int16_t(x0, y0);
  y0 = E_INK_HEIGHT - y0 - 1;
  break;
  default: // 0° — no transform
  break;
  }

  // write to buffers
  if (m_displayMode == BLACK_AND_WHITE)
  {
  int x1 = x0 >> 3;
  int x_sub = x0 & 7;
  uint8_t temp = *(m_newFramebuffer + (E_INK_WIDTH >> 3) * y0 + x1);
  *(m_newFramebuffer + (E_INK_WIDTH / 8 * y0) + x1) = (~pixelMaskLUT[x_sub] & temp) | (color ? pixelMaskLUT[x_sub] : 0);
  }
  else if (m_displayMode == GRAYSCALE)
  {
  color &= 7;
  int x1 = x0 >> 1;
  int x_sub = x0 & 1;
  uint8_t temp = *(m_framebufferColor + (E_INK_WIDTH >> 1) * y0 + x1);
  *(m_framebufferColor + (E_INK_WIDTH >> 1) * y0 + x1) = (pixelMaskGLUT[x_sub] & temp) | (x_sub ? color : color << 4);
  }
}

/**
 * @brief  Clear the framebuffer to white.
 */
void Inkplate10::clearDisplay()
{
  if (m_displayMode == BLACK_AND_WHITE)
  memset(m_newFramebuffer,   0x00, E_INK_WIDTH * E_INK_HEIGHT / 8);
  else if (m_displayMode == GRAYSCALE)
  memset(m_framebufferColor, 0xFF, E_INK_WIDTH * E_INK_HEIGHT / 2);

  ESP_LOGI(TAG, "Display cleared.");
}

/**
 * @brief  Fill the framebuffer to black.
 */
void Inkplate10::fillDisplay()
{
  if (m_displayMode == BLACK_AND_WHITE)
  memset(m_newFramebuffer,   0xFF, E_INK_WIDTH * E_INK_HEIGHT / 8);
  else if (m_displayMode == GRAYSCALE)
  memset(m_framebufferColor, 0x00, E_INK_WIDTH * E_INK_HEIGHT / 2);

  ESP_LOGI(TAG, "Display filled.");
}

/**
 * @brief  Display buffer to screen.
 */
esp_err_t Inkplate10::display(bool leaveOn)
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
 * @brief  Turn on epaper power supply (TPS65186).
 *
 * @return esp_err_t
 *         ESP_OK on success, ESP_ERR_TIMEOUT if power good not reached.
 *
 * @note   Power-on order matters — wrong order can damage the display.
 */
esp_err_t Inkplate10::einkOn()
{
  if (getPanelState())
  return ESP_OK;

  WAKEUP_SET;
  esp_rom_delay_us(5000);

  tps.enableRails();
  tps.setPowerUpSequence(TPS_PWRUP_SEQ);
  tps.setPowerDownSequence(TPS_PWRDN_SEQ);

  pinsAsOutputs();
  LE_CLEAR;
  CL_CLEAR;
  SPH_SET;
  GMOD_SET;
  SPV_SET;
  CKV_CLEAR;
  OE_CLEAR;
  PWRUP_SET;
  setPanelState(true);

  if (!tps.waitPowerGood(true))
  {
  einkOff();
  return ESP_ERR_TIMEOUT;
  }

  ESP_LOGI(TAG, "Eink turned on.");

  VCOM_SET;
  OE_SET;
  return ESP_OK;
}

/**
 * @brief  Turn off epaper power supply and put all IO pins in high-Z state.
 *
 * @return esp_err_t
 *         ESP_OK on success, or an I2C driver error code.
 */
esp_err_t Inkplate10::einkOff()
{
  if (!getPanelState())
  return ESP_OK;

  VCOM_CLEAR;
  OE_CLEAR;
  GMOD_CLEAR;
  GPIO.out &= ~(DATA | LE | CL);
  CKV_CLEAR;
  SPH_CLEAR;
  SPV_CLEAR;
  PWRUP_CLEAR;

  tps.waitPowerGood(false);

  WAKEUP_CLEAR;
  esp_err_t ret = tps.disableRails();

  pinsZstate();
  setPanelState(false);

  ESP_LOGI(TAG, "Eink turned off.");
  return ret;
}

/**
 * @brief  Send only the changed pixels to the display (1-bit mode only).
 *
 * @param  bool forced  
 *         if true, force update
 * @param  bool leaveOn
 *         if true, leave the eink power supply on after the update
 *
 * @return uint32_t
 *         Number of pixels that transitioned from black to white.
 *         Returns 0 if a full refresh was triggered instead, or if
 *         GRAYSCALE mode is active (not supported).
 *
 * @note   After m_partialUpdateLimiter partial updates, a full display1b refresh
 *         is forced automatically to clear accumulated ghosting.
 */
uint32_t Inkplate10::partialUpdate(bool forced, bool leaveOn)
{
  // grayscale not supported
  if (m_displayMode == GRAYSCALE)
  {
  ESP_LOGI(TAG, "Selected display mode does not support partial updating.");
  return 0;
  }

  if (m_blockPartial && !forced)
  {
  display1b(leaveOn);
  return 0;
  }

  if (m_partialUpdateCounter >= m_partialUpdateLimiter && m_partialUpdateLimiter != 0)
  {
  ESP_LOGI(TAG, "Partial update limit reached, forcing full update.");
  // force full update
  display1b(leaveOn);
  // reset the counter
  m_partialUpdateCounter = 0;
  return 0;
  }

  uint32_t position = (E_INK_WIDTH * E_INK_HEIGHT / 8) - 1;
  uint32_t n = (E_INK_WIDTH * E_INK_HEIGHT / 4) - 1;
  uint8_t diffWhite, diffBlack;
  uint8_t data = 0;
  uint32_t send;

  uint32_t changeCount = 0;
  
  for (int i = 0; i < E_INK_HEIGHT; i++)
  {
  for (int j = 0; j < E_INK_WIDTH / 8; j++)
  {
    diffWhite = *(m_framebuffer + position) & ~*(m_newFramebuffer + position);
    diffBlack = ~*(m_framebuffer + position) & *(m_newFramebuffer + position);
    // count pixels turning from black to white as these are visible blur
    if (diffWhite)
    {
    for (int bv = 1; bv < 256; bv <<=1)
    {
      if (diffWhite & bv)
      changeCount++;
    }
    }

    position--;
    *(m_waveformBuffer + n) = LUTW[diffWhite >> 4] & LUTB[diffBlack >> 4];
    n--;
    *(m_waveformBuffer + n) = LUTW[diffWhite & 0x0F] & LUTB[diffBlack & 0x0F];
    n--;
  }
  }

  if (einkOn() != ESP_OK)
  return 0;

  uint8_t rep = 5;

  for (int k = 0; k < rep; k++)
  {
  vscanStart();
  n = (E_INK_WIDTH * E_INK_HEIGHT / 4) - 1;

  for (int i = 0; i < E_INK_HEIGHT; i++)
  {
    data = *(m_waveformBuffer + n);
    send = m_pinLUT[data];
    hscanStart(send);
    n--;
    
    for (int j = 0; j < (E_INK_WIDTH / 4); j += 4)
    {
      data = *(m_waveformBuffer + n);
      send = m_pinLUT[data];
      GPIO.out_w1ts = send | CL;
      GPIO.out_w1tc = DATA | CL;
      n--;
    }
    GPIO.out_w1ts = send | CL;
    GPIO.out_w1tc = DATA | CL;
    vscanEnd();
  }
  esp_rom_delay_us(230);
  }

  clean(2, 2);
  clean(3, 1);
  vscanStart();

  if (einkOn() != ESP_OK)
  einkOff();

  memcpy(m_framebuffer, m_newFramebuffer, E_INK_WIDTH * E_INK_HEIGHT / 8);

  if (m_partialUpdateLimiter != 0)
  m_partialUpdateCounter++;

  return changeCount;
}

/**
 * @brief   Set the number of partial updates afterwhich full screen update is performed.
 *
 * @param   uint16_t numberOfPartialUpdates
 *          Number of allowed partial updates afterwhich full update is performed.
 *          0 = disabled, no automatic full update will be performed.
 *
 * @note    By default, this is disabled, but to keep best image quality perform a full update
 *          every 60-80 partial updates.
 */
void Inkplate10::setFullUpdateThreshold(uint16_t numberOfPartialUpdates)
{
  m_partialUpdateLimiter = numberOfPartialUpdates;

  if (numberOfPartialUpdates != 0)
  m_blockPartial = true;
}

/**
 * @brief   Cleans the screen of any potential burn in by writing a clear sequence to the panel.
 *
 * @param   uint8_t clearCycles
 *          number of clear cycles
 * @param   uint16_t cyclesDelay
 *          delay between clear cycles (in milliseconds)
 *
 * @note    Cycles delay should not be smaller than 5 seconds
 */
void Inkplate10::cleanBurnIn(uint8_t clearCycles, uint16_t cyclesDelay)
{
  einkOn();

  while (clearCycles)
  {
    clean(1, 12);
    clean(2, 1);
    clean(0, 9);
    clean(2, 1);
    clean(1, 12);
    clean(2, 1);
    clean(0, 9);
    clean(2, 1);

    esp_rom_delay_us(cyclesDelay * 1000);
    clearCycles--;
  }
}

/**
 * @brief  Read the battery voltage.
 *
 * @note   Briefly enables the voltage divider MOSFET, reads ADC1 channel 7
 *         (GPIO35), then disables the divider. 
 *
 * @return double
 *         Battery voltage in volts, or 0.0 if ADC calibration is unavailable.
 */
double Inkplate10::readBattery()
{
  // enable voltage divider
  expander1.setLevel(IO_NUM_B1, 1);
  esp_rom_delay_us(5000);

  // init ADC oneshot unit
  adc_oneshot_unit_handle_t adcHandle;
  adc_oneshot_unit_init_cfg_t initCfg = {};
  initCfg.unit_id = ADC_UNIT_1;
  adc_oneshot_new_unit(&initCfg, &adcHandle);

  adc_oneshot_chan_cfg_t chanCfg = {};
  chanCfg.atten    = ADC_ATTEN_DB_12;
  chanCfg.bitwidth = ADC_BITWIDTH_12;
  adc_oneshot_config_channel(adcHandle, ADC_CHANNEL_7, &chanCfg);

  // calibrate
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

  // disable voltage divider
  expander1.setLevel(IO_NUM_B1, 0);

  // voltage is divided by 2 on the board, so multiply back
  return (double(mv) * 2.0 / 1000.0);
}

/**
 * @brief  Initialise the SD card.
 *
 * @return esp_err_t
 *          ESP_OK on success, or an error code from the SPI/VFS driver
 */
esp_err_t Inkplate10::sdCardInit()
{
  return sdCard.sdCardInit();
}

/**
 * @brief  Send SD card to sleep.
 *
 * @return esp_err_t
 *         ESP_OK on success, or an error code if unmounting failed
 */
esp_err_t Inkplate10::sdCardSleep()
{
  return sdCard.sdCardSleep();
}

/**
 * @brief  Get the mount point string for constructing file paths.
 *
 * @return const char*
 *         Mount point, e.g. "/sdcard".
 */
const char* Inkplate10::getMountPoint()
{
  return sdCard.getMountPoint();
}

/**
 * @brief  Program VCOM voltage into the PMIC and persist it in NVS.
 *
 * @param  double vcom
 *         VCOM value in volts (must be in range -5.0 to 0.0).
 *
 * @return ESP_OK on success, ESP_ERR_INVALID_ARG if out of range,
 *         ESP_FAIL if EEPROM write verification fails.
 */
esp_err_t Inkplate10::setVCOM(double vcom)
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

  // Persist to NVS
  nvs_handle_t nvs;
  if (nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs) == ESP_OK)
  {
    // Store as fixed-point int32 (millivolts × 100 for 2 decimal places)
    int32_t stored = (int32_t)(vcom * 100.0);
    nvs_set_i32(nvs, NVS_VCOM_KEY, stored);
    nvs_commit(nvs);
    nvs_close(nvs);
  }

  return ESP_OK;
}

/**
 * @brief  Read the VCOM voltage stored in NVS (set by a previous setVCOM call).
 *
 * @return VCOM in volts (negative), or 0.0 if not set.
 */
double Inkplate10::getVCOM()
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
 * @brief  Read the VCOM voltage currently programmed in the PMIC registers.
 *
 * @return VCOM in volts (negative).
 *
 * @note   Turns eink on briefly to read registers, then turns it off.
 */
double Inkplate10::getStoredVCOM()
{
  if (einkOn() != ESP_OK)
    return 0.0;

  double vcom = tps.readVCOM();
  einkOff();
  return vcom;
}

/**
 * ============================================================
 * Private functions
 * ============================================================
 */

/**
 * @brief  Allocate and zero-initialise all framebuffers and DMA descriptors.
 *
 * @return esp_err_t
 *         ESP_OK on success, ESP_ERR_NO_MEM if any allocation fails.
 *
 */
esp_err_t Inkplate10::initBuffers()
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
 * @brief  Pre-compute m_glut and m_glut2 waveform lookup tables.
 */
void Inkplate10::calculateLUTs()
{
  for (int j = 0; j < 9; ++j)
  {
    for (int i = 0; i < 256; ++i)
    {
      uint8_t z = (waveform3Bit[i & 0x07][j] << 2) | (waveform3Bit[(i >> 4) & 0x07][j]);
      m_glut[j * 256 + i]  = ((z & 0B00000011) << 4) | (((z & 0B00001100) >> 2) << 18) |
                           (((z & 0B00010000) >> 4) << 23) | (((z & 0B11100000) >> 5) << 25);
      z = ((waveform3Bit[i & 0x07][j] << 2) | (waveform3Bit[(i >> 4) & 0x07][j])) << 4;
      m_glut2[j * 256 + i] = ((z & 0B00000011) << 4) | (((z & 0B00001100) >> 2) << 18) |
                           (((z & 0B00010000) >> 4) << 23) | (((z & 0B11100000) >> 5) << 25);
    }
  }
}

/**
 * @brief  Push the framebuffer to the display using 3-bit (8-level) grayscale.
 *
 * @param  bool leaveOn
 *         if true, leave the eink power supply on after the update
 */
esp_err_t Inkplate10::display3b(bool leaveOn)
{
  esp_err_t ret = einkOn();
  if (ret != ESP_OK)
  {
  ESP_LOGI(TAG, "Display is not on!");
  return ret;
  }

  clean(1, 1);
  clean(0, 10);
  clean(2, 1);
  clean(1, 10);
  clean(2, 1);
  clean(0, 10);
  clean(2, 1);
  clean(1, 10);

  for (int k = 0; k < 9; k++)
  {
  uint8_t *dp = m_framebufferColor + E_INK_WIDTH * E_INK_HEIGHT / 2;

  vscanStart();
  for (int i = 0; i < E_INK_HEIGHT; i++)
  {
    uint32_t t = m_glut2[k * 256 + (*(--dp))];
    t |= m_glut[k * 256 + (*(--dp))];
    hscanStart(t);
    t = m_glut2[k * 256 + (*(--dp))];
    t |= m_glut[k * 256 + (*(--dp))];
    GPIO.out_w1ts = t | CL;
    GPIO.out_w1tc = DATA | CL;

    for (int j = 0; j < ((E_INK_WIDTH / 8) - 1); j++)
    {
      t = m_glut2[k * 256 + (*(--dp))];
      t |= m_glut[k * 256 + (*(--dp))];
      GPIO.out_w1ts = t | CL;
      GPIO.out_w1tc = DATA | CL;
      t = m_glut2[k * 256 + (*(--dp))];
      t |= m_glut[k * 256 + (*(--dp))];
      GPIO.out_w1ts = t | CL;
      GPIO.out_w1tc = DATA | CL;
    }

    GPIO.out_w1ts = CL;
    GPIO.out_w1tc = DATA | CL;
   vscanEnd();
  }
  esp_rom_delay_us(230);
  }

  clean(3, 1);
  vscanStart();

  if (!leaveOn)
  einkOff();

  return ESP_OK;
}

/**
 * @brief  Push the framebuffer to the display using 1-bit (black and white).
 *
 * @param  bool leaveOn
 *         if true, leave the eink power supply on after the update
 */
esp_err_t Inkplate10::display1b(bool leaveOn)
{
  esp_err_t ret = einkOn();
  if (ret != ESP_OK)
  {
  ESP_LOGI(TAG, "Display is not on!");
  return ret;
  }

  uint32_t pos;
  uint8_t data;
  uint8_t dram;

  clean(0, 1);
  clean(1, 18);
  clean(2, 1);
  clean(0, 18);
  clean(2, 1);
  clean(1, 18);
  clean(2, 1);
  clean(0, 18);
  clean(2, 1);

  memcpy(m_framebuffer, m_newFramebuffer, E_INK_WIDTH * E_INK_HEIGHT / 8);

  int rep = 5;

  for (int k = 0; k < rep; k++)
  {
      pos = (E_INK_HEIGHT * E_INK_WIDTH / 8) - 1;
      vscanStart();
      for (int i = 0; i < E_INK_HEIGHT; i++)
      {
          dram = (*(m_framebuffer + pos));
          data = LUTB[(dram >> 4) & 0x0F];
          hscanStart(m_pinLUT[data]);
          data = LUTB[dram & 0x0F];
          GPIO.out_w1ts = m_pinLUT[data] | CL;
          GPIO.out_w1tc = DATA | CL;
          pos--;
          for (int j = 0; j < ((E_INK_WIDTH / 8) - 1); j++)
          {
              dram = (*(m_framebuffer + pos));
              data = LUTB[(dram >> 4) & 0x0F];
              GPIO.out_w1ts = m_pinLUT[data] | CL;
              GPIO.out_w1tc = DATA | CL;
              data = LUTB[dram & 0x0F];
              GPIO.out_w1ts = m_pinLUT[data] | CL;
              GPIO.out_w1tc = DATA | CL;
              pos--;
          }
          GPIO.out_w1ts = CL;
          GPIO.out_w1tc = DATA | CL;
          vscanEnd();
      }
        esp_rom_delay_us(230);
  }

  clean(2, 2);
  clean(3, 1);

  vscanStart();
  if (!leaveOn)
      einkOff();

  m_blockPartial = false;
  return ESP_OK;
}

/**
 * @brief  Starts writing data into current row.
 *
 * @param  uint32_t data
 *         data to be written into current row
 */
void Inkplate10::hscanStart(uint32_t data)
{
  SPH_CLEAR;
  GPIO.out_w1ts = (data) | CL;
  GPIO.out_w1tc = DATA | CL;
  SPH_SET;
  CKV_SET;
}

/**
 * @brief  Start a new frame by pulsing SPV/CKV to initialise the gate driver.
 */
void Inkplate10::vscanStart()
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
void Inkplate10::vscanEnd()
{
  CKV_CLEAR;
  LE_SET;
  LE_CLEAR;
  esp_rom_delay_us(0);
}

/**
 * @brief  Configure all GPIO, IO expander pins, and the pixel-to-GPIO LUT.
 *
 * @note   Sets SPI pins as inputs to reduce deep-sleep current on V2, configures
 *         expander1 EPD control signals, sets control and data GPIO pins as outputs,
 *         and initialises the expander2 data bus.
 */
void Inkplate10::gpioInit()
{
    expander1.setLevel(IO_NUM_B1, 0);

    expander1.setDirection(VCOM,         IO_MODE_OUTPUT);
    expander1.setDirection(PWRUP,        IO_MODE_OUTPUT);
    expander1.setDirection(WAKEUP,       IO_MODE_OUTPUT);
    expander1.setDirection(GPIO0_ENABLE, IO_MODE_OUTPUT);
    expander1.setLevel(GPIO0_ENABLE, 1);

    // For same reason, unused pins of first I/O expander have to be also set as
    // outputs, low.
    expander1.setDirection(IO_NUM_B6, IO_MODE_OUTPUT);
    expander1.setDirection(IO_NUM_B7, IO_MODE_OUTPUT);
    expander1.setLevel(IO_NUM_B6, 0);
    expander1.setLevel(IO_NUM_B7, 0);

    // Set SPI pins to input to reduce power consumption in deep sleep
    gpio_set_direction(GPIO_NUM_12, GPIO_MODE_INPUT);
    gpio_set_direction(GPIO_NUM_13, GPIO_MODE_INPUT);
    gpio_set_direction(GPIO_NUM_14, GPIO_MODE_INPUT);
    gpio_set_direction(GPIO_NUM_15, GPIO_MODE_INPUT);

    // And also disable uSD card supply
    expander1.setDirection(SD_PMOS_PIN, IO_MODE_INPUT);

    // CONTROL PINS
    gpio_set_direction(GPIO_NUM_0,  GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_2,  GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_32, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_33, GPIO_MODE_OUTPUT);
    expander1.setDirection(OE,   IO_MODE_OUTPUT);
    expander1.setDirection(GMOD, IO_MODE_OUTPUT);
    expander1.setDirection(SPV,  IO_MODE_OUTPUT);

    // DATA PINS
    gpio_set_direction(GPIO_NUM_4,  GPIO_MODE_OUTPUT);  // D0
    gpio_set_direction(GPIO_NUM_5,  GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_18, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_19, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_23, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_25, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_26, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_27, GPIO_MODE_OUTPUT);  // D7

    expander1.setDirection(IO_NUM_B2, IO_MODE_OUTPUT);
    expander1.setDirection(IO_NUM_B3, IO_MODE_OUTPUT);
    expander1.setDirection(IO_NUM_B4, IO_MODE_OUTPUT);
    expander1.setLevel(IO_NUM_B2, 0);
    expander1.setLevel(IO_NUM_B3, 0);
    expander1.setLevel(IO_NUM_B4, 0);

    // Battery voltage switch MOSFET
    expander1.setDirection(IO_NUM_B1, IO_MODE_OUTPUT);
    expander1.setLevel(IO_NUM_B1, 0);

    for (uint32_t i = 0; i < 256; ++i)
        m_pinLUT[i] = ((i & 0x03) << 4) | (((i & 0x0C) >> 2) << 18) | (((i & 0x10) >> 4) << 23) | (((i & 0xE0) >> 5) << 25);

    // Set all pins of second I/O expander to outputs, low.
    // For some reason, it draws more current in deep sleep when pins are set as inputs.
    expander2.setPort(IO_PORT_0, 0x00);
    expander2.setPort(IO_PORT_1, 0x00);
    expander2.setPortDirection(IO_PORT_0, 0x00);
    expander2.setPortDirection(IO_PORT_1, 0x00);
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
void Inkplate10::clean(uint8_t c, uint8_t rep)
{
  einkOn();
  uint8_t data = 0;
  if (c == 0)
    data = 0B10101010;
  else if (c == 1)
    data = 0B01010101;
  else if (c == 2)
    data = 0B00000000;
  else if (c == 3)
    data = 0B11111111;

  uint32_t _send = m_pinLUT[data];
  for (int k = 0; k < rep; ++k)
  {
    vscanStart();
    for (int i = 0; i < E_INK_HEIGHT; ++i)
    {
      hscanStart(_send);
      GPIO.out_w1ts = (_send) | CL;
      GPIO.out_w1tc = CL;
      for (int j = 0; j < ((E_INK_WIDTH / 8) - 1); ++j)
      {
          GPIO.out_w1ts = CL;
          GPIO.out_w1tc = CL;
          GPIO.out_w1ts = CL;
          GPIO.out_w1tc = CL;
      }
      GPIO.out_w1ts = CL;
      GPIO.out_w1tc = CL;
      vscanEnd();
    }
    esp_rom_delay_us(230);
  }
}

/**
 * @brief  Initialize the TPS65186 PMIC.
 *
 */
esp_err_t Inkplate10::pmicBegin()
{
  WAKEUP_SET;
  esp_rom_delay_us(1000);
  esp_err_t ret = tps.initSequences();
  esp_rom_delay_us(1000);
  WAKEUP_CLEAR;
  return ret;
}

/**
 * @brief  Restore all EPD control and data pins to outputs after pinsZstate().
 */
void Inkplate10::pinsAsOutputs()
{
  gpio_set_direction(GPIO_NUM_0,  GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NUM_2,  GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NUM_32, GPIO_MODE_OUTPUT);
  gpio_set_direction(GPIO_NUM_33, GPIO_MODE_OUTPUT);

  expander1.setDirection(OE,   IO_MODE_OUTPUT);
  expander1.setDirection(GMOD, IO_MODE_OUTPUT);
  expander1.setDirection(SPV,  IO_MODE_OUTPUT);

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
 * @brief  Release all EPD pins to high-Z (input) to save power.
 *
 * @note   Stops the I2S clock before releasing pins.
 */
void Inkplate10::pinsZstate()
{
  gpio_set_direction(GPIO_NUM_2,  GPIO_MODE_INPUT);
  gpio_set_direction(GPIO_NUM_32, GPIO_MODE_INPUT);
  gpio_set_direction(GPIO_NUM_33, GPIO_MODE_INPUT);

  expander1.setDirection(OE, IO_MODE_INPUT);
  expander1.setDirection(GMOD, IO_MODE_INPUT);
  expander1.setDirection(SPV, IO_MODE_INPUT);

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

/**
 * @brief  Set the panel power state.
 *
 * @param  bool state
 *         true if the panel is powered on, false if powered off
 */
void Inkplate10::setPanelState(bool state)
{
  m_panelState = state;
}

/**
 * @brief  Get the panel power state.
 *
 * @return bool
 *         true if the panel is powered on, false if powered off
 */
bool Inkplate10::getPanelState()
{
  return m_panelState;
}

