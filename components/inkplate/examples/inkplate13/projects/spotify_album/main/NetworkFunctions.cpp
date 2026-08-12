/**
 * @file        NetworkFunctions.cpp
 * @brief       Spotify OAuth2 token refresh + "currently playing" fetch,
 *              using esp_http_client and cJSON.
 */
#include "NetworkFunctions.h"

#include "cJSON.h"
#include "esp_crt_bundle.h"
#include "esp_http_client.h"
#include "esp_log.h"

static const char *TAG = "SPOTIFY_NET";

#define SPOTIFY_TOKEN_URL "https://accounts.spotify.com/api/token"
#define SPOTIFY_NOW_PLAYING_URL                                              \
  "https://api.spotify.com/v1/me/player/currently-playing"
#define HTTP_TIMEOUT_MS 15000
// Fallback buffer size used when the server doesn't send a Content-Length
// header (e.g. chunked responses). Spotify's responses are small (token) or
// a few KB (currently-playing with album art metadata), so 16 KB is ample.
#define HTTP_RESPONSE_FALLBACK_SIZE 16384

NetworkFunctions::NetworkFunctions(const char *spotifyClientId,
                                   const char *spotifyClientSecret,
                                   const char *spotifyRefreshToken) {
  snprintf(_clientId, sizeof(_clientId), "%s", spotifyClientId);
  snprintf(_clientSecret, sizeof(_clientSecret), "%s", spotifyClientSecret);
  snprintf(_refreshToken, sizeof(_refreshToken), "%s", spotifyRefreshToken);
}

bool NetworkFunctions::base64Encode(const char *in, char *out, size_t outSize) {
  size_t inLen = strlen(in);

  // First call (dstlen=0) reports the required output size in *olen.
  size_t requiredLen = 0;
  mbedtls_base64_encode(nullptr, 0, &requiredLen, (const unsigned char *)in, inLen);
  if (requiredLen == 0 || requiredLen > outSize) {
    ESP_LOGE(TAG, "base64Encode: output buffer too small (%u needed, %u given)",
             (unsigned)requiredLen, (unsigned)outSize);
    return false;
  }

  size_t outLen = 0;
  int rc = mbedtls_base64_encode((unsigned char *)out, outSize, &outLen,
                                 (const unsigned char *)in, inLen);
  if (rc != 0) {
    ESP_LOGE(TAG, "mbedtls_base64_encode failed: %d", rc);
    return false;
  }

  out[outLen] = '\0';
  return true;
}

// Reads the full body of an already-opened esp_http_client request into a
// heap buffer sized from Content-Length (falling back to
// HTTP_RESPONSE_FALLBACK_SIZE for chunked responses), null-terminated.
// Caller must free() the result. Returns nullptr on allocation failure.
static char *readHttpResponse(esp_http_client_handle_t client, int *outStatusCode) {
  int64_t contentLen = esp_http_client_fetch_headers(client);
  *outStatusCode = esp_http_client_get_status_code(client);

  size_t bufSize =
      contentLen > 0 ? (size_t)contentLen : HTTP_RESPONSE_FALLBACK_SIZE;
  char *buffer = (char *)malloc(bufSize + 1);
  if (!buffer) {
    ESP_LOGE(TAG, "Failed to allocate %u bytes for HTTP response",
             (unsigned)(bufSize + 1));
    return nullptr;
  }

  size_t totalRead = 0;
  int r;
  while (totalRead < bufSize &&
         (r = esp_http_client_read(client, buffer + totalRead,
                                   bufSize - totalRead)) > 0) {
    totalRead += (size_t)r;
  }
  buffer[totalRead] = '\0';
  return buffer;
}

