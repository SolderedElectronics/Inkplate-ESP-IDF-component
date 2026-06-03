/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       RTC alarm with interrupt example for Soldered Inkplate 4TEMPERA.
 *
 * @details     Demonstrates how to use the PCF85063A real-time clock (RTC)
 *              alarm functionality together with its interrupt output on
 *              Inkplate 4TEMPERA. The example shows how to set time and date,
 *              configure an alarm, read current time, print it on the display
 *              using partial updates, and handle the RTC interrupt event.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 4TEMPERA
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 4TEMPERA, USB cable
 * - Extra:      None
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate4
 *
 * How to use:
 * 1) Build and flash to Inkplate 4TEMPERA.
 * 2) Initialize RTC time and date if not already set.
 * 3) Configure the RTC alarm and enable interrupt handling.
 * 4) When the alarm triggers, the interrupt is handled in software.
 * 5) Current time and alarm status are displayed on the screen.
 *
 * Expected output:
 * - Inkplate display shows current date and time.
 * - "ALARM" text appears when the alarm interrupt fires.
 *
 * Notes:
 * - Inkplate 4TEMPERA uses the PCF85063A RTC chip.
 * - RTC interrupt is connected to GPIO39 on Inkplate 4TEMPERA.
 * - Partial update works only in 1-bit (black & white) mode.
 * - Perform a full refresh every 5–10 partial updates to maintain display
 *   quality.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE4
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate4 in the boards menu."
#endif

#include "Inkplate.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "time.h"

static const char *TAG = "RTC_INT_ALARM";

#define REFRESH_DELAY_MS 700
#define RTC_INT_PIN GPIO_NUM_39

static volatile int s_alarmFlag = 0;

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
  Inkplate display;

  gpio_config_t io_conf = {};
  io_conf.pin_bit_mask = (1ULL << RTC_INT_PIN);
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.intr_type = GPIO_INTR_NEGEDGE;
  gpio_config(&io_conf);

  gpio_install_isr_service(0);
  gpio_isr_handler_add(RTC_INT_PIN, alarmISR, NULL);

  display.setDisplayMode(BLACK_AND_WHITE);
  display.clearDisplay();
  display.display();
  display.setTextSize(3);
  display.setTextColor(BLACK, WHITE);

  esp_err_t err = display.rtc.setTime((time_t)1589610300);
  if (err != ESP_OK)
    ESP_LOGE(TAG, "setTime failed: %s", esp_err_to_name(err));

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
    display.setCursor(60, 280);
    printTime(display, now.tm_hour, now.tm_min, now.tm_sec, now.tm_mday,
              now.tm_wday, now.tm_mon, now.tm_year);

    if (s_alarmFlag) {
      display.rtc.clearAlarmFlag();
      display.setCursor(200, 350);
      display.print("ALARM");
      ESP_LOGI(TAG, "Alarm triggered!");
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
