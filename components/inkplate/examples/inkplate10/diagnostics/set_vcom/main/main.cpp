#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Inkplate.h"

#define VCOM_VALUE (-3.0)

static void display_test_image(Inkplate &inkplate)
{
    inkplate.clearDisplay();

    double vcom = inkplate.getStoredVCOM();

    inkplate.setTextColor(0);
    inkplate.setTextSize(2);
    inkplate.setCursor(5, 5);
    inkplate.print("Stored VCOM: ");
    inkplate.print(vcom);
    inkplate.print(" V");

    int w = inkplate.width() / 8;
    int h = inkplate.height();

    for (int i = 0; i < 8; i++)
    {
        int x = w * i;
        inkplate.fillRect(x, 40, w, h, i);
    }

    inkplate.display();
}

extern "C"
void app_main(void)
{
    Inkplate inkplate;

    printf("Setting VCOM to %.2f\n", VCOM_VALUE);

    if (inkplate.setVCOM(VCOM_VALUE))
        printf("VCOM programmed OK\n");
    else
        printf("VCOM programming failed\n");

    display_test_image(inkplate);
}