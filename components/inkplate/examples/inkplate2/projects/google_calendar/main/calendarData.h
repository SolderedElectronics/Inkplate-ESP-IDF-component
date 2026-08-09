/**
 * @file        calendarData.h
 * @author      Fran Fodor for Soldered
 * @brief       Fixed-capacity storage for events fetched from Google Calendar.
 *
 * @details     Ported from the original Arduino calendarData class. The
 *              Arduino String members of `Event` are replaced with
 *              fixed-size char buffers (filled via snprintf() in
 *              addEvent()) to avoid heap fragmentation from repeated String
 *              concatenation/copying.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 */

#pragma once

// Maximum number of upcoming events retained at once. Matches the original
// MAX_EVENTS and the "maxResults=14" query parameter used by
// NetworkFunctions::fetchCalendar().
#define MAX_EVENTS 16

// Buffer sizes for the fixed-size C strings held inside Event. Google
// Calendar event titles can be long, so summary gets a generous buffer (it
// is truncated for display purposes in Gui::showCalendar() anyway).
// ISO-8601 datetime strings ("YYYY-MM-DDTHH:MM:SS+HH:MM") fit comfortably in
// 32 bytes.
#define EVENT_SUMMARY_MAX_LEN 128
#define EVENT_DATETIME_MAX_LEN 32

/**
 * @brief One calendar event: title + ISO-8601 start/end timestamps, as
 * returned by the Google Calendar API ("dateTime" for timed events, "date"
 * for all-day events).
 */
struct Event {
  char summary[EVENT_SUMMARY_MAX_LEN];
  char startTime[EVENT_DATETIME_MAX_LEN];
  char endTime[EVENT_DATETIME_MAX_LEN];
};

/**
 * @brief Fixed-capacity store for the events fetched from Google Calendar.
 */
class calendarData {
public:
  calendarData();

  /// Reset the store to empty (called before re-fetching).
  void clearEvents();

  /// Append an event if there is still room (silently dropped otherwise,
  /// same behavior as the original).
  void addEvent(const char *summary, const char *start, const char *end);

  /// Pointer to the internal event array (valid until the next
  /// clearEvents()/addEvent() call).
  Event *getEvents();

  /// Number of events currently stored.
  int getEventCount();

private:
  Event events[MAX_EVENTS];
  int eventCount;
};
