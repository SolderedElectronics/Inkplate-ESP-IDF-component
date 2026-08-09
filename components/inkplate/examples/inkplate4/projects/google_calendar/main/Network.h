/**
 * @file        Network.h
 * @author      Fran Fodor for Soldered
 * @brief       Fetches events from a public Google Calendar via the Google
 *              Calendar API.
 *
 * Ported from the Inkplate4TEMPERA_Google_Calendar Arduino example. The
 * original used HTTPClient + ArduinoJson; this port performs the same
 * single HTTP GET with esp_http_client and parses the JSON response with
 * cJSON. Note that the original example (and this port) authenticates with
 * a simple Google API key against a *public* calendar - it does not use the
 * OAuth2 refresh-token flow needed for private calendars.
 */
#pragma once

#include "calendarData.h"

/**
 * @brief Fetches events from a public Google Calendar (Calendar ID + API
 *        key) and fills a calendarData with the result.
 */
class NetworkFunctions {
public:
  /**
   * @brief Construct with the target calendar and API key.
   *
   * @param calendarID Public Google Calendar ID, e.g.
   *                    "yourcalendar@group.calendar.google.com".
   * @param apiKey Google API key with the Calendar API enabled.
   */
  NetworkFunctions(const char *calendarID, const char *apiKey);

  /**
   * @brief Fetch events for the next CALENDAR_LOOKAHEAD_DAYS days.
   *
   * Waits for the SNTP-synced clock to be available, builds the
   * Calendar API "list events" URL, performs the HTTPS GET, and parses the
   * JSON response into `data`.
   *
   * @param data calendarData to clear and fill with the fetched events.
   * @return true on success, false on a time-sync, network, or parse error.
   */
  bool fetchCalendar(calendarData *data);

private:
  char calendarID[128];
  char apiKey[80];
};
