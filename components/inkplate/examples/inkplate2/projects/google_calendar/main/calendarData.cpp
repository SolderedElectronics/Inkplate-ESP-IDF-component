/**
 * @file        calendarData.cpp
 * @brief       Fixed-capacity storage for events fetched from Google Calendar.
 */

#include "calendarData.h"

#include <cstdio>

calendarData::calendarData() : eventCount(0) {}

void calendarData::clearEvents() { eventCount = 0; }

void calendarData::addEvent(const char *summary, const char *start,
                            const char *end) {
  if (eventCount < MAX_EVENTS) {
    snprintf(events[eventCount].summary, EVENT_SUMMARY_MAX_LEN, "%s", summary);
    snprintf(events[eventCount].startTime, EVENT_DATETIME_MAX_LEN, "%s",
             start);
    snprintf(events[eventCount].endTime, EVENT_DATETIME_MAX_LEN, "%s", end);
    eventCount++;
  }
}

Event *calendarData::getEvents() { return events; }

int calendarData::getEventCount() { return eventCount; }
