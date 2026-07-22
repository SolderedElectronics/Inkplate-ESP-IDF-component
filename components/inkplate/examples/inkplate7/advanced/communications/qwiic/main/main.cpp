/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       easyC / Qwiic I2C scanner for Soldered Inkplate 7.
 *
 * @details     Scans the I2C bus for connected easyC/Qwiic devices and
 *              displays all detected addresses on the e-paper screen and in
 *              the serial log. The scan repeats every 5 seconds. Useful for
 *              verifying sensor wiring before writing sensor-specific code.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 7
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 7, USB cable, optional easyC/Qwiic device
 * - Extra:      None
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate7
 *
 * How to use:
 * 1) Connect an easyC or Qwiic device to the Inkplate connector.
 * 2) Build and flash to Inkplate 7.
 * 3) Detected I2C addresses appear on the display and in the serial log.
 *
 * Expected output:
 * - List of found I2C device addresses on the display.
 * - Scan repeats every 5 seconds.
 *
 * Notes:
 * - Valid I2C addresses range from 0x01 to 0x7E.
 * - If no device is connected, "No devices found" is shown.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE7
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate7 in the boards menu."
#endif

#include "Inkplate.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern I2C i2c;

static const char *TAG = "easy_c";

static void scanI2C(Inkplate &display) {
  int nDevices = 0;
  int yCursor = 30;

  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.print("Scanning I2C...");
  display.setTextSize(1);

  ESP_LOGI(TAG, "Scanning...");

  for (uint8_t address = 1; address < 0x7F; address++) {
    esp_err_t ret = i2c_master_probe(i2c.getBusHandle(), address, 50);
    if (ret == ESP_OK) {
      ESP_LOGI(TAG, "Found device at 0x%02X", address);
      display.setCursor(0, yCursor);
      display.print("Found: 0x");
      if (address < 0x10)
        display.print("0");
      display.print(address, 16);
      yCursor += 12;
      nDevices++;
    }
  }

  if (nDevices == 0) {
    ESP_LOGI(TAG, "No I2C devices found");
    display.setCursor(0, yCursor);
    display.print("No devices found.");
  } else {
    ESP_LOGI(TAG, "Scan done: %d device(s)", nDevices);
  }

  display.display();
}

extern "C" void app_main(void) {
  Inkplate display;

  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(INKPLATE_BLACK);
  display.setCursor(0, 0);
  display.print("Inkplate Qwiic Scanner");
  display.display();

  while (true) {
    scanI2C(display);
    vTaskDelay(pdMS_TO_TICKS(5000));
  }
}
