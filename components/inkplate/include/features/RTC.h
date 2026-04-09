#ifndef _RTC_H_
#define _RTC_H_

#include "time.h"
#include "esp_check.h"
#include "driver/i2c_master.h"

#define RTC_I2C_ADDR       0x51

// control registers
#define RTC_CTRL_1         0x00
#define RTC_CTRL_2         0x01
#define RTC_OFFSET         0x02
#define RTC_RAM            0x03

#define RTC_12_24          0x02

#define RTC_CTRL_2_DEFAULT 0x00
#define RTC_CTRL_1_DEFAULT 0x58

#define RTC_SET            0xAA
#define RTC_NOT_SET        0x00

// time and date registers
#define RTC_SECOND_ADDR    0x04
#define RTC_MINUTE_ADDR    0x05
#define RTC_HOUR_ADDR      0x06
#define RTC_DAY_ADDR       0x07
#define RTC_WDAY_ADDR      0x08
#define RTC_MONTH_ADDR     0x09
#define RTC_YEAR_ADDR      0x0A

// alarm registers
#define RTC_SECOND_ALARM   0x0B
#define RTC_MINUTE_ALARM   0x0C
#define RTC_HOUR_ALARM     0x0D
#define RTC_DAY_ALARM      0x0E
#define RTC_WDAY_ALARM     0x0F

// alarm control
#define RTC_ALARM_AIE      0x80
#define RTC_ALARM_AF       0x40

// timer registers
#define RTC_TIMER_VALUE    0x10
#define RTC_TIMER_MODE     0x11

// timer control
#define RTC_TIMER_TI_TP    0x01
#define RTC_TIMER_TIE      0x02
#define RTC_TIMER_TE       0x04
#define RTC_TIMER_TF       0x08

typedef enum
{
  RTC_TIMER_CLOCK_4096HZ   = 0,
  RTC_TIMER_CLOCK_64HZ     = 1,
  RTC_TIMER_CLOCK_1HZ      = 2,
  RTC_TIMER_CLOCK_1PER60HZ = 3,
} rtcCountdownSrcClock_t;

typedef enum
{
  RTC_FORMAT_24H = 0,
  RTC_FORMAT_12H = 1,
} rtcHourFormat_t;

class RTC
{
public:
  RTC() = default;
  esp_err_t begin(i2c_master_bus_handle_t bus_handle);

  // time setting
  esp_err_t setTime(tm  time);
  esp_err_t getTime(tm *time);
  esp_err_t setTime(time_t  epoch);
  esp_err_t getTime(time_t *epoch);

  esp_err_t changeTimeFormat(rtcHourFormat_t format);

  // alarm setting
  esp_err_t setAlarm(uint8_t second);
  esp_err_t setAlarm(uint8_t second, uint8_t minute);
  esp_err_t setAlarm(uint8_t second, uint8_t minute, uint8_t hour);
  esp_err_t setAlarm(uint8_t second, uint8_t minute, uint8_t hour, uint8_t day);
  esp_err_t setAlarm(uint8_t second, uint8_t minute, uint8_t hour, uint8_t day, uint8_t weekday);
  esp_err_t setAlarmEpoch(time_t epoch);
  esp_err_t getAlarm(tm *time);
  esp_err_t clearAlarmFlag();
  bool      checkAlarmFlag();

  // timer setting
  esp_err_t setTimer(rtcCountdownSrcClock_t clockSource, uint8_t value, bool intEnable, bool intPulse);
  esp_err_t disableTimer();
  esp_err_t clearTimerFlag();
  bool      checkTimerFlag();

  // other
  esp_err_t reset();
  esp_err_t setInternalCapacitor(bool value);
  esp_err_t setClockOffset(bool mode, int8_t offsetValue);
  bool      isSet();

  // individual getters
  uint8_t   getSecond();
  uint8_t   getMinute();
  uint8_t   getHour();
  uint8_t   getDay();
  uint8_t   getWeekday();
  uint8_t   getMonth();
  uint16_t  getYear();

  uint8_t   getAlarmSecond();
  uint8_t   getAlarmMinute();
  uint8_t   getAlarmHour();
  uint8_t   getAlarmDay();
  uint8_t   getAlarmWeekday();

private:
  esp_err_t updateTime();
  esp_err_t updateAlarm();
  void      enableAlarm();
  uint8_t   encodeHour(uint8_t hour);

  uint8_t   decToBcd(uint8_t value);
  uint8_t   bcdToDec(uint8_t value);

  tm                      m_time;
  tm                      m_alarmTime;
  rtcHourFormat_t         m_hourFormat;

  i2c_master_dev_handle_t m_devHandle;
};

#endif
