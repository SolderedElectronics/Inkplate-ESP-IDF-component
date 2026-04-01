#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Inkplate.h"

extern "C"
void app_main(void)
{
    Inkplate display;

    // --- rotation 0 (normal) ---
    display.setRotation(0);
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(10, 10);
    display.print("Rotation 1");
    display.setCursor(10, 40);
    display.display();

    vTaskDelay(1000);

    display.setRotation(1);
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(10, 10);
    display.print("Rotation 2");
    display.setCursor(10, 40);
    display.display();

    vTaskDelay(1000);
    display.setRotation(2);
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(10, 10);
    display.print("Rotation 3");
    display.setCursor(10, 40);
    display.display();

    vTaskDelay(1000);
    display.setRotation(3);
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(10, 10);
    display.print("Rotation 4");
    display.setCursor(10, 40);
    display.display();
}
