/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Demonstrates how to drive the built-in buzzer on Inkplate 4TEMPERA.
 *
 * @details     Shows basic usage of the Inkplate 4TEMPERA buzzer API. After
 *              initializing the board the example plays several demo sequences:
 *
 *              - Fixed-duration beeps using beep(duration_ms)
 *              - Manual on/off control using beepOn() and beepOff()
 *              - Frequency-controlled beeps using beep(duration_ms, freq_hz)
 *
 *              Then the main loop plays a repeating short melody based on the
 *              C Maj7 chord (C, E, G, B).
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
 * 2) Listen to the startup demo sequences.
 * 3) After the demos the board plays a short repeating melody.
 *
 * Expected output:
 * - Three short startup beeps.
 * - Two manual on/off beeps.
 * - Two low-pitch then two high-pitch beeps.
 * - Repeating C Maj7 melody pattern.
 *
 * Notes:
 * - Frequency range: ~572–2933 Hz, controlled via MCP4018 digipot.
 *   Pitch control is approximate and non-linear.
 * - beep() is a blocking call.
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
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "BUZZER";

// C Maj7 chord frequencies (Hz): C, E, G, B
static const int chord[4] = {523, 659, 783, 987};

extern "C" void app_main(void) {
  Inkplate display;

  display.setDisplayMode(BLACK_AND_WHITE);
  display.clearDisplay();
  display.display();

  // --- Demo 1: three short beeps at default frequency ---
  display.buzzer.beep(80);
  vTaskDelay(pdMS_TO_TICKS(80));
  display.buzzer.beep(80);
  vTaskDelay(pdMS_TO_TICKS(80));
  display.buzzer.beep(80);
  vTaskDelay(pdMS_TO_TICKS(80));

  ESP_LOGI(TAG, "Demo 1 done: 3 short beeps");
  vTaskDelay(pdMS_TO_TICKS(5000));

  // --- Demo 2: manual beepOn / beepOff ---
  display.buzzer.beepOn();
  vTaskDelay(pdMS_TO_TICKS(200));
  display.buzzer.beepOff();
  vTaskDelay(pdMS_TO_TICKS(200));
  display.buzzer.beepOn();
  vTaskDelay(pdMS_TO_TICKS(200));
  display.buzzer.beepOff();
  vTaskDelay(pdMS_TO_TICKS(200));

  ESP_LOGI(TAG, "Demo 2 done: manual on/off");
  vTaskDelay(pdMS_TO_TICKS(5000));

  // --- Demo 3: low then high pitched beeps ---
  display.buzzer.beep(300, 750);
  vTaskDelay(pdMS_TO_TICKS(50));
  display.buzzer.beep(300, 750);
  vTaskDelay(pdMS_TO_TICKS(50));
  display.buzzer.beep(300, 2400);
  vTaskDelay(pdMS_TO_TICKS(50));
  display.buzzer.beep(300, 2400);
  vTaskDelay(pdMS_TO_TICKS(50));

  ESP_LOGI(TAG, "Demo 3 done: low/high pitch");
  vTaskDelay(pdMS_TO_TICKS(5000));

  // --- Melody loop: C Maj7 chord pattern ---
  int noteIndex = 0;
  int repeatCounter = 0;

  while (true) {
    if (repeatCounter < 2) {
      display.buzzer.beep(100, chord[noteIndex]);
      vTaskDelay(pdMS_TO_TICKS(600));
    } else {
      display.buzzer.beep(100, chord[noteIndex]);
      vTaskDelay(pdMS_TO_TICKS(250));
      display.buzzer.beep(50, chord[noteIndex]);
      vTaskDelay(pdMS_TO_TICKS(300));
    }

    noteIndex++;
    if (noteIndex >= 4) {
      noteIndex = 0;
      repeatCounter++;
      if (repeatCounter >= 4) {
        repeatCounter = 0;
        ESP_LOGI(TAG, "Melody loop done, restarting");
        vTaskDelay(pdMS_TO_TICKS(3000));
      }
    }
  }
}
