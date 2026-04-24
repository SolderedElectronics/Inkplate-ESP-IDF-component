#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE10
#error "Wrong board selection for this example, please select Inkplate10 in the boards menu."
#endif

#include "Inkplate.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "EEPROM_EXAMPLE";

// In the Arduino version, addresses 0-75 were reserved for waveform data.
// In the IDF version, waveforms live in NVS under their own namespace,
// so there is no offset needed — user data starts at key index 0.
#define DATA_SIZE 128
#define NVS_NAMESPACE "user_data"

static void clearNVS(Inkplate &display)
{
    nvs_handle_t handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_open failed: %s", esp_err_to_name(err));
        return;
    }

    // Erase every key we own (key names "d000" … "d127")
    for (int i = 0; i < DATA_SIZE; i++) {
        char key[8];
        snprintf(key, sizeof(key), "d%03d", i);
        nvs_erase_key(handle, key); // ignore NOT_FOUND errors
    }

    nvs_commit(handle);
    nvs_close(handle);
    ESP_LOGI(TAG, "NVS cleared.");
}

static void writeNVS(Inkplate &display)
{
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
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "nvs_set_u8[%d] failed: %s", i, esp_err_to_name(err));
        }
    }

    nvs_commit(handle);
    nvs_close(handle);
    ESP_LOGI(TAG, "NVS written.");
}

static void printNVS(Inkplate &display)
{
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

// ---------------------------------------------------------------------------

extern "C" void app_main(void)
{
    Inkplate display;
    // NVS must be initialised before any nvs_open call.
    // nvs_flash_init() is idempotent if already initialised by another component.
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // Partition was truncated or version mismatch — erase and retry.
        ESP_LOGW(TAG, "NVS partition problem (%s), erasing…", esp_err_to_name(err));
        nvs_flash_erase();
        nvs_flash_init();
    }

    display.setDisplayMode(BLACK_AND_WHITE);
    display.clearDisplay();
    display.display();

    display.setTextSize(6);
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
    vTaskDelay(pdMS_TO_TICKS(500));

    return;
}