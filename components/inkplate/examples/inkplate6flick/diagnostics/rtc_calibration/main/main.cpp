/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Calibrate the Inkplate 6 Flick RTC by selecting crystal load
 *              capacitance and applying a clock offset, then display the time
 *              with periodic partial/full refreshes.
 *
 * @details     This example demonstrates how to improve RTC accuracy on
 *              Inkplate 6 Flick by configuring the PCF85063(A) real-time clock.
 *              Two calibration mechanisms are shown:
 *
 *              1) Load capacitance selection:
 *                 Some boards populate external load capacitors for the 32.768
 *                 kHz crystal. If you choose to use the RTC's internal capacitor
 *                 setting instead, external capacitors must be removed. The
 *                 sketch shows how to select an internal capacitor value (e.g.,
 *                 7 pF or 12.5 pF) using setInternalCapacitor().
 *
 *              2) Clock offset correction:
 *                 The RTC supports a programmable offset (in ppm-equivalent
 *                 steps) applied periodically. setClockOffset(mode, value)
 *                 configures how often the correction is applied (mode) and the
 *                 signed correction magnitude (value).
 *
 *              After configuration, the sketch waits for a button press, sets
 *              an initial time, and then reads the RTC once per second and
 *              updates the e-paper display. To reduce flashing, it uses partial
 *              updates most of the time and forces a full refresh after a
 *              defined number of partial updates.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 6 Flick
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 6 Flick, USB cable
 * - Extra:      None.
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate6 Flick
 *
 * How to use:
 * 1) Decide whether you are using external crystal capacitors or the RTC's
 *    internal capacitor setting:
 *    - Using internal capacitor: remove external capacitors and enable
 *      setInternalCapacitor(...).
 *    - Using external capacitors: comment out setInternalCapacitor(...).
 * 2) (Optional) Determine and set the clock offset:
 *    - Best: measure the 32.768 kHz clock frequency and compute ppm deviation,
 *      then choose mode and offset register value accordingly.
 *    - Alternative: run without setClockOffset(), compare RTC time drift over
 *      2-3 days, estimate frequency error, then compute and apply an offset.
 * 3) Build and flash to Inkplate 6 Flick.
 * 4) Press the wake button when prompted to start the RTC counter.
 * 5) Observe the displayed time; adjust capacitor/offset values if needed and
 *    re-upload.
 *
 * Expected output:
 * - E-paper: A prompt to press the wake button, then a large HH:MM:SS time that
 *   updates about once per second.
 *
 * Notes:
 * - Display mode is 1-bit (BW). Partial updates are supported only in BW mode.
 * - The displayed seconds may appear to "skip" or look uneven because e-paper
 *   refresh takes time; the RTC time itself continues accurately.
 * - Partial update best practice: do a full refresh every 5-10 partial updates
 *   to maintain image quality (this example enforces a threshold).
 * - partialUpdate(false, true) keeps the e-paper power enabled for faster
 *   successive updates (higher power usage).
 * - RTC offset parameters are hardware-specific; refer to the PCF85063(A)
 *   datasheet section on offset calibration for exact ppm/LSB behavior.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE6FLICK
#error \
    "Wrong board selection for this example, please select Inkplate6 Flick in the boards menu."
#endif

#include "driver/gpio.h"
#include "esp_timer.h"
#include "time.h"

#include "Inkplate.h"

#define REFRESH_DELAY 1000
#define MAX_PARTIAL_UPDATES 9

int n = 0;

uint8_t hours = 0;
uint8_t minutes = 0;
uint8_t seconds = 0;

void print2Digits(Inkplate &display, uint8_t _d) {
  if (_d < 10)
    display.print('0');
  display.print(_d);
}

void printTime(Inkplate &display, uint8_t _hour, uint8_t _minutes,
               uint8_t _seconds) {
  print2Digits(display, _hour);
  display.print(':');
  print2Digits(display, _minutes);
  display.print(':');
  print2Digits(display, _seconds);
}

extern "C" void app_main(void) {
  Inkplate display;

  display.setDisplayMode(BLACK_AND_WHITE);
  display.clearDisplay();
  display.display();
  display.setTextSize(5);

  gpio_set_direction(GPIO_NUM_36, GPIO_MODE_INPUT);

  // Some Inkplates has external capacitors for RTC crystal, but you can use
  // internal one if you have issues with accuracy. IMPORTANT:
  //  - If you use an internal capacitor, you have to remove the external ones.
  //  - If you use an external one, you don't have the next line of code.
  // Here we setting internal capacitor value (7 pF):
  // display.rtc.setInternalCapacitor(0);
  // Another option is 12.5 pF:
  display.rtc.setInternalCapacitor(1);

  // Set offset for RTC crystal
  // The first argument is a mode (0 or 1):
  // 0 means that the offset is made once every two hours (Each LSB introduces
  //   an offset of 4.34 ppm)
  // 1 means that the offset is made every 4 minutes (Each LSB introduces
  //   an offset of 4.069 ppm)
  // The second argument is the offset value in decimal (from -64 to 63).
  // Example: mode 0 (4.34 ppm), offset value 15 = +65.1 ppm every 2 hours.
  // See section 8.2.3 in the PCF85063A datasheet for more details.
  display.rtc.setClockOffset(1, -63);

  // How to calculate this offset?
  // 1. Measure the frequency on the clock pin of the RTC (fMeasured).
  // 2. tMeasured = 1 / fMeasured
  // 3. D = 1 / 32768 - tMeasured
  // 4. Eppm = 1000000 * D / tMeasured
  // 5. Mode 0 -> Offset = Eppm / 4.34
  //    Mode 1 -> Offset = Eppm / 4.069
  //
  // Without an oscilloscope: run without setClockOffset(), compare displayed
  // time vs. a reference clock after 2-3 days, compute drift in seconds, then
  // derive ppm and offset from that. Re-upload with the computed value.

  display.setCursor(75, 380);
  display.print("Press the wake button to start RTC!");
  display.partialUpdate();

  while (gpio_get_level(GPIO_NUM_36) == 1) {
    vTaskDelay(1);
  }

  struct tm time = {};
  time.tm_hour = 0;
  time.tm_min = 0;
  time.tm_sec = 0;

  display.rtc.setTime(time);

  int64_t time1 = 0;

  while (true) {
    if ((esp_timer_get_time() - time1) > REFRESH_DELAY) {
      seconds = display.rtc.getSecond();
      minutes = display.rtc.getMinute();
      hours = display.rtc.getHour();

      display.clearDisplay();
      display.setCursor(380, 380);
      printTime(display, hours, minutes, seconds);

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
