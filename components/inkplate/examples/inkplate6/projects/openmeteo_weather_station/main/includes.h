/**
 * @file        includes.h
 * @brief       Shared includes for the Open-Meteo weather station example
 *              (ported from Inkplate6_OpenMeteo_Weather_Station's
 *              src/includes.h).
 *
 * @details     The original Arduino includes.h pulled in WiFi.h,
 *              HTTPClient.h and ArduinoJson.h; those are Arduino-specific
 *              and are replaced here by esp_http_client + cJSON, used
 *              internally by Network.cpp (WiFi itself is handled by
 *              display.wifi in main.cpp, matching the rest of this
 *              component's examples). The custom classes and the
 *              fonts/icons Gui.cpp draws with are still gathered here so
 *              main.cpp only needs a single include.
 */

#pragma once

#include "Inkplate.h"

// custom classes
#include "Gui.h"
#include "Network.h"
#include "WeatherData.h"

// fonts used by Gui.cpp
#include "fonts/FreeSans9pt7b.h"
#include "fonts/FreeSans12pt7b.h"
#include "fonts/FreeSans18pt7b.h"
#include "fonts/FreeSans24pt7b.h"
#include "fonts/FreeSans32pt7b.h"

// all the weather icons
#include "binary_Icons/icon_s_clear_sky.h"
#include "binary_Icons/icon_s_fog.h"
#include "binary_Icons/icon_s_gray.h"
#include "binary_Icons/icon_s_moon.h"
#include "binary_Icons/icon_s_partly_cloudy.h"
#include "binary_Icons/icon_s_rain.h"
#include "binary_Icons/icon_s_snow.h"
#include "binary_Icons/icon_s_storm.h"
#include "binary_Icons/icon_s_thermometer.h"

// all the battery icons
#include "binary_Icons/icon_s_full_battery.h"
#include "binary_Icons/icon_s_half_battery.h"
#include "binary_Icons/icon_s_high_battery.h"
#include "binary_Icons/icon_s_low_battery.h"
