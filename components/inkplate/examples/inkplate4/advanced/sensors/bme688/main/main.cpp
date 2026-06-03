/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Reads environmental data from the on-board BME688 sensor and
 *              displays the values on Inkplate 4TEMPERA.
 *
 * @details     Demonstrates how to use the built-in Bosch BME688 sensor on
 *              Inkplate 4TEMPERA. The example repeatedly reads:
 *              - Temperature (with an adjustable calibration offset)
 *              - Relative humidity
 *              - Barometric pressure
 *              - Gas resistance
 *              - Estimated altitude (derived from pressure)
 *
 *              Values are rendered to the e-paper display and updated once per
 *              second. Partial updates are used most of the time; a full
 *              refresh is forced periodically to limit ghosting.
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
 * 2) Values update every ~1 second.
 * 3) Adjust TEMP_OFFSET if temperature reads consistently high/low.
 *
 * Expected output:
 * - Temperature (°C), humidity (%), pressure (hPa), gas resistance (mΩ),
 *   and altitude (m) displayed and updated every second.
 *
 * Notes:
 * - Partial update is supported only in 1-bit (black & white) mode.
 * - Altitude is estimated from pressure and is not a precision measurement.
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
#include "icons.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "BME688";

// Temperature calibration offset in degrees Celsius
static const float TEMP_OFFSET = -4.0f;

extern "C" void app_main(void) {
  Inkplate display;

  display.setDisplayMode(BLACK_AND_WHITE);
  display.clearDisplay();
  display.display();

  display.setTextSize(3);
  display.setTextColor(BLACK, WHITE);

  ESP_LOGI(TAG, "BME688 demo running");

  int n = 0;

  while (true) {
    float temperature = display.bme.readTemperature() + TEMP_OFFSET;
    float humidity = display.bme.readHumidity();
    float pressure = display.bme.readPressure();
    float gasResistance = display.bme.readGasResistance();
    float altitude = display.bme.readAltitude();

    display.clearDisplay();

    // Temperature — top left
    display.image.draw(temperature_icon, 93, 100, temperature_icon_w,
                       temperature_icon_h, BLACK);
    display.setCursor(68, 69);
    display.print("Temperature: ");
    display.setCursor(100, 241);
    display.print(temperature, 2);
    display.print(" C");

    // Humidity — top right
    display.image.draw(humidity_icon, 378, 100, humidity_icon_w,
                       humidity_icon_h, BLACK);
    display.setCursor(378, 69);
    display.print("Humidity: ");
    display.setCursor(386, 241);
    display.print(humidity, 2);
    display.print(" %");

    // Pressure — bottom left
    display.image.draw(pressure_icon, 93, 368, pressure_icon_w,
                       pressure_icon_h, BLACK);
    display.setCursor(89, 337);
    display.print("Pressure: ");
    display.setCursor(85, 509);
    display.print(pressure, 2);
    display.print(" hPa");

    // Gas resistance — bottom right
    display.setCursor(312, 342);
    display.print("Gas resistance:");
    display.setCursor(312, 393);
    display.print(gasResistance, 2);
    display.print(" mOhm");

    // Altitude — bottom right below gas
    display.setCursor(312, 450);
    display.print("Altitude:");
    display.setCursor(312, 501);
    display.print(altitude, 2);
    display.print(" m");

    ESP_LOGI(TAG, "T=%.2f H=%.2f P=%.2f G=%.2f A=%.2f", temperature, humidity,
             pressure, gasResistance, altitude);

    if (n > 9) {
      display.display();
      n = 0;
    } else {
      display.partialUpdate(false, true);
      n++;
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
