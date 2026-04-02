#include "time.h"
#include "esp_log.h"
#include "esp_check.h"

#include "rtc.h"

static const char* TAG = "ESP_RTC";

/**
 * ============================================================
 * Public functions
 * ============================================================
 */

/**
 * @brief  RTC constructor.
 *
 * @note   Sets I2C port properties.
 */
RTC::RTC(i2c_master_bus_handle_t busHandle)
{
  i2c_device_config_t dev_config = {};
  dev_config.dev_addr_length = I2C_ADDR_BIT_LEN_7;
  dev_config.device_address = RTC_I2C_ADDR;
  dev_config.scl_speed_hz = 400000;

  ESP_ERROR_CHECK(i2c_master_bus_add_device(busHandle, &dev_config, &m_devHandle));

  uint8_t data[2] = { RTC_RAM, RTC_NOT_SET };
  ESP_ERROR_CHECK(i2c_master_transmit(m_devHandle, data, sizeof(data), -1));

  m_hourFormat = RTC_FORMAT_24H;

  ESP_LOGI(TAG, "I2C initilization finished!"); 
}

/**
 * @brief  Sets the time.
 *
 * @param  tm time
 *         struct containing time information to set
 *
 * @return esp_err_t
 *         ESP_OK if no error occured
 *
 * @note   PCF85063A datasheet pg. 23; read or write access should be performed in one go. If the full struct is not
 * provided the values will be undefinined (values of struct members that were not provided).
 */
esp_err_t RTC::setTime(tm time)
{
  esp_err_t ret = ESP_OK;

  uint8_t data[8] =
  {
     RTC_SECOND_ADDR,
     decToBcd(time.tm_sec),
     decToBcd(time.tm_min),
     encodeHour(time.tm_hour),
     decToBcd(time.tm_mday),
     decToBcd(time.tm_wday),
     decToBcd(time.tm_mon - 1),
     decToBcd(time.tm_year - 1900)
  };

  ret = i2c_master_transmit(m_devHandle, data, sizeof(data), -1);
  if (ret != ESP_OK)
    return ret;

  uint8_t data_[2] = { RTC_RAM, RTC_SET };
  ret = i2c_master_transmit(m_devHandle, data_, sizeof(data_), -1);

  return ret;
}

/**
 * @brief  Get the time set.
 *
 * @param  tm *time
 *         pointer to struct to which time information will be written
 *
 * @return esp_err_t
 *         ESP_OK if no error occured
 *
 * @note   PCF85063A datasheet pg. 26; read or write access should be performed in one go.
 */
esp_err_t RTC::getTime(tm *time)
{
  esp_err_t ret = updateTime();
  if (ret != ESP_OK)
    return ret;

  memcpy(time, &m_time, sizeof(tm));

  return ret;
}

/**
 * @brief  Sets the time using epoch.
 *
 * @param  time_t epoch
 *         time in epoch
 *
 * @return esp_err_t
 *         ESP_OK if no error occured
 *
 * @note   PCF85063A datasheet pg. 23; read or write access should be performed in one go. If the full struct is not
 * provided the values will be undefinined (values of struct members that were not provided).
 */
esp_err_t RTC::setTime(time_t epoch)
{
  tm time;
  gmtime_r(&epoch, &time);
  time.tm_year += 1900;

  return setTime(time);
}

/**
 * @brief  Get the time set in epoch.
 *
 * @param  time_t epoch
 *         pointer to epoch to which time information will be written
 *
 * @return esp_err_t
 *         ESP_OK if no error occured
 *
 * @note   PCF85063A datasheet pg. 26; read or write access should be performed in one go.
 */
esp_err_t RTC::getTime(time_t *epoch)
{
  esp_err_t ret = updateTime();
  if (ret != ESP_OK)
    return ret;
  
  m_time.tm_year -= 1900;
  *epoch = mktime(&m_time);

  return ret;
}

/**
 * @brief  Changes the RTC time format.
 *
 * @param  rtcHourFormat_t format
 *         hour format to set
 *
 * @return esp_err_t
 *         ESP_OK if no error occured
 */
