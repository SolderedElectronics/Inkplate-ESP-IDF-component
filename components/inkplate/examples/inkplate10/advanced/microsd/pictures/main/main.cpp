#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Inkplate.h"


// Image file on the SD card root
#define IMAGE_PATH "coast.jpg"

extern "C"
void app_main(void)
{
    Inkplate display;
    display.setDisplayMode(GRAYSCALE);

    if (display.sdCardInit() != ESP_OK)
    {
      ESP_LOGE(TAG, "SD card init failed");
      return;
    }

    display.image.draw(IMAGE_PATH, 0, 0, true, false);
    display.display();
    
}