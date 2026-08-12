/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Spotify "now playing" album screen for Soldered
 *              Inkplate 13SPECTRA.
 *
 * @details     Connects Inkplate 13SPECTRA to WiFi, refreshes a Spotify
 *              OAuth2 access token via the refresh_token grant (HTTP Basic
 *              auth, base64("clientId:clientSecret") built with
 *              mbedtls_base64_encode), then polls the Spotify Web API's
 *              "currently playing" endpoint (Bearer access token) and
 *              renders the track/artist/album, a playback progress bar, and
 *              the album cover art on the 6-color e-paper panel.
 *
 *              Networking (token refresh POST + currently-playing GET, both
 *              via esp_http_client + cJSON) lives in
 *              NetworkFunctions.cpp/.h; drawing lives in Gui.cpp/.h,
 *              mirroring the original sketch's src/ layout.
 *
 *              Between polls the board deep sleeps for POLL_SECONDS
 *              (includes.h) and restarts from app_main() on wake, matching
 *              the original sketch's refresh mechanism. The last-drawn
 *              album ID and "nothing playing" state survive deep sleep in
 *              RTC memory, so the e-paper is only redrawn when what's
 *              playing actually changes (e-paper refreshes are slow and
 *              visible, so this avoids needless flicker).
 *
 * Requirements:
 * - Board:      Soldered Inkplate 13SPECTRA
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 13SPECTRA, USB cable
 * - Extra:      WiFi (2.4 GHz) + Internet access, a Spotify Developer app
 *               (Client ID + Client Secret), and a Spotify refresh token -
 *               see README.md for how to obtain the refresh token, which
 *               requires completing Spotify's OAuth2 Authorization Code
 *               flow once, outside the device, in a browser.
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate13
 * - Menuconfig -> WiFi Configuration -> Enter your SSID and password
 * - SPOTIFY_CLIENT_ID / SPOTIFY_CLIENT_SECRET / SPOTIFY_REFRESH_TOKEN below
 *   -> fill in your own values (see README.md)
 *
 * How to use:
 * 1) Create a Spotify Developer app at
 *    https://developer.spotify.com/dashboard and copy its Client ID and
 *    Client Secret.
 * 2) Follow README.md to run Spotify's OAuth2 Authorization Code flow once
 *    (in a browser, outside the device) and obtain a refresh token.
 * 3) Fill in SPOTIFY_CLIENT_ID, SPOTIFY_CLIENT_SECRET, and
 *    SPOTIFY_REFRESH_TOKEN below.
 * 4) Configure WiFi credentials via menuconfig, then build and flash.
 * 5) Play something on Spotify on any device logged into the same account;
 *    the album screen updates on its next poll (every POLL_SECONDS).
 *
 * Expected output:
 * - E-paper: album art, track title, artist, album name, and a playback
 *   progress bar, or a "Nothing playing" screen when Spotify reports no
 *   active playback.
 * - Serial Monitor: WiFi/token/API status and error messages.
 *
 * Notes:
 * - Display mode: Inkplate 13SPECTRA's native 6-color e-paper mode (there
 *   is no setDisplayMode() call on this board - it always renders in
 *   color). Colors used: INKPLATE_BLACK/WHITE only. Inkplate 13SPECTRA has
 *   no INKPLATE_ORANGE, unlike Inkplate 6Color, but this example doesn't
 *   use either.
 * - Orientation: the board defaults to rotation 3 (landscape) right after
 *   construction (see Inkplate::Inkplate() for
 *   CONFIG_INKPLATE_BOARD_INKPLATE13). Both screens in Gui.cpp explicitly
 *   call display.setRotation(0) before drawing, switching to the
 *   1200x1600 portrait canvas their single-column layout is designed for -
 *   see the comment above that call in Gui.cpp for why this differs from
 *   what the original Arduino sketch's comments claimed.
 * - Power: the board deep sleeps between polls (POLL_SECONDS, includes.h);
 *   execution restarts from app_main() on every wake, so persistent state
 *   (lastAlbumId, lastWasNothingPlaying) lives in RTC memory.
 * - The access token is refreshed on every wake rather than cached across
 *   sleeps (it isn't persisted), so each poll costs one extra token-refresh
 *   HTTP request. This mirrors the original sketch.
 * - Album art is downloaded and JPEG-decoded via
 *   display.image.draw(url, x, y, dither, invert), which follows HTTP
 *   redirects automatically via esp_http_client. The original sketch used
 *   Inkplate's drawJpegFromWeb(), which has no equivalent in this
 *   component's API - display.image.draw() is the ESP-IDF port's
 *   URL-drawing entry point (see NetworkFunctions.cpp for why the ~640px
 *   Spotify image is picked instead of the ~300px one).
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE13
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate13 in the boards menu."
#endif

#include "esp_log.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Gui.h"
#include "NetworkFunctions.h"
#include "includes.h"

static const char *TAG = "SPOTIFY_ALBUM";

// TODO: fill in your Spotify Developer app's Client ID
// (https://developer.spotify.com/dashboard -> your app -> Settings).
#define SPOTIFY_CLIENT_ID "your_spotify_client_id"

// TODO: fill in your Spotify Developer app's Client Secret (same page).
#define SPOTIFY_CLIENT_SECRET "your_spotify_client_secret"

// TODO: fill in a Spotify refresh token for your account. This CANNOT be
// generated on the device - see README.md to run Spotify's OAuth2
// Authorization Code flow once, in a browser, to obtain it.
#define SPOTIFY_REFRESH_TOKEN "your_spotify_refresh_token"

#define WIFI_CONNECT_TIMEOUT_MS 20000

// Persisted across deep sleep so the e-paper is only redrawn when what's
// playing actually changes.
RTC_DATA_ATTR static char lastAlbumId[SPOTIFY_ID_MAX_LEN] = {0};
RTC_DATA_ATTR static bool lastWasNothingPlaying = true;

static void goToSleep(uint32_t seconds) {
  ESP_LOGI(TAG, "Deep sleeping for %u seconds...", (unsigned)seconds);
  vTaskDelay(pdMS_TO_TICKS(50));
  esp_sleep_enable_timer_wakeup((uint64_t)seconds * 1000000ULL);
  esp_deep_sleep_start();
}

// `display` is NOT a global: it's constructed as a local in app_main(). A
// file-scope `Inkplate display;` would race the library's own global
// I2C/PCAL peripheral objects (in BoardCommon.cpp) - C++ leaves
// cross-translation-unit static init order unspecified, so the Inkplate
// ctor can run before the I2C bus/expander objects it depends on, leaving
// peripherals uninitialized.
extern "C" void app_main(void) {
  Inkplate display;
  Gui gui(display);

  gui.begin();

  ESP_LOGI(TAG, "--- Inkplate Spotify Album ---");

  // Connect to WiFi using credentials configured via menuconfig (never
  // hardcoded).
  if (display.wifi.begin() != ESP_OK ||
      !display.wifi.waitForConnect(WIFI_CONNECT_TIMEOUT_MS)) {
    ESP_LOGE(TAG, "WiFi connection failed");
    goToSleep(POLL_SECONDS);
    return;
  }

  NetworkFunctions net(SPOTIFY_CLIENT_ID, SPOTIFY_CLIENT_SECRET,
                       SPOTIFY_REFRESH_TOKEN);

  char accessToken[SPOTIFY_TOKEN_MAX_LEN];
  if (!net.spotifyRefreshAccessToken(accessToken, sizeof(accessToken))) {
    goToSleep(POLL_SECONDS);
    return;
  }

  SpotifyNowPlaying now;
  if (!net.spotifyGetCurrentlyPlaying(accessToken, now)) {
    goToSleep(POLL_SECONDS);
    return;
  }

  if (!now.isPlaying) {
    if (!lastWasNothingPlaying) {
      ESP_LOGI(TAG, "State changed: was playing -> now nothing playing. "
                    "Updating display.");
      gui.renderNothingPlaying();
      lastWasNothingPlaying = true;
      lastAlbumId[0] = 0;
    } else {
      ESP_LOGI(TAG, "Still nothing playing. Skipping display update.");
    }

    goToSleep(POLL_SECONDS);
    return;
  }

  if (now.albumId[0] == '\0') {
    ESP_LOGI(TAG, "Playing but albumId empty. Showing Nothing playing UI.");
    if (!lastWasNothingPlaying) {
      gui.renderNothingPlaying();
      lastWasNothingPlaying = true;
      lastAlbumId[0] = 0;
    } else {
      ESP_LOGI(TAG, "Still in nothing-playing UI state. Skipping display "
                    "update.");
    }

    goToSleep(POLL_SECONDS);
    return;
  }

  bool albumSame =
      (strncmp(lastAlbumId, now.albumId, sizeof(lastAlbumId) - 1) == 0);

  if (albumSame && !lastWasNothingPlaying) {
    ESP_LOGI(TAG, "Album unchanged since last wake. Skipping e-paper "
                  "refresh.");
  } else {
    ESP_LOGI(TAG,
             "Album changed (or returning from nothing-playing). Updating "
             "display.");
    gui.renderAlbumScreen(now.albumName, now.artistName, now.trackName,
                          now.imageUrl, now.progressMs, now.durationMs);

    snprintf(lastAlbumId, sizeof(lastAlbumId), "%s", now.albumId);
    lastWasNothingPlaying = false;
  }

  goToSleep(POLL_SECONDS);
}