esp_err_t RTC::changeTimeFormat(rtcHourFormat_t format)
{
  esp_err_t ret = ESP_OK;

  uint8_t reg = RTC_CTRL_1;
  uint8_t ctrl1 = 0;

  ret = i2c_master_transmit_receive(m_devHandle, &reg, 1, &ctrl1, 1, -1);
  if (ret != ESP_OK)
    return ret;

  if (format == RTC_FORMAT_12H)
    ctrl1 |=  RTC_12_24;
  else
    ctrl1 &= ~RTC_12_24;

  uint8_t data[2] =
  {
    RTC_CTRL_1,
    ctrl1
  };

  ret = i2c_master_transmit(m_devHandle, data, sizeof(data), -1);
  if (ret != ESP_OK)
    return ret;

  m_hourFormat = format;

  return ret;
}

/**
 * @brief  Sets the alarm in seconds.
 *
 * @param  uint8_t second
 *         at what second to trigger the alarm
 *
 * @return esp_err_t
 *         ESP_OK if no error occured
 */
esp_err_t RTC::setAlarm(uint8_t second)
{
  esp_err_t ret = ESP_OK;
  enableAlarm();

  uint8_t data[2] =
  {
    RTC_SECOND_ALARM,
    (uint8_t)(decToBcd(second) & ~RTC_ALARM_AIE),
  };

  ret = i2c_master_transmit(m_devHandle, data, sizeof(data), -1);
  return ret;
}

/**
 * @brief  Sets the alarm in seconds and minutes.
 *
 * @param  uint8_t second
 *         at what second to trigger the alarm
 *
 * @param  uint8_t minute
 *         at what minute to trigger the alarm
 *
 * @return esp_err_t
 *         ESP_OK if no error occured
 */
esp_err_t RTC::setAlarm(uint8_t second, uint8_t minute)
{
  esp_err_t ret = ESP_OK;
  enableAlarm();

  uint8_t data[3] =
  {
    RTC_SECOND_ALARM,
    (uint8_t)(decToBcd(second) & ~RTC_ALARM_AIE),
    (uint8_t)(decToBcd(minute) & ~RTC_ALARM_AIE),
  };

  ret = i2c_master_transmit(m_devHandle, data, sizeof(data), -1);
  return ret;
}

/**
 * @brief  Sets the alarm in seconds, minutes and hours.
 *
 * @param  uint8_t second
 *         at what second to trigger the alarm
 *
 * @param  uint8_t minute
 *         at what minute to trigger the alarm
 *
 * @param  uint8_t hour
 *         at what hour to trigger the alarm
 *
 * @return esp_err_t
 *         ESP_OK if no error occured
 */
esp_err_t RTC::setAlarm(uint8_t second, uint8_t minute, uint8_t hour)
{
  esp_err_t ret = ESP_OK;
  enableAlarm();

  uint8_t data[4] =
  {
    RTC_SECOND_ALARM,
    (uint8_t)(decToBcd(second) & ~RTC_ALARM_AIE),
    (uint8_t)(decToBcd(minute) & ~RTC_ALARM_AIE),
    (uint8_t)(encodeHour(hour)   & ~RTC_ALARM_AIE),
  };

  ret = i2c_master_transmit(m_devHandle, data, sizeof(data), -1);
  return ret;
}

/**
 * @brief  Sets the alarm in seconds, minutes, hours and day.
 *
 * @param  uint8_t second
 *         at what second to trigger the alarm
 *
 * @param  uint8_t minute
 *         at what minute to trigger the alarm
 *
 * @param  uint8_t hour
 *         at what hour to trigger the alarm
 *
 * @param  uint8_t mday
 *         at what day of month to trigger the alarm
 *
 * @return esp_err_t
 *         ESP_OK if no error occured
 */
esp_err_t RTC::setAlarm(uint8_t second, uint8_t minute, uint8_t hour, uint8_t mday)
{
  esp_err_t ret = ESP_OK;
  enableAlarm();

  uint8_t data[5] =
  {
    RTC_SECOND_ALARM,
    (uint8_t)(decToBcd(second) & ~RTC_ALARM_AIE),
    (uint8_t)(decToBcd(minute) & ~RTC_ALARM_AIE),
    (uint8_t)(encodeHour(hour)   & ~RTC_ALARM_AIE),
    (uint8_t)(decToBcd(mday)   & ~RTC_ALARM_AIE),
  };

  ret = i2c_master_transmit(m_devHandle, data, sizeof(data), -1);
  return ret;
}

