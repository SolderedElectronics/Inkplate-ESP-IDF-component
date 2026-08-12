/**
 * @file        includes.h
 * @author      Fran Fodor for Soldered
 * @brief       Shared includes, fonts, and time-zone helper for the Google
 *              Calendar example (ported from Inkplate6COLOR_Google_Calendar).
 */
#pragma once

#include "Inkplate.h"
#include <ctime>

#include "Gui.h"
#include "Network.h"
#include "calendarData.h"

// Fonts used by Gui.cpp to render the calendar view.
#include "fonts/FreeSans9pt7b.h"
#include "fonts/FreeSans12pt7b.h"
#include "fonts/FreeSans18pt7b.h"
#include "fonts/FreeSansBold24pt7b.h"

// TODO: set your UTC offset in whole hours, e.g. 2 for Osijek/UTC+2, -4 for
// New York City. The original Arduino sketch called
// configTime(timeZone * 3600, 0, ntpServer) once at boot so that every later
// getLocalTime() call returned local time directly. This port instead syncs
// only the UTC epoch via display.wifi.setCurrentTime() (SNTP), so every
// local-time computation below has to apply this offset by hand.
#define TIME_ZONE_OFFSET_HOURS 2

/**
 * @brief Compute the current local wall-clock time from the SNTP-synced UTC
 *        epoch plus TIME_ZONE_OFFSET_HOURS.
 *
 * Stands in for Arduino's getLocalTime(): returns false if the system clock
 * has not been synced yet (i.e. the epoch is still near the 1970 default).
 *
 * @param timeinfo Out: local time broken down into struct tm fields.
 * @return true if the clock has been synced and timeinfo was filled in.
 */
static inline bool getLocalTimeAdjusted(struct tm *timeinfo) {
  time_t now = time(nullptr) + (time_t)TIME_ZONE_OFFSET_HOURS * 3600;
  if (now < 1700000000) // Sanity check: before ~Nov 2023 means SNTP hasn't synced yet.
    return false;
  gmtime_r(&now, timeinfo);
  return true;
}
