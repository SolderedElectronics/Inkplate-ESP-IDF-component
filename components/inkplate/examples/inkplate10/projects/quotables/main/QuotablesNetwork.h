/**
 * @file        QuotablesNetwork.h
 * @author      Fran Fodor for Soldered
 * @brief       Helper for fetching a random quote from the Quotable public API.
 *
 * @details     Ported from the Inkplate10_Quotables Arduino example. The
 *              original used ArduinoJson + HTTPClient inside a
 *              NetworkFunctions class; here the same responsibility (GET the
 *              quote endpoint, parse the JSON body, and hand back plain
 *              C strings) is implemented with esp_http_client + cJSON.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 */

#pragma once

#include <cstddef>

/**
 * @brief Fetches and parses quotes from the Quotable public API.
 */
class NetworkFunctions {
public:
  /**
   * @brief Fetch a random quote and its author.
   *
   * Performs an HTTP GET against the Quotable API, parses the JSON
   * response with cJSON, and copies the quote text and author name into
   * the caller-supplied buffers (truncated to fit if necessary).
   *
   * @param text buffer that receives the null-terminated quote text.
   * @param textSize size of the `text` buffer, in bytes.
   * @param author buffer that receives the null-terminated author name.
   * @param authorSize size of the `author` buffer, in bytes.
   * @return true if a quote was fetched and parsed successfully, false on
   *         a network error or unexpected/invalid JSON response.
   */
  bool getData(char *text, size_t textSize, char *author, size_t authorSize);
};