bool NetworkFunctions::spotifyRefreshAccessToken(char *outAccessToken,
                                                 size_t outAccessTokenSize) {
  // Basic auth header: "Basic " + base64("clientId:clientSecret").
  char basic[256];
  snprintf(basic, sizeof(basic), "%s:%s", _clientId, _clientSecret);

  char basicB64[344]; // enough for a base64'd 256-byte input, plus slack
  if (!base64Encode(basic, basicB64, sizeof(basicB64))) {
    return false;
  }

  char authHeader[360];
  snprintf(authHeader, sizeof(authHeader), "Basic %s", basicB64);

  char body[600];
  snprintf(body, sizeof(body), "grant_type=refresh_token&refresh_token=%s",
           _refreshToken);

  // accounts.spotify.com is signed by a well-known public CA, so verify the
  // server certificate using the ESP-IDF certificate bundle rather than
  // pinning a single certificate or disabling verification.
  esp_http_client_config_t config = {};
  config.url = SPOTIFY_TOKEN_URL;
  config.method = HTTP_METHOD_POST;
  config.timeout_ms = HTTP_TIMEOUT_MS;
  config.crt_bundle_attach = esp_crt_bundle_attach;

  esp_http_client_handle_t client = esp_http_client_init(&config);
  if (!client) {
    ESP_LOGE(TAG, "Failed to init HTTP client (token)");
    return false;
  }

  esp_http_client_set_header(client, "Authorization", authHeader);
  esp_http_client_set_header(client, "Content-Type",
                             "application/x-www-form-urlencoded");

  esp_err_t err = esp_http_client_open(client, strlen(body));
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "HTTP open failed (token): %s", esp_err_to_name(err));
    esp_http_client_cleanup(client);
    return false;
  }

  int wlen = esp_http_client_write(client, body, strlen(body));
  if (wlen < 0) {
    ESP_LOGE(TAG, "HTTP write failed (token)");
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    return false;
  }

  int statusCode = 0;
  char *payload = readHttpResponse(client, &statusCode);
  esp_http_client_close(client);
  esp_http_client_cleanup(client);

  if (!payload) {
    return false;
  }

  if (statusCode != 200) {
    ESP_LOGE(TAG, "Token refresh failed. HTTP %d\n%s", statusCode, payload);
    free(payload);
    return false;
  }

  cJSON *root = cJSON_Parse(payload);
  free(payload);
  if (!root) {
    ESP_LOGE(TAG, "Token JSON parse error");
    return false;
  }

  cJSON *tokenObj = cJSON_GetObjectItem(root, "access_token");
  const char *token =
      cJSON_IsString(tokenObj) ? cJSON_GetStringValue(tokenObj) : nullptr;
  if (!token || !strlen(token)) {
    ESP_LOGE(TAG, "No access_token in response");
    cJSON_Delete(root);
    return false;
  }

  snprintf(outAccessToken, outAccessTokenSize, "%s", token);
  cJSON_Delete(root);
  ESP_LOGI(TAG, "Got access token.");
  return true;
}

