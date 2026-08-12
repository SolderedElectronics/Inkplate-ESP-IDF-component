/**
 * @file        calendarData.h
 * @author      Fran Fodor for Soldered
 * @brief       Fixed-capacity storage for events fetched from Google Calendar.
 *
 * Ported from the Inkplate10_Google_Calendar Arduino example. The original
 * stored Arduino String fields inside a fixed-size Event array; this port
 * keeps the same shape but uses fixed char[] buffers instead of String to
 * avoid heap fragmentation on the ESP32.
 */
#pragma once

#include <cstddef>

// Maximum number of events stored/displayed at once (matches the original
// sketch's MAX_EVENTS and the Calendar API's maxResults request parameter).
constexpr int MAX_EVENTS = 16;

// Field size limits for one event. startTime/endTime hold RFC3339 timestamps
// such as "2026-08-05T10:00:00+02:00" (or the plain "2026-08-05" form used
// for all-day events).
constexpr size_t EVENT_SUMMARY_LEN = 96;
constexpr size_t EVENT_TIME_LEN = 32;

/**
 * @brief One calendar event: title plus ISO 8601/RFC3339 start/end timestamps.
 */
struct Event {
  char summary[EVENT_SUMMARY_LEN];
  char startTime[EVENT_TIME_LEN];
  char endTime[EVENT_TIME_LEN];
};

/**
 * @brief Fixed-capacity list of events fetched from the Google Calendar API.
 */
class calendarData {
public:
  calendarData();

  /// Reset the stored event list to empty.
  void clearEvents();

  /**
   * @brief Append one event, truncating fields that don't fit.
   *
   * Silently ignored once MAX_EVENTS entries are stored, matching the
   * original sketch's behavior.
   */
  void addEvent(const char *summary, const char *start, const char *end);

  /// @return pointer to the internal event array (getEventCount() entries valid).
  Event *getEvents();

  /// @return number of events currently stored.
  int getEventCount();

private:
  Event events[MAX_EVENTS];
  int eventCount;
};