/**
 * @brief  Sets the alarm in seconds, minutes, hours and day.
 *
 * @param  uint8_t second
 *         at what second to trigger the alarm
 *
 * @param  uint8_t minute
 *         at what minute to trigger the alarm
 *
 * @param  uint8_t hour
 *         at what hour to trigger the alarm
 *
 * @param  uint8_t mday
 *         at what day of month to trigger the alarm
 *
 * @param  uint8_t wday
 *         at what day of week to trigger the alarm
 *
 * @return esp_err_t
 *         ESP_OK if no error occured
 */
esp_err_t RTC::setAlarm(uint8_t second, uint8_t minute, uint8_t hour, uint8_t mday, uint8_t wday)
{
  esp_err_t ret = ESP_OK;
  enableAlarm();

  uint8_t data[6] =
  {
    RTC_SECOND_ALARM,
    (uint8_t)(decToBcd(second) & ~RTC_ALARM_AIE),
    (uint8_t)(decToBcd(minute) & ~RTC_ALARM_AIE),
    (uint8_t)(encodeHour(hour) & ~RTC_ALARM_AIE),
    (uint8_t)(decToBcd(mday)   & ~RTC_ALARM_AIE),
    (uint8_t)(decToBcd(wday)   & ~RTC_ALARM_AIE),
  };

  ret = i2c_master_transmit(m_devHandle, data, sizeof(data), -1);
  return ret;
}

/**
 * @brief  Sets the alarm epoch.
 *
 * @param  time_t epoch
 *         when to trigger the alarm in epoch
 *
 * @return esp_err_t
 *         ESP_OK if no error occured
 */
esp_err_t RTC::setAlarmEpoch(time_t epoch)
{
  tm time;
  gmtime_r(&epoch, &time);

  return setAlarm(time.tm_sec, time.tm_min, time.tm_hour, time.tm_mday, time.tm_wday);
}

/**
 * @brief  Gets the alarm values.
 *
 * @param  tm *time
 *         pointer to the struct in which the value will be stored
 *
 * @return esp_err_t
 *         ESP_OK if no error occured
 *
 * @note   Trying to display month or year will result in defined behavour as alarm doesn't have month and year
 * registers.
 */
esp_err_t RTC::getAlarm(tm *time)
{
  esp_err_t ret = ESP_OK;

  uint8_t reg = RTC_SECOND_ALARM;
  uint8_t data[5];

  ret = i2c_master_transmit_receive(m_devHandle, &reg, 1, data, sizeof(data), -1);
  if (ret != ESP_OK)
    return ret;

  m_alarmTime.tm_sec  = bcdToDec(data[0]);
  m_alarmTime.tm_min  = bcdToDec(data[1]);
  m_alarmTime.tm_hour = bcdToDec(data[2]);
  m_alarmTime.tm_mday = bcdToDec(data[3]);
  m_alarmTime.tm_wday = bcdToDec(data[4]);

  memcpy(time, &m_alarmTime, sizeof(tm));

  return ret;
}

/**
 * @brief  Reads the value of alarm flag.
 *
 * @return bool
 *         0 if alarm not triggered
 */
bool RTC::checkAlarmFlag()
{
  uint8_t reg = RTC_CTRL_2;
  uint8_t ctrl2 = 0;

  ESP_ERROR_CHECK(i2c_master_transmit_receive(m_devHandle, &reg, 1, &ctrl2, 1, -1));

  return (ctrl2 & RTC_ALARM_AF) != 0;
}

/**
 * @brief  Cleares the alarm interrupt flag.
 *
 * @return esp_err_t
 *         ESP_OK if no error occured
 *
 * @note   Need to call this after every interrupt.
 */
esp_err_t RTC::clearAlarmFlag()
{
  esp_err_t ret = ESP_OK;

  uint8_t reg = RTC_CTRL_2;
  uint8_t ctrl2 = 0;

  ret = i2c_master_transmit_receive(m_devHandle, &reg, 1, &ctrl2, 1, -1);
  if (ret != ESP_OK)
    return ret;

  // clear AF bit and write back
  uint8_t data[2] = {RTC_CTRL_2, (uint8_t)(ctrl2 & ~RTC_ALARM_AF)};
  ret = i2c_master_transmit(m_devHandle, data, sizeof(data), -1);

  return ret;
}

