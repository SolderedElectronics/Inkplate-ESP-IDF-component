idf_component_register(SRCS "src/features/rtc.cpp" "src/features/pcal.cpp" "src/features/i2s.cpp"
                             "src/boards/inkplate6.cpp"
                       REQUIRES "esp_driver_i2c" "esp_driver_i2s" "esp_driver_gpio" "esp_timer"
                       INCLUDE_DIRS "include" "include/features" "include/boards/inkplate6")
