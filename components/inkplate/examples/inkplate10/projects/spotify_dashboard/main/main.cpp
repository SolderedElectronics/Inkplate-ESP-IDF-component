/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Spotify "now playing" dashboard for Soldered Inkplate 10.
 *
 * @details     Connects Inkplate 10 to WiFi, refreshes a Spotify OAuth2
 *              access token via the refresh_token grant (HTTP Basic auth,
 *              base64("clientId:clientSecret") built with
 *              mbedtls_base64_encode), then polls the Spotify Web API's
 *              "currently playing" endpoint (Bearer access token) and
 *              renders the track/artist/album, a playback progress bar, and
 *              the album cover art on the grayscale e-paper panel.
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
 * - Board:      Soldered Inkplate 10
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 10, USB cable (battery recommended for deployment)
 * - Extra:      WiFi (2.4 GHz) + Internet access, a Spotify Developer app
 *               (Client ID + Client Secret), and a Spotify refresh token -
 *               see README.md for how to obtain the refresh token, which
 *               requires completing Spotify's OAuth2 Authorization Code
 *               flow once, outside the device, in a browser.
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate10
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
 *    the dashboard updates on its next poll (every POLL_SECONDS).
 *
 * Expected output:
 * - E-paper: album art, track title, artist, album name, and a playback
 *   progress bar, or a "Nothing playing" screen when Spotify reports no
 *   active playback.
 * - Serial Monitor: WiFi/token/API status and error messages.
 *
 * Notes:
 * - Display: 3-bit GRAYSCALE mode (8 shades, 0=black..7=white). This is
 *   already Inkplate 10's default display mode, but Gui::begin() sets it
 *   explicitly for clarity, matching the original sketch's
 *   Inkplate display(INKPLATE_3BIT) constructor argument. Partial updates
 *   are not supported in grayscale mode, so every refresh is a full
 *   refresh - unlike the Inkplate6Color original, there is no color/dither
 *   palette involved; the album art is drawn straight to grayscale with
 *   Floyd-Steinberg dithering (display.image.draw()'s `dither` flag).
 * - Orientation: the album screen rotates the panel into portrait
 *   (setRotation(3) in Gui.cpp, giving an 825x1200 canvas) since the layout
 *   is a single centered column; the "nothing playing" screen stays in the
 *   native landscape orientation (1200x825) since it's just one centered
 *   line of text. The native panel is 1200x825.
 * - Power: the board deep sleeps between polls (POLL_SECONDS, includes.h);
 *   execution restarts from app_main() on every wake, so persistent state
 *   (lastAlbumId, lastWasNothingPlaying) lives in RTC memory.
 * - The access token is refreshed on every wake rather than cached across
 *   sleeps (it isn't persisted), so each poll costs one extra token-refresh
 *   HTTP request. This mirrors the original sketch.
 * - Album art is downloaded and JPEG-decoded via
 *   display.image.draw(url, x, y, dither, invert), which follows HTTP
 *   redirects automatically via esp_http_client.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE10
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate10 in the boards menu."
#endif

#include "esp_log.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Gui.h"
#include "NetworkFunctions.h"
#include "includes.h"

static const char *TAG = "SPOTIFY_DASHBOARD";

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

extern "C" void app_main(void) {
  Inkplate display;
  Gui gui(display);

  gui.begin();

  ESP_LOGI(TAG, "--- Inkplate Spotify Album Dashboard ---");

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
