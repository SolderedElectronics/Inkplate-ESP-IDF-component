/**
 * @file        NetworkFunctions.h
 * @brief       Refreshes a Spotify OAuth2 access token and fetches the
 *              "currently playing" track from the Spotify Web API.
 *
 * Ported from the Inkplate10_Spotify_Dashboard Arduino example. The
 * original used WiFiClientSecure + HTTPClient + ArduinoJson; this port
 * performs the same HTTPS POST (token refresh) and GET (currently-playing)
 * requests with esp_http_client (verified against the ESP-IDF certificate
 * bundle) and parses the JSON responses with cJSON. WiFi connection itself
 * is handled in main.cpp via display.wifi (menuconfig-driven credentials),
 * not by this class. None of this layer is board-specific - it is identical
 * to the Inkplate6Color port of this example.
 */
#pragma once
#include "includes.h"

class NetworkFunctions {
public:
  /**
   * @brief Construct with the Spotify Developer app credentials and a
   *        refresh token obtained once via Spotify's OAuth2 Authorization
   *        Code flow (see README.md).
   */
  NetworkFunctions(const char *spotifyClientId, const char *spotifyClientSecret,
                    const char *spotifyRefreshToken);

  /**
   * @brief POSTs to accounts.spotify.com/api/token using the refresh_token
   *        grant, authenticated with HTTP Basic auth
   *        (base64("clientId:clientSecret") built via
   *        mbedtls_base64_encode).
   *
   * @param outAccessToken buffer to receive the null-terminated access
   *                        token on success.
   * @param outAccessTokenSize size of outAccessToken in bytes.
   * @return true on success, false on a network, HTTP, or JSON error.
   */
  bool spotifyRefreshAccessToken(char *outAccessToken, size_t outAccessTokenSize);

  /**
   * @brief GETs api.spotify.com/v1/me/player/currently-playing with
   *        "Authorization: Bearer <accessToken>".
   *
   * @param accessToken a valid Spotify access token.
   * @param out filled with the current playback state.
   * @return true if the request itself succeeded (even if nothing is
   *         currently playing - see out.isPlaying), false on a network,
   *         HTTP, or JSON error.
   */
  bool spotifyGetCurrentlyPlaying(const char *accessToken, SpotifyNowPlaying &out);

private:
  char _clientId[128];
  char _clientSecret[128];
  char _refreshToken[SPOTIFY_TOKEN_MAX_LEN];

  /**
   * @brief Base64-encodes the null-terminated string `in` into `out`
   *        (null-terminated), via mbedtls_base64_encode.
   *
   * @return true on success, false if `out`/`outSize` is too small or the
   *         mbedtls call fails.
   */
  bool base64Encode(const char *in, char *out, size_t outSize);
};
