#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "inkplate6.h"

static Inkplate6 inkplate;

extern "C"
void app_main(void)
{
  inkplate.begin();

  inkplate.clearDisplay();
  inkplate.display3b();
  vTaskDelay(pdMS_TO_TICKS(5000));

  inkplate.fillDisplay();
  inkplate.display3b();
}
