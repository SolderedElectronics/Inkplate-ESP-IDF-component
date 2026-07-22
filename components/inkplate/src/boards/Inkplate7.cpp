/**
 * @file Inkplate7.cpp
 * @author Fran Fodor for Soldered
 * @brief Driver for Inkplate 7 (Spectra7 panel) board.
 *
 * https://github.com/SolderedElectronics/Inkplate-Esp-library
 * For more info about the product, please check:
 * https://docs.soldered.com/inkplate/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "driver/gpio.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "esp_rom_sys.h"
#include "string.h"

#include "freertos/FreeRTOS.h"

#include "Inkplate7.h"

// Peripherals defined in BoardCommon.cpp
extern PCAL expander1;
extern I2C i2c;

static const char *TAG = "INKPLATE7";

#define _swap_int16_t(a, b)                                                  \
  {                                                                          \
    int16_t t = (a);                                                        \
    (a) = (b);                                                               \
    (b) = t;                                                                 \
  }

/* -------------------------------------------------------------------------- */
/*                              Public functions                              */
/* -------------------------------------------------------------------------- */

Inkplate7::Inkplate7()
    : BoardCommon(E_INK_WIDTH, E_INK_HEIGHT, 0, 0),
      m_spi(SPECTRA73_SPI_MOSI, SPECTRA73_SPI_SCK) {
  ESP_ERROR_CHECK(initBuffers());

  clearDisplay();

  setPanelPinsToLow();

  setPanelPower(false);

  rtc.begin(i2c.getBusHandle());

  ESP_LOGI(TAG, "Initialization finished!");
}

double Inkplate7::readBattery() {
  // Read the pin on the battery MOSFET. If it's high, that means it's the
  // older version of the board that uses PMOS only. If it's low, it's the
  // newer board with both PMOS and NMOS.
  expander1.setDirection(SPECTRA73_BATT_MOSFET_PIN, IO_MODE_INPUT);
  int state = expander1.getLevel(SPECTRA73_BATT_MOSFET_PIN);
  expander1.setDirection(SPECTRA73_BATT_MOSFET_PIN, IO_MODE_OUTPUT);

  // If the input is pulled high, it's PMOS only.
  // If it's pulled low, it's PMOS and NMOS.
  if (state)
    expander1.setLevel(SPECTRA73_BATT_MOSFET_PIN, 0);
  else
    expander1.setLevel(SPECTRA73_BATT_MOSFET_PIN, 1);

  // Wait a little bit after a MOSFET enable.
  esp_rom_delay_us(5000);

  adc_oneshot_unit_handle_t adcHandle;
  adc_oneshot_unit_init_cfg_t initCfg = {};
  initCfg.unit_id = ADC_UNIT_1;
  adc_oneshot_new_unit(&initCfg, &adcHandle);

  adc_oneshot_chan_cfg_t chanCfg = {};
  chanCfg.atten = ADC_ATTEN_DB_12;
  chanCfg.bitwidth = ADC_BITWIDTH_12;
  adc_oneshot_config_channel(adcHandle, ADC_CHANNEL_0, &chanCfg);

  adc_cali_handle_t caliHandle = NULL;
  adc_cali_curve_fitting_config_t caliCfg = {};
  caliCfg.unit_id = ADC_UNIT_1;
  caliCfg.atten = ADC_ATTEN_DB_12;
  caliCfg.bitwidth = ADC_BITWIDTH_12;
  bool calibrated =
      (adc_cali_create_scheme_curve_fitting(&caliCfg, &caliHandle) == ESP_OK);

  int raw = 0, mv = 0;
  adc_oneshot_read(adcHandle, ADC_CHANNEL_0, &raw);
  if (calibrated) {
    adc_cali_raw_to_voltage(caliHandle, raw, &mv);
    adc_cali_delete_scheme_curve_fitting(caliHandle);
  }
  adc_oneshot_del_unit(adcHandle);

  // Turn off the MOSFET (and voltage divider).
  if (state)
    expander1.setLevel(SPECTRA73_BATT_MOSFET_PIN, 1);
  else
    expander1.setLevel(SPECTRA73_BATT_MOSFET_PIN, 0);

  // Calculate the voltage at the battery terminal (voltage is divided in
  // half by the voltage divider).
  return (double(mv) * 2.0 / 1000.0);
}

