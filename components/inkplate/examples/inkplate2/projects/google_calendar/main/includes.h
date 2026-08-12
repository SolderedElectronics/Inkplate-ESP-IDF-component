/**
 * @file        includes.h
 * @brief       Aggregates the headers/fonts used by this example.
 *
 * @details     Mirrors the original Arduino sketch's src/includes.h.
 *              HTTPClient.h + ArduinoJson.h are replaced by
 *              esp_http_client + cJSON (used inside Network.cpp), and
 *              Arduino's WiFi.h is replaced by Inkplate's built-in
 *              display.wifi helper (see main.cpp) - so neither is included
 *              here.
 */

#pragma once

#include "Inkplate.h"

#include "calendarData.h"
#include "Gui.h"
#include "Network.h"

// font (same subset the original includes.h pulled in for setup())
#include "fonts/FreeSans9pt7b.h"
#include "fonts/FreeSans12pt7b.h"
#include "fonts/FreeSans18pt7b.h"
#include "fonts/FreeSans48pt7b.h"
#include "fonts/FreeSansBold24pt7b.h"
#include "fonts/FreeSansBold48pt7b.h"