/**
 * @brief  Sets the timer.
 *
 * @param  rtcCountdownSrcClock sourceClock
 *         timer clock frequency
 *
 * @param  uint8_t value
 *         timer value
 *
 * @param  bool intEnable
 *         timer interrupt enable
 *
 * @param  bool intPulse
 *         timer interrupt mode
 *         0 - interrupt follows timer flag
 *         1 - interrpt generates a pulse
 *
 * @return esp_err_t
 *         ESP_OK if no error occured
 */
esp_err_t RTC::setTimer(rtcCountdownSrcClock_t sourceClock, uint8_t value, bool intEnable, bool intPulse)
{
  esp_err_t ret = ESP_OK;

  uint8_t data[2] = { RTC_CTRL_2, 0x00 };

  ret = i2c_master_transmit(m_devHandle, data, sizeof(data), -1);
  if (ret != ESP_OK)
    return ret;

  uint8_t mode = RTC_TIMER_TE;
  if (intEnable)
    mode |= RTC_TIMER_TIE;
  if (intPulse)
    mode |= RTC_TIMER_TI_TP;
  mode |= sourceClock << 3;

  uint8_t data_[3] = { RTC_TIMER_VALUE, value, mode };

  ret = i2c_master_transmit(m_devHandle, data_, sizeof(data_), -1);

  return ret;
}

/**
 * @brief  Disables the timer.
 *
 * @return esp_err_t
 *         ESP_OK if no error occured
 */
esp_err_t RTC::disableTimer()
{
  esp_err_t ret = ESP_OK;
  
  uint8_t reg = RTC_TIMER_MODE;
  uint8_t mode = 0;
  
  ret = i2c_master_transmit_receive(m_devHandle, &reg, 1, &mode, 1, -1);
  if (ret != ESP_OK)
    return ret;

  uint8_t data[2] = { RTC_TIMER_MODE, (uint8_t)(mode & ~RTC_TIMER_TE) };

  ret = i2c_master_transmit(m_devHandle, data, sizeof(data), -1);

  return ret;
}

/**
 * @brief  Reads the value of timer flag.
 *
 * @return bool
 *         0 if timer not triggered
 */
bool RTC::checkTimerFlag()
{
  uint8_t reg = RTC_CTRL_2;
  uint8_t ctrl2 = 0;

  ESP_ERROR_CHECK(i2c_master_transmit_receive(m_devHandle, &reg, 1, &ctrl2, 1, -1));

  return (ctrl2 & RTC_TIMER_TF) != 0;
}

/**
 * @brief  Cleares the timer interrupt flag.
 *
 * @return esp_err_t
 *         ESP_OK if no error occured
 *
 * @note   Need to call this after every interrupt.
 */
esp_err_t RTC::clearTimerFlag()
{
  esp_err_t ret = ESP_OK;

  uint8_t reg = RTC_CTRL_2;
  uint8_t ctrl2 = 0;

  ret = i2c_master_transmit_receive(m_devHandle, &reg, 1, &ctrl2, 1, -1);
  if (ret != ESP_OK)
    return ret;

  // clear TF bit and write back
  uint8_t data[2] = {RTC_CTRL_2, (uint8_t)(ctrl2 & ~RTC_TIMER_TF)};
  ret = i2c_master_transmit(m_devHandle, data, sizeof(data), -1);

  return ret;
}

/**
 * @brief  Reset RTC.
 *
 * @return esp_err_t
 *         ESP_OK if no error occured
 */
esp_err_t RTC::reset()
{
  esp_err_t ret = ESP_OK;

  uint8_t data[2] = { RTC_CTRL_1,RTC_CTRL_1_DEFAULT, };

  ret = i2c_master_transmit(m_devHandle, data, sizeof(data), -1);

  return ret;
}

/**
 * @brief  Set internal capacitor value.
 *
 * @param  bool value
 *         0 or 1 which represents 7pF or 12.5pF
 *
 * @return esp_err_t
 *         ESP_OK if no error occured
 */
esp_err_t RTC::setInternalCapacitor(bool value)
{
  esp_err_t ret;

  uint8_t reg = RTC_CTRL_1;
  uint8_t ctrl1 = 0;

  ret = i2c_master_transmit_receive(m_devHandle, &reg, 1, &ctrl1, 1, -1);
  if (ret != ESP_OK)
    return ret;

  if (value)
    ctrl1 |=  (1 << 0);
  else
    ctrl1 &= ~(1 << 0);

  uint8_t data[2] =  { RTC_CTRL_1, ctrl1 };
  
  ret = i2c_master_transmit(m_devHandle, data, sizeof(data), -1);

  return ret;
}

