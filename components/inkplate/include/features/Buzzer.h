/**
 * @file Buzzer.h
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

#pragma once

#include "I2C.h"
#include "PCAL.h"
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "stdint.h"

#define MCP4018_I2C_ADDR 0x2F

#define BUZZER_FREQ_MIN 572
#define BUZZER_FREQ_MAX 2933

/**
 * @brief Driver for the MCP4018 digipot-controlled passive buzzer.
 *
 * Frequency is set by adjusting the MCP4018 digital potentiometer wiper
 * position via I2C. The enable pin on the PCAL IO expander gates the
 * oscillator circuit (LOW = on, HIGH = off).
 *
 * Supported frequency range: 572–2933 Hz (approximate, non-linear).
 */
class Buzzer {
public:
  Buzzer() = default;

  /**
   * @brief Register the buzzer on the I2C bus and configure the enable pin.
   *
   * @param i2c I2C instance.
   * @param expander IO expander instance.
   * @param enablePin expander pin that gates the buzzer oscillator.
   * @return esp_err_t I2C error code.
   */
  esp_err_t begin(I2C &i2c, PCAL &expander, IOPin_t enablePin);

  /**
   * @brief Beep for a given duration at the default frequency (50% wiper).
   *
   * @param length_ms duration in milliseconds (blocking).
   */
  void beep(uint32_t length_ms);

  /**
   * @brief Beep for a given duration at a specific frequency.
   *
   * @param length_ms duration in milliseconds (blocking).
   * @param freq_hz target frequency in Hz (clamped to 572–2933 Hz).
   */
  void beep(uint32_t length_ms, int freq_hz);

  /**
   * @brief Turn the buzzer on at the default frequency (50% wiper).
   */
  void beepOn();

  /**
   * @brief Turn the buzzer on at a specific frequency.
   *
   * @param freq_hz target frequency in Hz (clamped to 572–2933 Hz).
   */
  void beepOn(int freq_hz);

  /**
   * @brief Turn the buzzer off.
   */
  void beepOff();

private:
  i2c_master_dev_handle_t m_devHandle = nullptr;
  PCAL *m_expander = nullptr;
  IOPin_t m_enablePin;

  void setWiperValue(uint8_t value);
  int freqToWiperValue(int freq_hz);
};
