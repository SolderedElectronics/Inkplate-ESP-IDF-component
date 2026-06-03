/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Calibrate the Inkplate 5 RTC by selecting crystal load
 *              capacitance and applying a clock offset, then display the time
 *              with periodic partial/full refreshes.
 *
 * @details     This example demonstrates how to improve RTC accuracy on
 *              Inkplate 5 by configuring the PCF85063(A) real-time clock.
 *              Two calibration mechanisms are shown:
 *
 *              1) Load capacitance selection:
 *                 Some boards populate external load capacitors for the 32.768
 *                 kHz crystal. If you choose to use the RTC's internal
 *                 capacitor setting instead, external capacitors must be
 *                 removed. The sketch shows how to select an internal capacitor
 *                 value (7 pF or 12.5 pF) using setInternalCapacitor().
 *
 *              2) Clock offset correction:
 *                 The RTC supports a programmable offset (in ppm-equivalent
 *                 steps) applied periodically. setClockOffset(mode, value)
 *                 configures how often the correction is applied (mode) and
 *                 the signed correction magnitude (value).
 *
 *              After configuration, the sketch waits for the wake button press,
 *              sets an initial time of 00:00:00, and then reads the RTC once
 *              per second and updates the e-paper display. To reduce flashing,
 *              it uses partial updates most of the time and forces a full
 *              refresh after a defined number of partial updates.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 5
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 5, USB cable
 * - Extra:      None.
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate5
 *
 * How to use:
 * 1) Decide whether you are using external crystal capacitors or the RTC's
 *    internal capacitor setting:
 *    - Using internal capacitor: remove external capacitors and enable
 *      setInternalCapacitor(1) for 12.5 pF, or setInternalCapacitor(0) for
 *      7 pF.
 *    - Using external capacitors: comment out setInternalCapacitor().
 * 2) (Optional) Determine and set the clock offset:
 *    - Best: measure the 32.768 kHz clock frequency and compute ppm deviation,
 *      then choose mode and offset register value accordingly.
 *    - Alternative: run without setClockOffset(), compare RTC time drift over
 *      2-3 days, estimate frequency error, then compute and apply an offset.
 * 3) Build and flash to Inkplate 5.
 * 4) Press the wake button when prompted to start the RTC counter.
 * 5) Observe the displayed time; adjust capacitor/offset values if needed and
 *    re-flash.
 *
 * Expected output:
 * - E-paper: A prompt to press the wake button, then a large HH:MM:SS time
 *   that updates about once per second.
 *
 * Notes:
 * - Display mode is 1-bit (BW). Partial updates are supported only in BW mode.
 * - The displayed seconds may appear to "skip" or look uneven because e-paper
 *   refresh takes time; the RTC time itself continues accurately.
 * - partialUpdate(false, true) keeps the e-paper power enabled for faster
 *   successive updates (higher power usage).
 * - RTC offset parameters are hardware-specific; refer to the PCF85063(A)
 *   datasheet section 8.2.3 on offset calibration for exact ppm/LSB behavior.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE5
#error \
    "Wrong board selection for this example, please select Inkplate5 in the boards menu."
#endif

#include "driver/gpio.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "time.h"

#include "Inkplate.h"

#define REFRESH_DELAY     1000 // ms between display refreshes
#define MAX_PARTIAL_UPDATES 9  // full refresh after this many partial updates

static void print2Digits(Inkplate &display, uint8_t d) {
  if (d < 10)
    display.print('0');
  display.print(d);
}

extern "C" void app_main(void) {
  Inkplate display;

  display.setDisplayMode(BLACK_AND_WHITE);
  display.clearDisplay();
  display.display();

  gpio_set_direction(GPIO_NUM_36, GPIO_MODE_INPUT);

  // Crystal load capacitance:
  // comment out if using external capacitors on the board.
  // display.rtc.setInternalCapacitor(0); // 7 pF
  display.rtc.setInternalCapacitor(1);    // 12.5 pF

  // Clock offset correction — see PCF85063A datasheet 8.2.3.
  // Mode 0: correction every ~2 h (4.34 ppm / LSB)
  // Mode 1: correction every ~4 min (4.069 ppm / LSB)
  // Value range: -64 to 63
  display.rtc.setClockOffset(1, -63);

  // Prompt user to press the wake button
  display.setTextSize(3);
  display.setCursor(75, 310);
  display.print("Press the wake button to start RTC!");
  display.partialUpdate();

  // Wait for wake button (active low)
  while (gpio_get_level(GPIO_NUM_36) == 1)
    vTaskDelay(1);

  // Start RTC at 00:00:00
  struct tm time = {};
  display.rtc.setTime(time);

  int n = 0;
  uint64_t time1 = 0;

  while (true) {
    if ((esp_timer_get_time() - time1) > (uint64_t)REFRESH_DELAY * 1000) {
      uint8_t seconds = display.rtc.getSecond();
      uint8_t minutes = display.rtc.getMinute();
      uint8_t hours   = display.rtc.getHour();

      display.clearDisplay();
      display.setTextSize(5);
      display.setCursor(490, 340);
      print2Digits(display, hours);
      display.print(':');
      print2Digits(display, minutes);
      display.print(':');
      print2Digits(display, seconds);

      if (n > MAX_PARTIAL_UPDATES) {
        display.display(true);
        n = 0;
      } else {
        display.partialUpdate(false, true);
        n++;
      }

      time1 = esp_timer_get_time();
    }
  }
}
