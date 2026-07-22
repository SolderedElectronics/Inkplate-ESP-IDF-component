/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       NVS (EEPROM) read/write example for Soldered Inkplate 7.
 *
 * @details     Demonstrates how to use ESP-IDF NVS (Non-Volatile Storage) to
 *              store data that persists across resets and power cycles.
 *              The example shows how to safely clear, write, and read 128 bytes
 *              of user data from NVS and display the results on the screen.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 7
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 7, USB cable
 * - Extra:      None
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate7
 *
 * How to use:
 * 1) Build and flash to Inkplate 7.
 * 2) The display will clear NVS user data, write sample data, then read it back.
 *
 * Expected output:
 * - Messages indicating NVS clearing, writing, and reading.
 * - A list of values read from NVS displayed on the screen.
 *
 * Notes:
 * - NVS replaces the Arduino EEPROM library in ESP-IDF.
 * - Data is stored in a dedicated NVS flash partition and persists across resets.
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
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"

static const char *TAG = "eeprom_usage";

#define DATA_SIZE     128
#define NVS_NAMESPACE "user_data"

static void clearNVS(void) {
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
  ESP_LOGI(TAG, "NVS cleared");
}

static void writeNVS(void) {
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
  ESP_LOGI(TAG, "NVS written");
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
    nvs_get_u8(handle, key, &val);
    display.print(val);
    if (i != DATA_SIZE - 1)
      display.print(", ");
  }
  nvs_close(handle);
  display.display();
}

extern "C" void app_main(void) {
  Inkplate display;

  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES ||
      err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    nvs_flash_erase();
    nvs_flash_init();
  }

  display.clearDisplay();
  display.display();
  display.setTextColor(INKPLATE_BLACK);
  display.setTextSize(4);

  display.println("Clearing NVS...");
  display.display();
  clearNVS();
  vTaskDelay(pdMS_TO_TICKS(500));

  display.println("Writing data to NVS...");
  display.display();
  writeNVS();
  vTaskDelay(pdMS_TO_TICKS(500));

  display.println("Reading data from NVS:");
  display.display();
  display.setTextSize(2);
  printNVS(display);
}