/**
 * @brief  Offset used to correct frequency of the crystal used for RTC.
 *
 * @param  bool mode
 *         0 - normal mode, offset made once every two hours
 *         1 - couse mode, offset made every four minutes
 *
 * @param  int8_t offsetValue
 *         coded in two's complement
 *
 * @return esp_err_t
 *         ESP_OK if no error occured
 */
esp_err_t RTC::setClockOffset(bool mode, int8_t offsetValue)
{
  if (offsetValue > 63 || offsetValue < -64)
    return ESP_ERR_INVALID_ARG;

  if (offsetValue < 0)
    offsetValue += 128;

  uint8_t regValue = (uint8_t)offsetValue;
  if (mode)
    regValue |= (1 << 7);
  else
    regValue &= ~(1 << 7);

  uint8_t data[2] = { RTC_OFFSET, regValue };
  return i2c_master_transmit(m_devHandle, data, sizeof(data), -1);
}

/**
 * @brief  Check if RTC is set.
 *
 * @return True if set.
 */
bool RTC::isSet()
{
  uint8_t reg = RTC_RAM;
  uint8_t ramByte = 0;
  
  ESP_ERROR_CHECK(i2c_master_transmit_receive(m_devHandle, &reg, 1, &ramByte, 1, -1));
  
  return ramByte == RTC_SET;
}

/**
 * @brief  Updates RTC and reads value.
 *
 * @return Current second.
 */
uint8_t RTC::getSecond()
{
  updateTime();
  return m_time.tm_sec;
}

/**
 * @brief  Updates RTC and reads value.
 *
 * @return Current minute.
 */
uint8_t RTC::getMinute()
{
  updateTime();
  return m_time.tm_min;
}

/**
 * @brief  Updates RTC and reads value.
 *
 * @return Current hour.
 */
uint8_t RTC::getHour()
{
  updateTime();
  return m_time.tm_hour;
}

/**
 * @brief  Updates RTC and reads value.
 *
 * @return Current day.
 */
uint8_t RTC::getDay()
{
  updateTime();
  return m_time.tm_mday;
}

/**
 * @brief  Updates RTC and reads value.
 *
 * @return Current weekday.
 */
uint8_t RTC::getWeekday()
{
  updateTime();
  return m_time.tm_wday;
}

/**
 * @brief  Updates RTC and reads value.
 *
 * @return Current month.
 */
uint8_t RTC::getMonth()
{
  updateTime();
  return m_time.tm_mon;
}

/**
 * @brief  Updates RTC and reads value.
 *
 * @return Current year.
 */
uint16_t RTC::getYear()
{
  updateTime();
  return m_time.tm_year;
}

/**
 * @brief  Reads alarm value.
 *
 * @return Alarm second.
 */
uint8_t RTC::getAlarmSecond()
{
  updateAlarm();
  return m_time.tm_sec;
}

/**
 * @brief  Reads alarm value.
 *
 * @return Alarm minute.
 */
uint8_t RTC::getAlarmMinute()
{
  updateAlarm();
  return m_time.tm_min;
}
/**
 * @brief  Reads alarm value.
 *
 * @return Alarm hour.
 */
uint8_t RTC::getAlarmHour()
{
  updateAlarm();
  return m_time.tm_hour;
}
/**
 * @brief  Reads alarm value.
 *
 * @return Alarm day.
 */
uint8_t RTC::getAlarmDay()
{
  updateAlarm();
  return m_time.tm_mday;
}
/**
 * @brief  Reads alarm value.
 *
 * @return Alarm weekday.
 */
uint8_t RTC::getAlarmWeekday()
{
  updateAlarm();
  return m_time.tm_wday;
}
/**
 * ============================================================
 * Private functions
 * ============================================================
 */

/**
 * @brief  Internal function to update values in the class.
 *
 * @return esp_err_t
 *         ESP_OK if no error occured
 *
 * @note   PCF85063A datasheet pg. 26; read or write access should be performed in one go.
 */
