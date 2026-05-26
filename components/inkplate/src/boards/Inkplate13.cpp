/**
 * @file Inkplate13.cpp
 * @author Fran Fodor for Soldered
 * @brief Driver for Inkplate 13 board.
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
#include "esp_heap_caps.h"
#include "esp_log.h"
#include "soc/gpio_sig_map.h"
#include "soc/i2s_struct.h"
#include "string.h"

#include "freertos/FreeRTOS.h"

#include "Inkplate13.h"
#include "TPS.h"

// Peripherals defined in BoardCommon.cpp
extern TPS tps;
extern I2C i2c;

static const char *TAG = "INKPLATE13";

/* -------------------------------------------------------------------------- */
/*                              Public functions                              */
/* -------------------------------------------------------------------------- */

Inkplate13::Inkplate13()
    : BoardCommon(E_INK_WIDTH, E_INK_HEIGHT, 0, 0),
      m_spi(SPECTRA133_SPI_MOSI, SPECTRA133_SPI_SCK) {
  ESP_ERROR_CHECK(initBuffers());

  clearDisplay();

  gpio_set_direction(SPECTRA133_RST_PIN, GPIO_MODE_OUTPUT);
  gpio_set_direction(SPECTRA133_DC_PIN, GPIO_MODE_OUTPUT);
  gpio_set_direction(SPECTRA133_CS_M_PIN, GPIO_MODE_OUTPUT);
  gpio_set_direction(SPECTRA133_CS_S_PIN, GPIO_MODE_OUTPUT);
  gpio_set_direction(SPECTRA133_PWR_EN, GPIO_MODE_OUTPUT);
  gpio_set_direction(SPECTRA133_BS0, GPIO_MODE_OUTPUT);
  gpio_set_direction(SPECTRA133_BS1, GPIO_MODE_OUTPUT);

  gpio_set_level(SPECTRA133_RST_PIN, 0);
  gpio_set_level(SPECTRA133_DC_PIN, 0);
  gpio_set_level(SPECTRA133_CS_M_PIN, 0);
  gpio_set_level(SPECTRA133_CS_S_PIN, 0);
  gpio_set_level(SPECTRA133_PWR_EN, 0);
  gpio_set_level(SPECTRA133_BS0, 0);
  gpio_set_level(SPECTRA133_BS1, 0);

  gpio_set_direction(SPECTRA133_BUSYN_PIN, GPIO_MODE_INPUT);
  gpio_pullup_en(SPECTRA133_BUSYN_PIN);

  if (!setPanelDeepSleep(false))
    ESP_LOGE(TAG, "Panel init failed");

  setPanelDeepSleep(true);

  rtc.begin(i2c.getBusHandle());

  ESP_LOGI(TAG, "Initialization finished!");
}

/* -------------------------------------------------------------------------- */
/*                              Private functions                             */
/* -------------------------------------------------------------------------- */

