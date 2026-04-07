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
#elif CONFIG_INKPLATE_BOARD_INKPLATE10
  #include "inkplate10/pins.h"
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

#define _swap_int16_t(a, b) { int16_t t = (a); (a) = (b); (b) = t; }

// ---------------------------------------------------------------------------
// Global peripheral instances — shared by all board implementations
// ---------------------------------------------------------------------------

I2C     i2c;
PCAL    expander1(IO_INT_ADDR, i2c);
PCAL    expander2(IO_EXT_ADDR, i2c);
TPS     tps(i2c);
SDCard  sdCard(expander1);

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

BoardCommon::BoardCommon(uint16_t einkWidth, uint16_t einkHeight,
                         uint8_t cleanCycles1, uint8_t cleanCycles0)
  : rtc(i2c),
    m_einkWidth(einkWidth),
    m_einkHeight(einkHeight),
    m_cleanCycles1(cleanCycles1),
    m_cleanCycles0(cleanCycles0)
{
}

// ---------------------------------------------------------------------------
// Public — display mode
// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------
// Public — framebuffer operations
// ---------------------------------------------------------------------------

void BoardCommon::clearDisplay()
{
  if (m_displayMode == BLACK_AND_WHITE)
    memset(m_newFramebuffer,   0x00, m_einkWidth * m_einkHeight / 8);
  else if (m_displayMode == GRAYSCALE)
    memset(m_framebufferColor, 0xFF, m_einkWidth * m_einkHeight / 2);

  ESP_LOGI(TAG, "Display cleared.");
}

void BoardCommon::fillDisplay()
{
  if (m_displayMode == BLACK_AND_WHITE)
    memset(m_newFramebuffer,   0xFF, m_einkWidth * m_einkHeight / 8);
  else if (m_displayMode == GRAYSCALE)
    memset(m_framebufferColor, 0x00, m_einkWidth * m_einkHeight / 2);

  ESP_LOGI(TAG, "Display filled.");
}

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
}

// ---------------------------------------------------------------------------
// Public — display dispatch
// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------
// Public — power management
// ---------------------------------------------------------------------------

esp_err_t BoardCommon::einkOn()
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
  einkOnBoardInit();  // Inkplate10 asserts CL_CLEAR here; default is no-op

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

esp_err_t BoardCommon::einkOff()
{
  if (!getPanelState())
    return ESP_OK;

  VCOM_CLEAR;
  OE_CLEAR;
  GMOD_CLEAR;
  einkOffClearPins();  // Inkplate6: LE_CLEAR; Inkplate10: GPIO atomic DATA|LE|CL clear

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

// ---------------------------------------------------------------------------
// Public — partial update threshold
// ---------------------------------------------------------------------------

void BoardCommon::setFullUpdateThreshold(uint16_t numberOfPartialUpdates)
{
  m_partialUpdateLimiter = numberOfPartialUpdates;

  if (numberOfPartialUpdates != 0)
    m_blockPartial = true;
}

// ---------------------------------------------------------------------------
// Public — burn-in cleaning
// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------
// Public — battery
// ---------------------------------------------------------------------------

double BoardCommon::readBattery()
{
  expander1.setLevel(IO_NUM_B1, 1);
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

  expander1.setLevel(IO_NUM_B1, 0);

  return (double(mv) * 2.0 / 1000.0);
}

// ---------------------------------------------------------------------------
// Public — VCOM
// ---------------------------------------------------------------------------

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

double BoardCommon::getStoredVCOM()
{
  if (einkOn() != ESP_OK)
    return 0.0;

  double vcom = tps.readVCOM();
  einkOff();
  return vcom;
}

// ---------------------------------------------------------------------------
// Public — SD card
// ---------------------------------------------------------------------------

esp_err_t BoardCommon::sdCardInit()
{
  return sdCard.sdCardInit();
}

esp_err_t BoardCommon::sdCardSleep()
{
  return sdCard.sdCardSleep();
}

const char* BoardCommon::getMountPoint()
{
  return sdCard.getMountPoint();
}

// ---------------------------------------------------------------------------
// Protected — shared low-level helpers
// ---------------------------------------------------------------------------

void BoardCommon::vscanStart()
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

void BoardCommon::vscanEnd()
{
  CKV_CLEAR;
  LE_SET;
  LE_CLEAR;
  esp_rom_delay_us(0);
}

void BoardCommon::setPanelState(bool state)
{
  m_panelState = state;
}

bool BoardCommon::getPanelState()
{
  return m_panelState;
}

esp_err_t BoardCommon::pmicBegin()
{
  WAKEUP_SET;
  esp_rom_delay_us(1000);
  esp_err_t ret = tps.initSequences();
  esp_rom_delay_us(1000);
  WAKEUP_CLEAR;
  return ret;
}