/* -------------------------------------------------------------------------- */
/*                              Private functions                             */
/* -------------------------------------------------------------------------- */

esp_err_t Inkplate7::initBuffers() {
  m_framebufferColor = (uint8_t *)heap_caps_malloc(
      E_INK_WIDTH * E_INK_HEIGHT / 2, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  if (!m_framebufferColor)
    return ESP_ERR_NO_MEM;

  return ESP_OK;
}

void Inkplate7::writePixelInternal(int16_t x, int16_t y, uint16_t color) {
  int16_t x0 = x;
  int16_t y0 = y;

  uint8_t r = getRotation();
  int16_t logW = (r == 1 || r == 3) ? (int16_t)E_INK_HEIGHT : (int16_t)E_INK_WIDTH;
  int16_t logH = (r == 1 || r == 3) ? (int16_t)E_INK_WIDTH : (int16_t)E_INK_HEIGHT;
  if (x0 < 0 || y0 < 0 || x0 >= logW || y0 >= logH)
    return;
  if (color > 5)
    return;
  color = colorPalette[color];

  // The panel is mounted rotated by 180 degrees inside the enclosure, so
  // rotation 0 (the default) maps user coordinates to flipped panel
  // coordinates.
  switch (r) {
  case 3:
    _swap_int16_t(x0, y0);
    x0 = (int16_t)E_INK_HEIGHT - x0 - 1;
    break;
  case 0:
    x0 = (int16_t)E_INK_WIDTH - x0 - 1;
    y0 = (int16_t)E_INK_HEIGHT - y0 - 1;
    break;
  case 1:
    _swap_int16_t(x0, y0);
    y0 = (int16_t)E_INK_WIDTH - y0 - 1;
    break;
  default:
    break;
  }

  int x1 = x0 / 2;
  int xSub = x0 % 2;
  uint8_t temp = *(m_framebufferColor + E_INK_WIDTH / 2 * y0 + x1);
  *(m_framebufferColor + E_INK_WIDTH / 2 * y0 + x1) =
      (pixelMaskGLUT[xSub] & temp) | (xSub ? color : color << 4);
}

void Inkplate7::clearDisplay() {
  memset(m_framebufferColor, (INKPLATE_WHITE << 4) | INKPLATE_WHITE,
         E_INK_WIDTH * E_INK_HEIGHT / 2);
  ESP_LOGI(TAG, "Display cleared.");
}

void Inkplate7::fillDisplay() {
  memset(m_framebufferColor, (INKPLATE_BLACK << 4) | INKPLATE_BLACK,
         E_INK_WIDTH * E_INK_HEIGHT / 2);
  ESP_LOGI(TAG, "Display filled.");
}

esp_err_t Inkplate7::display3b(bool leaveOn) {
  setPanelPower(true);

  sendCommand(SPECTRA73_REGISTER_DTM, nullptr, 0);
  gpio_set_level(SPECTRA73_CS_PIN, 0);
  m_spi.sendData(m_framebufferColor, E_INK_WIDTH * E_INK_HEIGHT / 2,
                 SPECTRA73_DC_PIN);
  gpio_set_level(SPECTRA73_CS_PIN, 1);

  waitForEpd(60000);

  sendCommand(SPECTRA73_REGISTER_DRF, SPECTRA73_REGISTER_DRF_V,
              sizeof(SPECTRA73_REGISTER_DRF_V));
  waitForEpd(60000);

  if (!leaveOn)
    setPanelPower(false);

  return ESP_OK;
}

bool Inkplate7::waitForEpd(uint32_t timeout) {
  uint32_t elapsed = 0;
  const uint32_t STEP = 10;

  while (gpio_get_level(SPECTRA73_BUSYN_PIN) == 0) {
    if (elapsed >= timeout) {
      ESP_LOGE(TAG, "EPD busy timeout");
      return false;
    }
    vTaskDelay(pdMS_TO_TICKS(STEP));
    elapsed += STEP;
  }

  return true;
}

void Inkplate7::resetPanel() {
  gpio_set_level(SPECTRA73_RST_PIN, 0);
  vTaskDelay(pdMS_TO_TICKS(10));
  gpio_set_level(SPECTRA73_RST_PIN, 1);
  vTaskDelay(pdMS_TO_TICKS(20));
}

void Inkplate7::setIO() {
  gpio_set_direction(SPECTRA73_DC_PIN, GPIO_MODE_OUTPUT);
  gpio_set_direction(SPECTRA73_CS_PIN, GPIO_MODE_OUTPUT);
  gpio_set_direction(SPECTRA73_RST_PIN, GPIO_MODE_OUTPUT);
  gpio_set_direction(SPECTRA73_BUSYN_PIN, GPIO_MODE_INPUT);
  gpio_pullup_en(SPECTRA73_BUSYN_PIN);
  gpio_set_direction(SPECTRA73_PWR_EN, GPIO_MODE_OUTPUT);
  gpio_set_direction(SPECTRA73_BS0, GPIO_MODE_OUTPUT);
  gpio_set_direction(SPECTRA73_BS1, GPIO_MODE_OUTPUT);

  // BS0 and BS1 low select the 4-wire SPI interface mode on the panel.
  gpio_set_level(SPECTRA73_DC_PIN, 1);
  gpio_set_level(SPECTRA73_CS_PIN, 1);
  gpio_set_level(SPECTRA73_RST_PIN, 0);
  gpio_set_level(SPECTRA73_PWR_EN, 0);
  gpio_set_level(SPECTRA73_BS0, 0);
  gpio_set_level(SPECTRA73_BS1, 0);

  if (!m_spi.isInitialized())
    m_spi.init();
}

void Inkplate7::setPanelPower(bool state) {
  if (state == getPanelState())
    return;

  if (state) {
    setPanelPinsToLow();
    vTaskDelay(pdMS_TO_TICKS(50));

    setIO();

    gpio_set_level(SPECTRA73_PWR_EN, 1);
    vTaskDelay(pdMS_TO_TICKS(100));

    resetPanel();
    waitForEpd(60000);

    screenInit();

    sendCommand(SPECTRA73_REGISTER_PON, nullptr, 0);
    waitForEpd(60000);
  } else {
    sendCommand(SPECTRA73_REGISTER_POF, SPECTRA73_REGISTER_POF_V,
                sizeof(SPECTRA73_REGISTER_POF_V));
    waitForEpd(60000);

    gpio_set_level(SPECTRA73_PWR_EN, 0);

    gpio_set_direction(SPECTRA73_DC_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(SPECTRA73_CS_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(SPECTRA73_RST_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(SPECTRA73_BUSYN_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(SPECTRA73_PWR_EN, GPIO_MODE_INPUT);

    m_spi.deinit();
  }

  setPanelState(state);
}

void Inkplate7::setPanelPinsToLow() {
  const gpio_num_t pins[] = {SPECTRA73_DC_PIN,   SPECTRA73_CS_PIN,
                             SPECTRA73_RST_PIN,  SPECTRA73_BUSYN_PIN,
                             SPECTRA73_PWR_EN,   SPECTRA73_BS0,
                             SPECTRA73_BS1};
  for (auto p : pins) {
    gpio_set_direction(p, GPIO_MODE_OUTPUT);
    gpio_set_level(p, 0);
  }
}

void Inkplate7::sendCommand(uint8_t cmd, const uint8_t *data, int n) {
  gpio_set_level(SPECTRA73_CS_PIN, 0);
  m_spi.sendCommand(cmd, SPECTRA73_DC_PIN);
  gpio_set_level(SPECTRA73_CS_PIN, 1);

  if (data && n > 0) {
    gpio_set_level(SPECTRA73_CS_PIN, 0);
    m_spi.sendData((uint8_t *)data, n, SPECTRA73_DC_PIN);
    gpio_set_level(SPECTRA73_CS_PIN, 1);
  }
}

void Inkplate7::screenInit() {
  sendCommand(SPECTRA73_REGISTER_CMDH, SPECTRA73_REGISTER_CMDH_V,
              sizeof(SPECTRA73_REGISTER_CMDH_V));
  sendCommand(SPECTRA73_REGISTER_PWR, SPECTRA73_REGISTER_PWR_V,
              sizeof(SPECTRA73_REGISTER_PWR_V));
  sendCommand(SPECTRA73_REGISTER_PSR, SPECTRA73_REGISTER_PSR_V,
              sizeof(SPECTRA73_REGISTER_PSR_V));
  sendCommand(SPECTRA73_REGISTER_PFS, SPECTRA73_REGISTER_PFS_V,
              sizeof(SPECTRA73_REGISTER_PFS_V));
  sendCommand(SPECTRA73_REGISTER_BTST1, SPECTRA73_REGISTER_BTST1_V,
              sizeof(SPECTRA73_REGISTER_BTST1_V));
  sendCommand(SPECTRA73_REGISTER_BTST2, SPECTRA73_REGISTER_BTST2_V,
              sizeof(SPECTRA73_REGISTER_BTST2_V));
  sendCommand(SPECTRA73_REGISTER_BTST3, SPECTRA73_REGISTER_BTST3_V,
              sizeof(SPECTRA73_REGISTER_BTST3_V));
  sendCommand(SPECTRA73_REGISTER_IPC, SPECTRA73_REGISTER_IPC_V,
              sizeof(SPECTRA73_REGISTER_IPC_V));
  sendCommand(SPECTRA73_REGISTER_PLL, SPECTRA73_REGISTER_PLL_V,
              sizeof(SPECTRA73_REGISTER_PLL_V));
  sendCommand(SPECTRA73_REGISTER_TSE, SPECTRA73_REGISTER_TSE_V,
              sizeof(SPECTRA73_REGISTER_TSE_V));
  sendCommand(SPECTRA73_REGISTER_CDI, SPECTRA73_REGISTER_CDI_V,
              sizeof(SPECTRA73_REGISTER_CDI_V));
  sendCommand(SPECTRA73_REGISTER_TCON, SPECTRA73_REGISTER_TCON_V,
              sizeof(SPECTRA73_REGISTER_TCON_V));
  sendCommand(SPECTRA73_REGISTER_TRES, SPECTRA73_REGISTER_TRES_V,
              sizeof(SPECTRA73_REGISTER_TRES_V));
  sendCommand(SPECTRA73_REGISTER_VDCS, SPECTRA73_REGISTER_VDCS_V,
              sizeof(SPECTRA73_REGISTER_VDCS_V));
  sendCommand(SPECTRA73_REGISTER_T_VDCS, SPECTRA73_REGISTER_T_VDCS_V,
              sizeof(SPECTRA73_REGISTER_T_VDCS_V));
  sendCommand(SPECTRA73_REGISTER_AGID, SPECTRA73_REGISTER_AGID_V,
              sizeof(SPECTRA73_REGISTER_AGID_V));
  sendCommand(SPECTRA73_REGISTER_PWS, SPECTRA73_REGISTER_PWS_V,
              sizeof(SPECTRA73_REGISTER_PWS_V));
  sendCommand(SPECTRA73_REGISTER_CCSET, SPECTRA73_REGISTER_CCSET_V,
              sizeof(SPECTRA73_REGISTER_CCSET_V));
  sendCommand(SPECTRA73_REGISTER_TSSET, SPECTRA73_REGISTER_TSSET_V,
              sizeof(SPECTRA73_REGISTER_TSSET_V));
}
