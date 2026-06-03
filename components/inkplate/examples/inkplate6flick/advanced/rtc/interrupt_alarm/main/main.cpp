/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       RTC alarm with GPIO interrupt example for Soldered Inkplate 6 Flick.
 *
 * @details     Demonstrates how to use the PCF85063A real-time clock (RTC)
 *              alarm functionality together with its hardware interrupt output
 *              on Inkplate 6 Flick. Instead of polling the alarm flag in
 *              software, this example configures a falling-edge ISR on the
 *              RTC interrupt pin (GPIO39) so the alarm is detected immediately
 *              without spinning on an I2C register.
 *
 *              Time is set via a Unix epoch value for convenience. The alarm
 *              fires 10 seconds after the configured start time. When the ISR
 *              sets the flag, the main loop clears the RTC alarm flag and
 *              prints "ALARM" on the display.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 6 Flick
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 6 Flick, USB cable
 * - Extra:      None
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate6 Flick
 *
 * How to use:
 * 1) Build and flash to Inkplate 6 Flick.
 * 2) The RTC is set to a fixed epoch on boot; the alarm is configured to fire
 *    10 seconds later.
 * 3) A GPIO ISR on GPIO39 (RTC INT pin) is installed to catch the falling edge
 *    produced by the RTC when the alarm fires.
 * 4) Current time and alarm status are displayed on the screen.
 *
 * Expected output:
 * - E-paper display shows the current date and time, updating roughly every 700 ms.
 * - After 10 seconds "ALARM" appears on the display.
 *
 * Notes:
 * - Inkplate 6 Flick uses the PCF85063A RTC chip.
 * - The RTC interrupt pin is connected to GPIO39 (input only, no pull-up needed
 *   as the RTC drives the line open-drain with an external pull-up on the board).
 * - Partial update works only in 1-bit (black & white) mode.
 * - It is not recommended to use partial update on the first refresh after
 *   power-up.
 * - Perform a full refresh every 5-10 partial updates to maintain display
 *   quality.
 * - The ISR only sets a flag; all I2C and display operations are done in the
 *   main task to avoid illegal calls from interrupt context.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE6FLICK
#error \
    "Wrong board selection for this example, please select Inkplate6 Flick in the boards menu."
#endif

#include "Inkplate.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "time.h"

static const char *TAG = "RTC_INT_ALARM";

#define REFRESH_DELAY_MS 700
#define RTC_INT_PIN      GPIO_NUM_39

// Set by the ISR when the RTC pulls the interrupt line low
static volatile int s_alarmFlag = 0;

// ISR runs in IRAM; only set a flag — no I2C or display calls here
static void IRAM_ATTR alarmISR(void *arg) { s_alarmFlag = 1; }

static void print2Digits(Inkplate &display, uint8_t d) {
  if (d < 10)
    display.print('0');
  display.print(d);
}

static void printTime(Inkplate &display, uint8_t hour, uint8_t minutes,
                      uint8_t seconds, uint8_t day, uint8_t weekday,
                      uint8_t month, uint16_t year) {
  const char *wday[] = {"Sunday",   "Monday", "Tuesday", "Wednesday",
                        "Thursday", "Friday", "Saturday"};

  print2Digits(display, hour);
  display.print(':');
  print2Digits(display, minutes);
  display.print(':');
  print2Digits(display, seconds);
  display.print(' ');
  display.print(wday[weekday % 7]);
  display.print(", ");
  print2Digits(display, day);
  display.print('/');
  print2Digits(display, month);
  display.print('/');
  display.print(year);
}

extern "C" void app_main(void) {
  // Init Inkplate first — TouchCypress::begin() calls gpio_install_isr_service()
  // internally. Calling it again before Inkplate init would cause an abort.
  Inkplate display;

  // Configure GPIO39 as input with falling-edge interrupt after display init.
  // Do NOT call gpio_install_isr_service() here — Inkplate already did it.
  gpio_config_t io_conf = {};
  io_conf.pin_bit_mask = (1ULL << RTC_INT_PIN);
  io_conf.mode         = GPIO_MODE_INPUT;
  io_conf.intr_type    = GPIO_INTR_NEGEDGE; // RTC pulls line low on alarm
  gpio_config(&io_conf);
  gpio_isr_handler_add(RTC_INT_PIN, alarmISR, NULL);

  display.setDisplayMode(BLACK_AND_WHITE);
  display.clearDisplay();
  display.display();
  display.setTextSize(4);
  display.setTextColor(BLACK, WHITE);

  // Set time via Unix epoch (1589610300 = 2020-05-16 06:25:00 UTC)
  esp_err_t err = display.rtc.setTime((time_t)1589610300);
  if (err != ESP_OK)
    ESP_LOGE(TAG, "setTime failed: %s", esp_err_to_name(err));

  // Set alarm 10 seconds after start time
  err = display.rtc.setAlarmEpoch((time_t)(1589610300 + 10));
  if (err != ESP_OK)
    ESP_LOGE(TAG, "setAlarmEpoch failed: %s", esp_err_to_name(err));

  int n = 0;

  while (true) {
    vTaskDelay(pdMS_TO_TICKS(REFRESH_DELAY_MS));

    tm now = {};
    err = display.rtc.getTime(&now);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "getTime failed: %s", esp_err_to_name(err));
      continue;
    }

    display.clearDisplay();
    display.setCursor(50, 280);
    printTime(display, now.tm_hour, now.tm_min, now.tm_sec, now.tm_mday,
              now.tm_wday,
              now.tm_mon,   // driver returns 1-based month
              now.tm_year); // driver returns full year e.g. 2020

    // Handle interrupt-driven alarm flag (set by ISR)
    if (s_alarmFlag) {
      display.rtc.clearAlarmFlag();
      s_alarmFlag = 0;
      display.setCursor(280, 420);
      display.print("ALARM");
      ESP_LOGI(TAG, "Alarm triggered via interrupt!");
    }

    if (n > 9) {
      display.display();
      n = 0;
    } else {
      display.partialUpdate(false, true);
      n++;
    }
  }
}
