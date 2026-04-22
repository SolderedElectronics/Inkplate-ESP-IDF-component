#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Inkplate.h"
#include "image_ex.h"

static void show_gradient(Inkplate &inkplate, int selected)
{
    inkplate.clearDisplay();

    int w = inkplate.width() / 8;
    int h = inkplate.height() - 100;

    inkplate.fillRect(0, 725, 1200, 100, 7);

    inkplate.setTextSize(4);
    inkplate.setTextColor(0);
    inkplate.setCursor(10, 743);

    inkplate.drawRect((selected * 6 * 4 * 2) + 432 - 3, 740, (6 * 4) + 2, (8 * 4) + 2, 0);

    for (int i = 0; i < 8; i++)
        inkplate.fillRect(i * w, 0, w, h, i);

    inkplate.setTextSize(3);
    inkplate.setCursor(10, 792);
    inkplate.print("Waveform ");
    inkplate.print(selected);

    inkplate.display();
}

extern "C"
void app_main(void)
{
    Inkplate inkplate;

    for (int waveform = 1; waveform <= 5; waveform++)
    {
        printf("Testing waveform %d...\n", waveform);

        inkplate.setWaveform(waveform, false);
        show_gradient(inkplate, waveform);

        vTaskDelay(pdMS_TO_TICKS(5000)); // 3 seconds per waveform
    }
}