esp_err_t RTC::updateTime()
{
  esp_err_t ret = ESP_OK;

  uint8_t reg = RTC_SECOND_ADDR;
  uint8_t data[7] = {0};

  ret = i2c_master_transmit(m_devHandle, &reg, 1, -1);
  if (ret != ESP_OK)
    return ret;
  ret = i2c_master_receive(m_devHandle, data, sizeof(data), -1);
  if (ret != ESP_OK)
    return ret;

  // check datasheet to see unused bits in registers
  m_time.tm_sec  = bcdToDec(data[0] & 0x7F);
  m_time.tm_min  = bcdToDec(data[1] & 0x7F);
  // m_time.tm_hour = bcdToDec(data[2] & 0x3F);
  m_time.tm_mday = bcdToDec(data[3] & 0x3F);
  m_time.tm_wday = bcdToDec(data[4] & 0x07);
  m_time.tm_mon  = bcdToDec(data[5] & 0x1F) + 1;
  m_time.tm_year = bcdToDec(data[6]) + 1900;

  if (m_hourFormat == RTC_FORMAT_12H)
  {
    bool pm = (data[2] & 0x20) != 0;
    uint8_t hour = bcdToDec(data[2] & 0x1F);
    if (pm && hour != 12) hour += 12;
    if (!pm && hour == 12) hour = 0;
    m_time.tm_hour = hour;
  }
  else
  {
    m_time.tm_hour = bcdToDec(data[2] & 0x3F);
  }

  return ret;
}

/**
 * @brief  Internal function to update values in the class.
 *
 * @return esp_err_t
 *         ESP_OK if no error occured
 */
esp_err_t RTC::updateAlarm()
{
  esp_err_t ret = ESP_OK;
  uint8_t reg = RTC_SECOND_ALARM;
  uint8_t data[5] = {0};
  ret = i2c_master_transmit_receive(m_devHandle, &reg, 1, data, sizeof(data), -1);
  if (ret != ESP_OK)
    return ret;

  m_alarmTime.tm_sec  = bcdToDec(data[0] & 0x7F);
  m_alarmTime.tm_min  = bcdToDec(data[1] & 0x7F);
  m_alarmTime.tm_mday = bcdToDec(data[3] & 0x3F);
  m_alarmTime.tm_wday = bcdToDec(data[4] & 0x07);

  if (m_hourFormat == RTC_FORMAT_12H)
  {
    bool pm = (data[2] & 0x20) != 0;
    uint8_t hour = bcdToDec(data[2] & 0x1F);
    if (pm && hour != 12) hour += 12;
    if (!pm && hour == 12) hour = 0;
    m_alarmTime.tm_hour = hour;
  }
  else
  {
    m_alarmTime.tm_hour = bcdToDec(data[2] & 0x3F);
  }

  return ret;
}

/**
 * @brief  Internal function to enable alarm.
 *
 * @note   PCF85063A datasheet pg. 11.
 */
void RTC::enableAlarm()
{
  uint8_t registerMask;
  registerMask = (RTC_CTRL_2_DEFAULT | RTC_ALARM_AIE) & ~RTC_ALARM_AF;

  uint8_t data[2] = { RTC_CTRL_2, registerMask };

  ESP_ERROR_CHECK(i2c_master_transmit(m_devHandle, data, 2, -1));
}

/**
 * @brief  Handles hour conversion for different time formats.
 *
 * @param  uint8_t hour
 *         hour to convert
 *
 * @return uint8_t
 *         hour in Bcd
 */
uint8_t RTC::encodeHour(uint8_t hour)
{
  if (m_hourFormat == RTC_FORMAT_12H)
  {
    bool pm = hour >= 12;
    uint8_t hour12 = hour % 12;
    if (hour12 == 0) hour12 = 12;
    return decToBcd(hour12) | (pm ? 0x20 : 0x00);
  }

  return decToBcd(hour);
}

/**
 * @brief  Converts decimal to BCD.
 *
 * @param  uint8_t val
 *         number which needs to be converted from decimal to Bcd value
 *
 * @return uint8_t
 *         value in Bcd
 */
uint8_t RTC::decToBcd(uint8_t val)
{
  return ((val / 10 * 16) + (val % 10));
}

/**
 * @brief  Converts BCD to decimal.
 *
 * @param  uint8_t val
 *         number which needs to be converted from Bcd to decimal value
 * @return uint8_t
 *         value in Dec
 */
uint8_t RTC::bcdToDec(uint8_t val)
{
  return ((val / 16 * 10) + (val % 16));
}
