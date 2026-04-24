#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "tempSymbol.h"
#include "Inkplate.h"

extern "C"
void app_main(void)
{
    Inkplate display;
    display.setDisplayMode(BLACK_AND_WHITE);

    display.clearDisplay();             // Clear frame buffer of display
    display.display();                  // Put clear image on display
    display.setTextSize(2);             // Scale text to be two times bigger then original (5x7 px)
    display.setTextColor(BLACK, WHITE); // Set text color to black and background color to white

    while(true)
    {
        int temperature = display.readTemperature();            // Read temperature from on-board temperature sensor
        display.clearDisplay();                                 // Clear frame buffer of display
        display.image.draw(tempSymbol, 100, 100, 38, 79, BLACK); // Draw temperature symbol at position X=100, Y=100
        display.setCursor(150, 125);
        display.print(temperature); // Print temperature
        display.print('C');
        display.display(); // Send everything to display (refresh the screen)
        vTaskDelay(pdMS_TO_TICKS(10000));      // Wait 10 seconds before new measurement
    }
}