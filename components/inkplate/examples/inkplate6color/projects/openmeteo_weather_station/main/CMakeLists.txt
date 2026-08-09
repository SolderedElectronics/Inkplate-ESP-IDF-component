idf_component_register(SRCS "main.cpp" "Network.cpp" "WeatherData.cpp" "Gui.cpp"
                       REQUIRES "inkplate" "json" "mbedtls"
                       INCLUDE_DIRS ".")