bool NetworkFunctions::spotifyGetCurrentlyPlaying(const char *accessToken,
                                                  SpotifyNowPlaying &out) {
  out = SpotifyNowPlaying{}; // reset

  char authHeader[SPOTIFY_TOKEN_MAX_LEN + 16];
  snprintf(authHeader, sizeof(authHeader), "Bearer %s", accessToken);

  // api.spotify.com is also signed by a well-known public CA.
  esp_http_client_config_t config = {};
  config.url = SPOTIFY_NOW_PLAYING_URL;
  config.method = HTTP_METHOD_GET;
  config.timeout_ms = HTTP_TIMEOUT_MS;
  config.crt_bundle_attach = esp_crt_bundle_attach;

  esp_http_client_handle_t client = esp_http_client_init(&config);
  if (!client) {
    ESP_LOGE(TAG, "Failed to init HTTP client (currently-playing)");
    return false;
  }

  esp_http_client_set_header(client, "Authorization", authHeader);
  esp_http_client_set_header(client, "Accept", "application/json");

  esp_err_t err = esp_http_client_open(client, 0);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "HTTP open failed (currently-playing): %s",
             esp_err_to_name(err));
    esp_http_client_cleanup(client);
    return false;
  }

  int statusCode = 0;
  char *payload = readHttpResponse(client, &statusCode);
  esp_http_client_close(client);
  esp_http_client_cleanup(client);

  if (statusCode == 204) {
    // No Content: nothing is currently playing. This is a successful
    // request, not a failure.
    ESP_LOGI(TAG, "Spotify: nothing playing (204).");
    if (payload) {
      free(payload);
    }
    out.isPlaying = false;
    return true;
  }

  if (!payload) {
    return false;
  }

  if (statusCode != 200) {
    ESP_LOGE(TAG, "Currently-playing failed. HTTP %d\n%s", statusCode, payload);
    free(payload);
    return false;
  }

  cJSON *root = cJSON_Parse(payload);
  free(payload);
  if (!root) {
    ESP_LOGE(TAG, "Currently-playing JSON parse error");
    return false;
  }

  cJSON *isPlayingObj = cJSON_GetObjectItem(root, "is_playing");
  out.isPlaying = cJSON_IsTrue(isPlayingObj);

  cJSON *progressObj = cJSON_GetObjectItem(root, "progress_ms");
  out.progressMs = (progressObj && cJSON_IsNumber(progressObj))
                       ? (uint32_t)progressObj->valuedouble
                       : 0;

  if (!out.isPlaying) {
    ESP_LOGI(TAG, "Spotify: not playing (is_playing=false).");
    cJSON_Delete(root);
    return true;
  }

  cJSON *item = cJSON_GetObjectItem(root, "item");
  if (!item || cJSON_IsNull(item)) {
    ESP_LOGI(TAG, "Spotify: playing but item is null.");
    cJSON_Delete(root);
    return true;
  }

  cJSON *durationObj = cJSON_GetObjectItem(item, "duration_ms");
  out.durationMs = (durationObj && cJSON_IsNumber(durationObj))
                       ? (uint32_t)durationObj->valuedouble
                       : 0;

  cJSON *typeObj = cJSON_GetObjectItem(item, "type");
  const char *itemType = cJSON_IsString(typeObj) ? cJSON_GetStringValue(typeObj) : "";
  if (strcmp(itemType, "track") != 0) {
    ESP_LOGI(TAG, "Spotify: item type is '%s' (expected 'track').", itemType);
    out.isPlaying = false;
    cJSON_Delete(root);
    return true;
  }

  cJSON *nameObj = cJSON_GetObjectItem(item, "name");
  if (cJSON_IsString(nameObj)) {
    snprintf(out.trackName, sizeof(out.trackName), "%s",
             cJSON_GetStringValue(nameObj));
  }

  cJSON *album = cJSON_GetObjectItem(item, "album");
  if (album) {
    cJSON *albumIdObj = cJSON_GetObjectItem(album, "id");
    if (cJSON_IsString(albumIdObj)) {
      snprintf(out.albumId, sizeof(out.albumId), "%s",
               cJSON_GetStringValue(albumIdObj));
    }

    cJSON *albumNameObj = cJSON_GetObjectItem(album, "name");
    if (cJSON_IsString(albumNameObj)) {
      snprintf(out.albumName, sizeof(out.albumName), "%s",
               cJSON_GetStringValue(albumNameObj));
    }

    // Spotify's "images" array is sorted largest-first (typically 640, 300,
    // and 64 px). This example draws the cover into a 640x640 slot
    // (Gui.cpp), so index 0 (the ~640px image) is used directly - unlike
    // the Inkplate 6Color Spotify Dashboard port, which uses a ~300x300
    // slot and picks index 1 instead. cJSON_GetArrayItem() safely returns
    // NULL if the array has fewer entries than requested.
    cJSON *images = cJSON_GetObjectItem(album, "images");
    if (cJSON_IsArray(images) && cJSON_GetArraySize(images) > 0) {
      cJSON *img0 = cJSON_GetArrayItem(images, 0);
      cJSON *urlObj = img0 ? cJSON_GetObjectItem(img0, "url") : nullptr;
      if (cJSON_IsString(urlObj)) {
        snprintf(out.imageUrl, sizeof(out.imageUrl), "%s",
                 cJSON_GetStringValue(urlObj));
      }
    }
  }

  cJSON *artists = cJSON_GetObjectItem(item, "artists");
  if (cJSON_IsArray(artists) && cJSON_GetArraySize(artists) > 0) {
    cJSON *artist0 = cJSON_GetArrayItem(artists, 0);
    cJSON *artistNameObj = artist0 ? cJSON_GetObjectItem(artist0, "name") : nullptr;
    if (cJSON_IsString(artistNameObj)) {
      snprintf(out.artistName, sizeof(out.artistName), "%s",
               cJSON_GetStringValue(artistNameObj));
    }
  }

  ESP_LOGI(TAG, "Spotify: playing track parsed:");
  ESP_LOGI(TAG, "  albumId: %s", out.albumId);
  ESP_LOGI(TAG, "  album:   %s", out.albumName);
  ESP_LOGI(TAG, "  artist:  %s", out.artistName);
  ESP_LOGI(TAG, "  track:   %s", out.trackName);
  ESP_LOGI(TAG, "  image:   %s", out.imageUrl);

  cJSON_Delete(root);
  return true;
}
