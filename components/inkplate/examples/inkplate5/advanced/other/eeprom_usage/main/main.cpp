/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       NVS (EEPROM equivalent) read/write example for Soldered Inkplate 5.
 *
 * @details     Demonstrates how to use ESP32 Non-Volatile Storage (NVS) on
 *              Inkplate 5 to store data that persists across resets and power
 *              cycles. This is the ESP-IDF equivalent of the Arduino EEPROM
 *              library. The example shows how to clear, write, and read user
 *              data from NVS using key-value pairs.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 5
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 5, USB cable
 * - Extra:      None
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate5
 *
 * How to use:
 * 1) Build and flash to Inkplate 5.
 * 2) The display will first clear NVS user data.
 * 3) Sample data is written to NVS.
 * 4) Stored data is read back and shown on the display.
 *
 * Expected output:
 * - Messages indicating NVS clearing, writing, and reading.
 * - A list of values read from NVS displayed on the screen.
 *
 * Notes:
 * - NVS replaces Arduino EEPROM for persistent storage in ESP-IDF projects.
 * - Keys are formatted as "d000" to "d127" to emulate byte-address access.
 * - Data is stored in the NVS partition — make sure the partition table
 *   includes an NVS partition (the default sdkconfig.defaults handles this).
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

#include "Inkplate.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"

static const char *TAG = "EEPROM_EXAMPLE";

#define DATA_SIZE    128
#define NVS_NAMESPACE "user_data"

static void clearNVS(Inkplate &display) {
  nvs_handle_t handle;
  esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "nvs_open failed: %s", esp_err_to_name(err));
    return;
  }

  for (int i = 0; i < DATA_SIZE; i++) {
    char key[8];
    snprintf(key, sizeof(key), "d%03d", i);
    nvs_erase_key(handle, key);
  }

  nvs_commit(handle);
  nvs_close(handle);
  ESP_LOGI(TAG, "NVS cleared.");
}

static void writeNVS(Inkplate &display) {
  nvs_handle_t handle;
  esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "nvs_open failed: %s", esp_err_to_name(err));
    return;
  }

  for (int i = 0; i < DATA_SIZE; i++) {
    char key[8];
    snprintf(key, sizeof(key), "d%03d", i);
    err = nvs_set_u8(handle, key, (uint8_t)i);
    if (err != ESP_OK)
      ESP_LOGE(TAG, "nvs_set_u8[%d] failed: %s", i, esp_err_to_name(err));
  }

  nvs_commit(handle);
  nvs_close(handle);
  ESP_LOGI(TAG, "NVS written.");
}

static void printNVS(Inkplate &display) {
  nvs_handle_t handle;
  esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &handle);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "nvs_open failed: %s", esp_err_to_name(err));
    return;
  }

  for (int i = 0; i < DATA_SIZE; i++) {
    char key[8];
    snprintf(key, sizeof(key), "d%03d", i);

    uint8_t val = 0;
    err = nvs_get_u8(handle, key, &val);
    if (err != ESP_OK) {
      ESP_LOGW(TAG, "nvs_get_u8[%d] failed: %s", i, esp_err_to_name(err));
      val = 0;
    }

    display.print(val);
    if (i != DATA_SIZE - 1)
      display.print(", ");
  }

  nvs_close(handle);
  display.partialUpdate(false, false);
}

extern "C" void app_main(void) {
  Inkplate display;

  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
      err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_LOGW(TAG, "NVS partition problem (%s), erasing...", esp_err_to_name(err));
    nvs_flash_erase();
    nvs_flash_init();
  }

  display.setDisplayMode(BLACK_AND_WHITE);
  display.clearDisplay();
  display.display();

  display.setTextSize(4);
  display.println("Clearing NVS...");
  display.display();
  clearNVS(display);
  vTaskDelay(pdMS_TO_TICKS(500));

  display.println("Writing data to NVS...");
  display.partialUpdate(false, false);
  writeNVS(display);
  vTaskDelay(pdMS_TO_TICKS(500));

  display.println("Reading data from NVS:");
  display.partialUpdate(false, false);
  display.setTextSize(3);
  printNVS(display);
}
