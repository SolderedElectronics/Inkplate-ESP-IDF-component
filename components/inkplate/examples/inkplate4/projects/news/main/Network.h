/**
 * Network.h
 * Inkplate ESP-IDF component - Inkplate 4TEMPERA news example
 *
 * Ported from the Inkplate-Arduino-library Inkplate4TEMPERA_News example's
 * src/Network.h. The original used HTTPClient + ArduinoJson; this version
 * uses esp_http_client (with the ESP-IDF certificate bundle for TLS
 * verification) together with cJSON (bundled with ESP-IDF).
 *
 * Original Arduino version:
 * Matej Andracic @ Soldered
 * https://github.com/SolderedElectronics/Inkplate-Arduino-library/tree/master/examples/Inkplate4TEMPERA
 * For more info about the product, please check: https://docs.soldered.com/inkplate/
 * This code is released under the GNU Lesser General Public License v3.0.
 */

#pragma once

#include "Inkplate.h"

/**
 * @brief Single news headline as returned by NewsAPI.org.
 *
 * @note title/description are heap-allocated (via strdup()) and freed by the
 * destructor, mirroring the malloc()/free() pattern used in the original
 * Arduino struct.
 */
struct news
{
    char *title;       // Dynamically allocated headline text.
    char *description; // Dynamically allocated short summary.

    news() : title(nullptr), description(nullptr)
    {
    }

    ~news()
    {
        free(title);
        free(description);
    }
};

/**
 * @brief Fetches and parses top headlines from NewsAPI.org.
 *
 * @note Unlike the original Arduino version, WiFi connection itself is not
 *       handled here (see main.cpp, which uses display.wifi directly) -
 *       this class only performs the HTTPS request + JSON parsing. The
 *       Inkplate reference passed into getData() is only used for
 *       connectivity (display.wifi) and to show an on-screen message if the
 *       API reports no results, matching the original's use of the Inkplate
 *       object for the same purpose.
 */
class NetworkFunctions
{
  public:
    /**
     * @brief Store the NewsAPI.org API key used to build the request URL.
     *
     * @param apiKey NewsAPI.org API key (see https://newsapi.org/).
     */
    void setApiKey(const char *apiKey);

    /**
     * @brief Download and parse the NewsAPI.org "top-headlines" endpoint.
     *
     * @param display Reference to the Inkplate display, used for
     *                 display.wifi (HTTPS download) and to render an
     *                 on-screen error if the API returns no news.
     * @param outCount Out: number of items in the returned array (0 on
     *                  failure).
     * @return news* heap-allocated array of `outCount` items (caller must
     *         `delete[]` it), or nullptr on failure.
     */
    news *getData(Inkplate &display, int *outCount);

  private:
    char apiKeyNews[64] = {0};
};
