/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Reads battery statistics from the on-board BQ27441 fuel gauge
 *              and displays them on Inkplate 4TEMPERA.
 *
 * @details     Demonstrates how to use the built-in BQ27441-G1A fuel gauge on
 *              Inkplate 4TEMPERA. The example sets the configured battery
 *              capacity and then periodically reads:
 *              - State of charge (SoC %)
 *              - Voltage (mV)
 *              - Average current (mA)
 *              - Full and remaining capacity (mAh)
 *              - Average power draw (mW)
 *              - State of health (SoH %)
 *
 *              Values are displayed in 1-bit BW mode. Partial updates are used
 *              for faster refreshes; a full refresh is forced periodically to
 *              reduce ghosting. Display updates every 2 seconds.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 4TEMPERA
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 4TEMPERA, USB cable, Li-Ion battery
 * - Extra:      None
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate4
 *
 * How to use:
 * 1) Connect a Li-Ion battery to Inkplate 4TEMPERA.
 * 2) Set BATTERY_CAPACITY to your battery capacity in mAh.
 * 3) Build and flash to Inkplate 4TEMPERA.
 * 4) Screen updates every ~2 seconds with live fuel gauge readings.
 *
 * Expected output:
 * - Text lines for SoC, voltage, current, full/remaining capacity,
 *   power draw, and state of health.
 *
 * Notes:
 * - Partial update is supported only in 1-bit (black & white) mode.
 * - Set BATTERY_CAPACITY to match your battery for accurate readings.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE4
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate4 in the boards menu."
#endif

#include "Inkplate.h"
#include "batteryIcon.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "FUEL_GAUGE";

// Set to match your battery capacity in mAh
#define BATTERY_CAPACITY 1200

#define NUM_PARTIAL_UPDATES_BEFORE_FULL_REFRESH 15

extern "C" void app_main(void) {
  Inkplate display;

  display.setDisplayMode(BLACK_AND_WHITE);
  display.clearDisplay();
  display.display();

  display.bq.setCapacity(BATTERY_CAPACITY);

  display.setTextSize(3);
  display.setTextColor(BLACK, WHITE);

  ESP_LOGI(TAG, "Fuel gauge demo running, battery capacity set to %d mAh",
           BATTERY_CAPACITY);

  int numRefreshes = 0;

  while (true) {
    int soc = display.bq.soc();
    int volts = display.bq.voltage();
    int current = display.bq.current(AVG);
    int fullCap = display.bq.capacity(FULL);
    int remainCap = display.bq.capacity(REMAIN);
    int power = display.bq.power();
    int health = display.bq.soh();

    ESP_LOGI(TAG,
             "SoC=%d%% V=%dmV I=%dmA full=%dmAh rem=%dmAh P=%dmW SoH=%d%%",
             soc, volts, current, fullCap, remainCap, power, health);

    display.clearDisplay();
    display.image.draw(batteryIcon, 0, 0, batteryIcon_w, batteryIcon_h, BLACK);
    display.fillRect(195, 425, (int)(202 * (soc / 100.0f)), 95, BLACK);

    const char *infoNames[] = {
        "State of charge (%): ",
        "Voltage (mV): ",
        "Avg. current (mA): ",
        "Full capacity (mAh): ",
        "Remaining cap (mAh): ",
        "Power draw (mW): ",
        "State of Health (%): "
    };
    int data[] = {soc, volts, current, fullCap, remainCap, power, health};
    for (int i = 0; i < 7; i++) {
      display.setCursor(30, 30 + 45 * i);
      display.print(infoNames[i]);
      display.print(data[i]);
    }

    if (numRefreshes > NUM_PARTIAL_UPDATES_BEFORE_FULL_REFRESH) {
      display.display();
      numRefreshes = 0;
    } else {
      display.partialUpdate(false, true);
      numRefreshes++;
    }

    vTaskDelay(pdMS_TO_TICKS(2000));
  }
}
