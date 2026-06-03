/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       RTC alarm with GPIO interrupt for Soldered Inkplate 13SPECTRA.
 *
 * @details     Demonstrates how to use the PCF85063A RTC alarm with its
 *              interrupt output on Inkplate 13SPECTRA. The example sets time
 *              via epoch, configures an alarm 60 seconds later, and handles
 *              the interrupt on GPIO 2. When the alarm fires the interrupt
 *              flag is set and "ALARM" is printed on the next display refresh.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 13SPECTRA
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 13SPECTRA, USB cable
 * - Extra:      None
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate13
 *
 * How to use:
 * 1) Build and flash to Inkplate 13SPECTRA.
 * 2) The alarm triggers 60 seconds after boot.
 * 3) "ALARM" appears on screen after the interrupt fires.
 *
 * Expected output:
 * - Current time displayed and updated every minute.
 * - "ALARM" shown in red when alarm interrupt fires.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE13
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate13 in the boards menu."
#endif

#include "Inkplate.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "time.h"

static const char *TAG = "RTC_INT_ALARM";

#define REFRESH_DELAY_MS 60000
#define RTC_INT_PIN      GPIO_NUM_2

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
  // Configure RTC interrupt pin (active low)
  gpio_config_t io_conf = {};
  io_conf.pin_bit_mask = (1ULL << RTC_INT_PIN);
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
  io_conf.intr_type = GPIO_INTR_NEGEDGE;
  gpio_config(&io_conf);

  gpio_install_isr_service(0);
  gpio_isr_handler_add(RTC_INT_PIN, alarmISR, NULL);

  Inkplate display;

  display.clearDisplay();
  display.display();
  display.setTextSize(4);

  display.rtc.reset();

  // Set time via epoch and alarm 60 seconds later
  esp_err_t err = display.rtc.setTime((time_t)1770032087);
  if (err != ESP_OK)
    ESP_LOGE(TAG, "setTime failed: %s", esp_err_to_name(err));

  err = display.rtc.setAlarmEpoch((time_t)(1770032087 + 60));
  if (err != ESP_OK)
    ESP_LOGE(TAG, "setAlarmEpoch failed: %s", esp_err_to_name(err));

  while (true) {
    tm now = {};
    err = display.rtc.getTime(&now);
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "getTime failed: %s", esp_err_to_name(err));
    } else {
      display.clearDisplay();
      display.setTextColor(INKPLATE_BLACK, INKPLATE_WHITE);
      display.setCursor(60, 100);
      printTime(display, now.tm_hour, now.tm_min, now.tm_sec, now.tm_mday,
                now.tm_wday, now.tm_mon, now.tm_year);

      if (s_alarmFlag) {
        display.rtc.clearAlarmFlag();
        display.setTextColor(INKPLATE_RED, INKPLATE_WHITE);
        display.setCursor(200, 200);
        display.print("ALARM");
        ESP_LOGI(TAG, "Alarm triggered!");
      }

      display.display();
    }

    vTaskDelay(pdMS_TO_TICKS(REFRESH_DELAY_MS));
  }
}
