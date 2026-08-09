/**
 * @file        Network.h
 * @author      Fran Fodor for Soldered
 * @brief       Fetches upcoming events from the Google Calendar public API.
 *
 * @details     Ported from the original Arduino NetworkFunctions class.
 *              WiFi connection handling is left to app_main() (via
 *              display.wifi), matching the rest of this component's
 *              examples; this class is only responsible for the HTTP GET +
 *              JSON parsing step that the original performed with
 *              HTTPClient + ArduinoJson. Arduino String parameters are
 *              replaced with const char* / fixed buffers, and ArduinoJson is
 *              replaced with cJSON (see Network.cpp).
 *
 *              The original sketch authenticates with a plain Google Cloud
 *              API key against a *public* calendar (Calendar ID + "key="
 *              query parameter) rather than OAuth2 - that is exactly what
 *              this port keeps.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 */

#pragma once

class calendarData;

/**
 * @brief Fetches events from a public Google Calendar via the Calendar API
 * v3 "events.list" endpoint, authenticated with an API key.
 */
class NetworkFunctions {
public:
  /**
   * @brief Construct with the target calendar ID and Google Cloud API key.
   *
   * @param calendarID Public Google Calendar ID, e.g.
   *        "abc123@group.calendar.google.com".
   * @param apiKey Google Cloud API key with the Calendar API enabled.
   */
  NetworkFunctions(const char *calendarID, const char *apiKey);

  /**
   * @brief Fetch events for the next 14 days and store them in `data`.
   *
   * Builds the events.list request URL (singleEvents=true,
   * orderBy=startTime, timeMin/timeMax spanning "today" through 14 days
   * from now, maxResults=14), performs the HTTPS GET with esp_http_client,
   * and parses the JSON response with cJSON - same query and limits as the
   * original sketch.
   *
   * @param data Destination calendarData object; existing events are
   *        cleared on success.
   * @param timezoneOffsetHours UTC offset (in hours) used to compute the
   *        "today" window sent to the API, matching the original sketch's
   *        `timeZone` variable. Requires that the system clock has already
   *        been synced (e.g. via display.wifi.setCurrentTime() in
   *        app_main()) before this is called.
   * @return true on success, false on a network/HTTP/JSON error.
   */
  bool fetchCalendar(calendarData *data, int timezoneOffsetHours);

private:
  char calendarID[128];
  char apiKey[128];
};
