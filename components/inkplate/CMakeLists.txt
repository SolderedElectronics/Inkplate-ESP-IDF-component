idf_component_register(SRCS "src/features/RTC.cpp" "src/features/PCAL.cpp" "src/features/I2S.cpp"
                             "src/boards/inkplate6/Inkplate6.cpp"
                             "Inkplate.cpp"
                             "graphics/Graphics.cpp"
                             "graphics/Adafruit_GFX/Adafruit_GFX.cpp"
                       REQUIRES "esp_driver_i2c" "esp_driver_i2s" "esp_driver_gpio" "esp_timer"
                       INCLUDE_DIRS "." "include" "include/features" "include/boards/inkplate6")