esp_err_t Inkplate13::initBuffers() {
  m_framebufferColor = (uint8_t *)heap_caps_malloc(
      E_INK_WIDTH * E_INK_HEIGHT / 2, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  if (!m_framebufferColor)
    return ESP_ERR_NO_MEM;

  return ESP_OK;
}

esp_err_t Inkplate13::display3b(bool leaveOn) {
  setPanelDeepSleep(false);

  gpio_set_level(SPECTRA133_CS_M_PIN, 0);
  gpio_set_level(SPECTRA133_CS_S_PIN, 1);
  m_spi.sendCommand(SPECTRA133_REGISTER_DTM, SPECTRA133_DC_PIN);
  for (int i = 0; i < E_INK_HEIGHT; i++)
    m_spi.sendData(m_framebufferColor + (i * E_INK_WIDTH / 2), E_INK_WIDTH / 4,
                   SPECTRA133_DC_PIN);

  gpio_set_level(SPECTRA133_CS_M_PIN, 1);

  waitForEpd(60000);

  gpio_set_level(SPECTRA133_CS_S_PIN, 0);
  gpio_set_level(SPECTRA133_CS_M_PIN, 1);

  m_spi.sendCommand(SPECTRA133_REGISTER_DTM, SPECTRA133_DC_PIN);
  for (int i = 0; i < E_INK_HEIGHT; i++)
    m_spi.sendData(m_framebufferColor + (i * E_INK_WIDTH / 2) +
                       (E_INK_WIDTH / 4),
                   E_INK_WIDTH / 4, SPECTRA133_DC_PIN);

  gpio_set_level(SPECTRA133_CS_S_PIN, 1);
  waitForEpd(60000);

  sendCommandData(SPECTRA133_REGISTER_DRF, SPECTRA133_REGISTER_DRF_V,
                  sizeof(SPECTRA133_REGISTER_DRF_V), eChipIdBoth);
  waitForEpd(60000);

  if (!leaveOn)
    setPanelDeepSleep(true);

  return ESP_OK;
}

bool Inkplate13::waitForEpd(uint32_t timeout) {
  uint32_t elapsed = 0;
  const uint32_t STEP = 10;

  while (gpio_get_level(SPECTRA133_BUSYN_PIN) == 0) {
    if (elapsed >= timeout) {
      return false;
    }
    vTaskDelay(pdMS_TO_TICKS(STEP));
    elapsed += STEP;
  }

  return true;
}

void Inkplate13::resetPanel() {
  gpio_set_level(SPECTRA133_RST_PIN, 0);
  vTaskDelay(pdMS_TO_TICKS(100));
  gpio_set_level(SPECTRA133_RST_PIN, 1);
  vTaskDelay(pdMS_TO_TICKS(100));
}

void Inkplate13::sendCommandData(uint8_t cmd, uint8_t *data, int n,
                                 enum eSpectraChipID chipId) {
  if (chipId & eChipIdMaster)
    gpio_set_level(SPECTRA133_CS_M_PIN, 0);
  if (chipId & eChipIdSlave)
    gpio_set_level(SPECTRA133_CS_S_PIN, 0);

  m_spi.sendCommand(cmd, SPECTRA133_DC_PIN);

  if (data && n > 0)
    m_spi.sendData(data, n, SPECTRA133_DC_PIN);

  if (chipId & eChipIdMaster)
    gpio_set_level(SPECTRA133_CS_M_PIN, 1);
  if (chipId & eChipIdSlave)
    gpio_set_level(SPECTRA133_CS_S_PIN, 1);
}

bool Inkplate13::setPanelDeepSleep(bool sleep) {
  if (!sleep) {
    if (!m_spi.isInitialized())
      m_spi.init();

    setPanelPinsToLow();
    vTaskDelay(pdMS_TO_TICKS(100));

    gpio_set_level(SPECTRA133_DC_PIN, 1);
    gpio_set_level(SPECTRA133_CS_M_PIN, 1);
    gpio_set_level(SPECTRA133_CS_S_PIN, 1);
    gpio_set_level(SPECTRA133_RST_PIN, 0);
    gpio_set_level(SPECTRA133_BS0, 0);
    gpio_set_level(SPECTRA133_BS1, 1);

    gpio_set_level(SPECTRA133_PWR_EN, 1);
    vTaskDelay(pdMS_TO_TICKS(150));

    resetPanel();
    vTaskDelay(pdMS_TO_TICKS(150));

    screenInit();

    sendCommandData(SPECTRA133_REGISTER_PON, nullptr, 0, eChipIdBoth);
    waitForEpd(60000);
    return true;
  } else {
    sendCommandData(SPECTRA133_REGISTER_POF, SPECTRA133_REGISTER_POF_V,
                    sizeof(SPECTRA133_REGISTER_POF_V), eChipIdBoth);
    waitForEpd(60000);

    gpio_set_direction(SPECTRA133_DC_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(SPECTRA133_CS_M_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(SPECTRA133_CS_S_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(SPECTRA133_RST_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(SPECTRA133_BUSYN_PIN, GPIO_MODE_INPUT);
    gpio_set_direction(SPECTRA133_PWR_EN, GPIO_MODE_INPUT);

    gpio_set_level(SPECTRA133_PWR_EN, 0);

    m_spi.deinit();

    return true;
  }
}

void Inkplate13::setPanelPinsToLow() {
  const gpio_num_t pins[] = {SPECTRA133_DC_PIN,   SPECTRA133_CS_M_PIN,
                             SPECTRA133_CS_S_PIN, SPECTRA133_RST_PIN,
                             SPECTRA133_PWR_EN,   SPECTRA133_BS0,
                             SPECTRA133_BS1};
  for (auto p : pins) {
    gpio_set_direction(p, GPIO_MODE_OUTPUT);
    gpio_set_level(p, 0);
  }
}

esp_err_t Inkplate13::displayPartial(int16_t x, int16_t y, int16_t w, int16_t h,
                                     bool leaveOn) {
  uint8_t rot = getRotation();
  int16_t logW =
      (rot == 1 || rot == 3) ? (int16_t)E_INK_HEIGHT : (int16_t)E_INK_WIDTH;
  int16_t logH =
      (rot == 1 || rot == 3) ? (int16_t)E_INK_WIDTH : (int16_t)E_INK_HEIGHT;

  if (x < 0) {
    w += x;
    x = 0;
  }
  if (y < 0) {
    h += y;
    y = 0;
  }
  if (x + w > logW)
    w = logW - x;
  if (y + h > logH)
    h = logH - y;
  if (w <= 0 || h <= 0)
    return ESP_OK;

  // Map user rectangle to panel-native rectangle.
  // Transforms match writePixelInternal() in BoardCommon.cpp.
  int16_t colStart, colEnd, rowStart, rowEnd;
  switch (rot) {
  case 0:
    // panel_col = x,  panel_row = y
    colStart = x;
    colEnd = x + w - 1;
    rowStart = y;
    rowEnd = y + h - 1;
    break;
  case 1:
    // panel_col = E_INK_WIDTH-1-y,  panel_row = x
    colStart = (int16_t)E_INK_WIDTH - y - h;
    colEnd = (int16_t)E_INK_WIDTH - 1 - y;
    rowStart = x;
    rowEnd = x + w - 1;
    break;
  case 2:
    // panel_col = E_INK_WIDTH-1-x,  panel_row = E_INK_HEIGHT-1-y
    colStart = (int16_t)E_INK_WIDTH - x - w;
    colEnd = (int16_t)E_INK_WIDTH - 1 - x;
    rowStart = (int16_t)E_INK_HEIGHT - y - h;
    rowEnd = (int16_t)E_INK_HEIGHT - 1 - y;
    break;
  default:
  case 3:
    // panel_col = y,  panel_row = E_INK_HEIGHT-1-x
    colStart = y;
    colEnd = y + h - 1;
    rowStart = (int16_t)E_INK_HEIGHT - x - w;
    rowEnd = (int16_t)E_INK_HEIGHT - 1 - x;
    break;
  }

  // PTLW alignment (GDEP133C02):
  // H: colStart and (colEnd+1) must be multiples of 4.
  // V: rowStart and (rowEnd+1) must be even.
  colStart = (colStart / 4) * 4;
  colEnd = (((colEnd + 4) / 4) * 4) - 1;
  if (colEnd >= (int16_t)E_INK_WIDTH)
    colEnd = (int16_t)E_INK_WIDTH - 1;
  if (rowStart % 2 != 0)
    rowStart--;
  if (rowStart < 0)
    rowStart = 0;
  if ((rowEnd + 1) % 2 != 0)
    rowEnd++;
  if (rowEnd >= (int16_t)E_INK_HEIGHT)
    rowEnd = (int16_t)E_INK_HEIGHT - 1;

  setPanelDeepSleep(false);

  const int16_t HALF_WIDTH = (int16_t)(E_INK_WIDTH / 2);
  const int16_t HALF_BYTES = HALF_WIDTH / 2;

  bool masterNeeded = (colStart < HALF_WIDTH);
  bool slaveNeeded = (colEnd >= HALF_WIDTH);

  // Both chips must receive PTLW+DTM before DRF fires. For the uninvolved chip,
  // a minimal 4×4 null window that replays existing framebuffer data is used so
  // the refresh produces no visible change on that side.
  static const uint8_t ptlwNull[9] = {
      0x00, 0x00, // HRST = 0
      0x00, 0x07, // HRED = 7
      0x00, 0x00, // VRST = 0
      0x00, 0x01, // VRED = 1
      0x01        // PT   = 1
  };

  // Master chip
  {
    uint8_t ptlwData[9];
    int16_t bytesPerRow, memColOff, rStart, rEnd;

    if (masterNeeded) {
      int16_t lcs = colStart;
      int16_t lce = (colEnd < HALF_WIDTH) ? colEnd : (HALF_WIDTH - 1);
      uint16_t HRST = (uint16_t)lcs * 2;
      uint16_t HRED = (uint16_t)(lce + 1) * 2 - 1;
      uint16_t VRST = (uint16_t)rowStart / 2;
      uint16_t VRED = (uint16_t)(rowEnd + 1) / 2 - 1;
      ptlwData[0] = HRST >> 8;
      ptlwData[1] = HRST & 0xFF;
      ptlwData[2] = HRED >> 8;
      ptlwData[3] = HRED & 0xFF;
      ptlwData[4] = VRST >> 8;
      ptlwData[5] = VRST & 0xFF;
      ptlwData[6] = VRED >> 8;
      ptlwData[7] = VRED & 0xFF;
      ptlwData[8] = 0x01;
      bytesPerRow = (lce - lcs + 1) / 2;
      memColOff = lcs / 2;
      rStart = rowStart;
      rEnd = rowEnd;
    } else {
      memcpy(ptlwData, ptlwNull, 9);
      bytesPerRow = 2;
      memColOff = 0;
      rStart = 0;
      rEnd = 3;
    }

    sendCommandData(SPECTRA133_REGISTER_CMD66, SPECTRA133_REGISTER_CMD66_V,
                    sizeof(SPECTRA133_REGISTER_CMD66_V), eChipIdMaster);
    sendCommandData(SPECTRA133_REGISTER_PTLW, ptlwData, 9, eChipIdMaster);

    gpio_set_level(SPECTRA133_CS_M_PIN, 0);
    m_spi.sendCommand(SPECTRA133_REGISTER_DTM, SPECTRA133_DC_PIN);
    for (int16_t row = rStart; row <= rEnd; row++)
      m_spi.sendData(m_framebufferColor + row * (E_INK_WIDTH / 2) + memColOff,
                     bytesPerRow, SPECTRA133_DC_PIN);
    gpio_set_level(SPECTRA133_CS_M_PIN, 1);
  }

  // Slave chip
  waitForEpd(60000);
  {
    uint8_t ptlwData[9];
    int16_t bytesPerRow, memColOff, rStart, rEnd;

    if (slaveNeeded) {
      int16_t lcs =
          (colStart >= HALF_WIDTH) ? (colStart - HALF_WIDTH) : (int16_t)0;
      int16_t lce = colEnd - HALF_WIDTH;
      uint16_t HRST = (uint16_t)lcs * 2;
      uint16_t HRED = (uint16_t)(lce + 1) * 2 - 1;
      uint16_t VRST = (uint16_t)rowStart / 2;
      uint16_t VRED = (uint16_t)(rowEnd + 1) / 2 - 1;
      ptlwData[0] = HRST >> 8;
      ptlwData[1] = HRST & 0xFF;
      ptlwData[2] = HRED >> 8;
      ptlwData[3] = HRED & 0xFF;
      ptlwData[4] = VRST >> 8;
      ptlwData[5] = VRST & 0xFF;
      ptlwData[6] = VRED >> 8;
      ptlwData[7] = VRED & 0xFF;
      ptlwData[8] = 0x01;
      bytesPerRow = (lce - lcs + 1) / 2;
      memColOff = HALF_BYTES + lcs / 2;
      rStart = rowStart;
      rEnd = rowEnd;
    } else {
      memcpy(ptlwData, ptlwNull, 9);
      bytesPerRow = 2;
      memColOff = HALF_BYTES;
      rStart = 0;
      rEnd = 3;
    }

    sendCommandData(SPECTRA133_REGISTER_CMD66, SPECTRA133_REGISTER_CMD66_V,
                    sizeof(SPECTRA133_REGISTER_CMD66_V), eChipIdSlave);
    sendCommandData(SPECTRA133_REGISTER_PTLW, ptlwData, 9, eChipIdSlave);

    gpio_set_level(SPECTRA133_CS_S_PIN, 0);
    m_spi.sendCommand(SPECTRA133_REGISTER_DTM, SPECTRA133_DC_PIN);
    for (int16_t row = rStart; row <= rEnd; row++)
      m_spi.sendData(m_framebufferColor + row * (E_INK_WIDTH / 2) + memColOff,
                     bytesPerRow, SPECTRA133_DC_PIN);
    gpio_set_level(SPECTRA133_CS_S_PIN, 1);
  }

  waitForEpd(60000);
  sendCommandData(SPECTRA133_REGISTER_DRF, SPECTRA133_REGISTER_DRF_V,
                  sizeof(SPECTRA133_REGISTER_DRF_V), eChipIdBoth);
  waitForEpd(60000);

  if (!leaveOn)
    setPanelDeepSleep(true);

  return ESP_OK;
}

void Inkplate13::screenInit() {
  sendCommandData(SPECTRA133_REGISTER_AN_TM, SPECTRA133_REGISTER_AN_TM_V,
                  sizeof(SPECTRA133_REGISTER_AN_TM_V), eChipIdMaster);

  sendCommandData(SPECTRA133_REGISTER_CMD66, SPECTRA133_REGISTER_CMD66_V,
                  sizeof(SPECTRA133_REGISTER_CMD66_V), eChipIdBoth);
  sendCommandData(SPECTRA133_REGISTER_PSR, SPECTRA133_REGISTER_PSR_V,
                  sizeof(SPECTRA133_REGISTER_PSR_V), eChipIdBoth);
  sendCommandData(SPECTRA133_REGISTER_PLL, SPECTRA133_REGISTER_PLL_V,
                  sizeof(SPECTRA133_REGISTER_PLL_V), eChipIdBoth);
  sendCommandData(SPECTRA133_REGISTER_CDI, SPECTRA133_REGISTER_CDI_V,
                  sizeof(SPECTRA133_REGISTER_CDI_V), eChipIdBoth);
  sendCommandData(SPECTRA133_REGISTER_TCON, SPECTRA133_REGISTER_TCON_V,
                  sizeof(SPECTRA133_REGISTER_TCON_V), eChipIdBoth);
  sendCommandData(SPECTRA133_REGISTER_AGID, SPECTRA133_REGISTER_AGID_V,
                  sizeof(SPECTRA133_REGISTER_AGID_V), eChipIdBoth);
  sendCommandData(SPECTRA133_REGISTER_PWS, SPECTRA133_REGISTER_PWS_V,
                  sizeof(SPECTRA133_REGISTER_PWS_V), eChipIdBoth);
  sendCommandData(SPECTRA133_REGISTER_CCSET, SPECTRA133_REGISTER_CCSET_V,
                  sizeof(SPECTRA133_REGISTER_CCSET_V), eChipIdBoth);
  sendCommandData(SPECTRA133_REGISTER_TRES, SPECTRA133_REGISTER_TRES_V,
                  sizeof(SPECTRA133_REGISTER_TRES_V), eChipIdBoth);

  sendCommandData(SPECTRA133_REGISTER_PWR, SPECTRA133_REGISTER_PWR_V,
                  sizeof(SPECTRA133_REGISTER_PWR_V), eChipIdMaster);
  sendCommandData(SPECTRA133_REGISTER_EN_BUF, SPECTRA133_REGISTER_EN_BUF_V,
                  sizeof(SPECTRA133_REGISTER_EN_BUF_V), eChipIdMaster);
  sendCommandData(SPECTRA133_REGISTER_BTST_P, SPECTRA133_REGISTER_BTST_P_V,
                  sizeof(SPECTRA133_REGISTER_BTST_P_V), eChipIdMaster);
  sendCommandData(SPECTRA133_REGISTER_BOOST_VDDP_EN,
                  SPECTRA133_REGISTER_BOOST_VDDP_EN_V,
                  sizeof(SPECTRA133_REGISTER_BOOST_VDDP_EN_V), eChipIdMaster);
  sendCommandData(SPECTRA133_REGISTER_BTST_N, SPECTRA133_REGISTER_BTST_N_V,
                  sizeof(SPECTRA133_REGISTER_BTST_N_V), eChipIdMaster);
  sendCommandData(SPECTRA133_REGISTER_BUCK_BOOST_VDDN,
                  SPECTRA133_REGISTER_BUCK_BOOST_VDDN_V,
                  sizeof(SPECTRA133_REGISTER_BUCK_BOOST_VDDN_V), eChipIdMaster);
  sendCommandData(SPECTRA133_REGISTER_TFT_VCOM_POWER,
                  SPECTRA133_REGISTER_TFT_VCOM_POWER_V,
                  sizeof(SPECTRA133_REGISTER_TFT_VCOM_POWER_V), eChipIdMaster);
}
