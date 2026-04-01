#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Inkplate.h"

extern "C"
void app_main(void)
{
    Inkplate display;
    
    display.clearDisplay();             // Clear frame buffer of display
    display.display();                  // Put clear image on display
    display.setTextSize(2);             // Scale text to be two times bigger then original (5x7 px)

    double voltage = display.readBattery();                  // Read battery voltage
    char voltageStr[16];
    snprintf(voltageStr, sizeof(voltageStr), "%.2fV", voltage);

    display.clearDisplay();                                  // Clear everything in frame buffer of e-paper display
    display.setCursor(210, 120);
    display.print(voltageStr);
    display.display(); // Send everything to display (refresh the screen)
}