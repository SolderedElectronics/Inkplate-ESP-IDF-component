/**
 * @file Buzzer.cpp
 * @author Fran Fodor for Soldered
 * @brief Driver for the on-board passive buzzer on Inkplate 4TEMPERA.
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

#include "Buzzer.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "BUZZER";

/* -------------------------------------------------------------------------- */
/*                              Public functions                              */
/* -------------------------------------------------------------------------- */

esp_err_t Buzzer::begin(I2C &i2c, PCAL &expander, IOPin_t enablePin) {
  m_expander = &expander;
  m_enablePin = enablePin;
  m_expander->setDirection(m_enablePin, IO_MODE_OUTPUT);
  m_expander->setLevel(m_enablePin, 1, true); // HIGH = off by default
  ESP_LOGI(TAG, "Buzzer init OK");
  return i2c.addDevice(MCP4018_I2C_ADDR, &m_devHandle);
}

void Buzzer::beep(uint32_t length_ms) {
  beepOn();
  vTaskDelay(pdMS_TO_TICKS(length_ms));
  beepOff();
}

void Buzzer::beep(uint32_t length_ms, int freq_hz) {
  beepOn(freq_hz);
  vTaskDelay(pdMS_TO_TICKS(length_ms));
  beepOff();
}

void Buzzer::beepOn() {
  // 50% wiper = default pitch
  setWiperValue((uint8_t)((50.0f / 100.0f) * 127.0f));
  m_expander->setLevel(m_enablePin, 0, true); // LOW = on
}

void Buzzer::beepOn(int freq_hz) {
  setWiperValue((uint8_t)freqToWiperValue(freq_hz));
  m_expander->setLevel(m_enablePin, 0, true); // LOW = on
}

void Buzzer::beepOff() {
  m_expander->setLevel(m_enablePin, 1, true); // HIGH = off
}

/* -------------------------------------------------------------------------- */
/*                             Private functions                              */
/* -------------------------------------------------------------------------- */

void Buzzer::setWiperValue(uint8_t value) {
  uint8_t buf = value & 0x7F;
  i2c_master_transmit(m_devHandle, &buf, 1, -1);
}

int Buzzer::freqToWiperValue(int freq_hz) {
  if (freq_hz < BUZZER_FREQ_MIN)
    freq_hz = BUZZER_FREQ_MIN;
  if (freq_hz > BUZZER_FREQ_MAX)
    freq_hz = BUZZER_FREQ_MAX;

  // Quadratic regression from Arduino library: wiper% = 156.5 - 0.1303 * freq
  float percent = 156.499576f + (-0.130347337f * (float)freq_hz);

  if (percent < 0.0f)
    percent = 0.0f;
  if (percent > 100.0f)
    percent = 100.0f;

  return (int)((percent / 100.0f) * 127.0f);
}
