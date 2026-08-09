/**
 * @file        NetworkFunctions.h
 * @author      Fran Fodor for Soldered
 * @brief       Helper class for computing local time in different timezones.
 *
 * @details     The original Arduino sketch queried a remote REST service
 *              (timeapi.io) over HTTPS to resolve a city name to a full IANA
 *              timezone identifier and to fetch the current local time for
 *              that identifier. ESP-IDF's newlib toolchain does not ship the
 *              IANA timezone database, and pulling in an HTTPS client + JSON
 *              parser just to reproduce that lookup would add a network
 *              dependency with no benefit here. This port keeps the same
 *              class name/shape but computes each city's local time
 *              directly from the SNTP-synced UTC epoch using a fixed
 *              UTC offset per city (the same approach already used by the
 *              rtc/alarm example in this component).
 *
 * https://github.com/SolderedElectronics/Inkplate-Esp-library
 * For more info about the product, please check:
 * https://docs.soldered.com/inkplate/
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

/**
 * @brief Helper class for all the timezone/clock computations.
 *
 */
class NetworkFunctions {
public:
  /**
   * @brief Reserved for future setup. Kept to mirror the original Arduino
   *        API (`network.begin(ssid, pass)`) and to keep call sites in
   *        main.cpp unchanged if this class is later extended to fetch
   *        time from a network service again.
   */
  void begin();

  /**
   * @brief Compute the local hour/minute for a city given a fixed UTC
   *        offset.
   *
   * @param utcOffsetMinutes Offset from UTC, in minutes (can be negative,
   *                          and supports half-hour/quarter-hour zones).
   * @param hours   Out: local hour (0-23).
   * @param minutes Out: local minute (0-59).
   * @return bool true if computed successfully.
   */
  bool getData(int utcOffsetMinutes, int *hours, int *minutes);
};
