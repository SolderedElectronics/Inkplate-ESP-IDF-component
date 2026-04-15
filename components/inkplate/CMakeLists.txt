if(CONFIG_INKPLATE_BOARD_INKPLATE10)
    set(BOARD_SRC "src/boards/Inkplate10.cpp")
elseif(CONFIG_INKPLATE_BOARD_INKPLATE6)
    set(BOARD_SRC "src/boards/Inkplate6.cpp" "src/features/I2S.cpp")
elseif(CONFIG_INKPLATE_BOARD_INKPLATE6FLICK)
    set(BOARD_SRC "src/boards/Inkplate6.cpp" "src/features/I2S.cpp")
elseif(CONFIG_INKPLATE_BOARD_INKPLATE6COLOR)
    set(BOARD_SRC "src/boards/Inkplate6Color.cpp")
elseif(CONFIG_INKPLATE_BOARD_INKPLATE5)
    set(BOARD_SRC "src/boards/Inkplate5.cpp")
elseif(CONFIG_INKPLATE_BOARD_INKPLATE4)
    set(BOARD_SRC "src/boards/Inkplate4.cpp")
    set(SENSOR_SRCS
        "src/features/APDS9960/APDS9960.cpp"
        "src/features/APDS9960/libs/APDS9960Driver.cpp"
        "src/features/BME680/BME680.cpp"
        "src/features/BME680/libs/BME680Driver.cpp"
        "src/features/BQ27441/BQ27441.cpp"
        "src/features/BQ27441/libs/BQ27441Driver.cpp"
        "src/features/LSM6DS3/LSM6DS3.cpp"
        "src/features/LSM6DS3/libs/LSM6DS3Driver.cpp")
    set(SENSOR_INCLUDES
        "include/features/APDS9960"
        "include/features/APDS9960/libs"
        "include/features/BME680"
        "include/features/BME680/libs"
        "include/features/BQ27441"
        "include/features/BQ27441/libs"
        "include/features/LSM6DS3"
        "include/features/LSM6DS3/libs")
elseif(CONFIG_INKPLATE_BOARD_INKPLATE2)
    set(BOARD_SRC "src/boards/Inkplate2.cpp")
endif()

set(DECODER_SRCS
    "src/graphics/Decoders/BMP.cpp"
    "src/graphics/Decoders/JPEG.cpp"
    "src/graphics/Decoders/PNG.cpp"
    "src/graphics/Decoders/pngle/pngle.c"
)

set(DECODER_INCLUDES
    "include/graphics/Decoders"
    "include/graphics/Decoders/pngle"
)

set(IMAGE_COLOR_SRCS
    "src/graphics/ImageColor.cpp"
)

set(GRAPHICS_SRCS
    "src/graphics/Graphics.cpp"
    "src/graphics/Adafruit_GFX/Adafruit_GFX.cpp"
    "src/graphics/Shapes.cpp"
    "src/graphics/ShapesPolygon.cpp"
)

set(GRAPHICS_INCLUDES
    "include/graphics"
    "include/graphics/Adafruit_GFX"
)

set(IMAGE_SRCS
    "src/graphics/Image.cpp"
)

if(CONFIG_INKPLATE_BOARD_INKPLATE2)
idf_component_register(SRCS "src/features/Network.cpp"
                            "src/boards/Inkplate.cpp"
                            ${BOARD_SRC}
                            ${GRAPHICS_SRCS}
                            ${IMAGE_COLOR_SRCS}
                            ${DECODER_SRCS}
                       REQUIRES "esp_driver_gpio"
                                "esp_driver_spi"
                                "esp_timer"
                                "esp_wifi" "nvs_flash" "esp_http_client"
                       INCLUDE_DIRS "include"
                                    "include/features"
                                    "include/boards"
                                    ${GRAPHICS_INCLUDES}
                                    ${IMAGE_COLOR_INCLUDES}
                                    ${DECODER_INCLUDES}
                                    )
else()
idf_component_register(SRCS "src/features/I2C.cpp" "src/features/RTC.cpp" "src/features/Frontlight.cpp" 
                            "src/features/PCAL.cpp" "src/features/TPS.cpp" "src/features/I2S.cpp"
                            "src/features/Network.cpp" "src/features/SDCard.cpp"
                            "src/boards/BoardCommon.cpp"
                            "src/boards/Inkplate.cpp"
                            ${BOARD_SRC}
                            ${SENSOR_SRCS}
                            ${GRAPHICS_SRCS}
                            ${IMAGE_SRCS}
                            ${DECODER_SRCS}
                       REQUIRES "esp_driver_i2c" "esp_driver_i2s" "esp_driver_gpio" "esp_timer" "esp_adc"
                                "esp_driver_spi" "fatfs" "sdmmc"
                                "esp_wifi" "nvs_flash" "esp_http_client"
                       INCLUDE_DIRS "include" 
                                    "include/features"
                                    "include/boards"
                                    ${GRAPHICS_INCLUDES}
                                    ${IMAGE_INCLUDES}
                                    ${SENSOR_INCLUDES}
                                    ${DECODER_INCLUDES}
                                    )
endif()