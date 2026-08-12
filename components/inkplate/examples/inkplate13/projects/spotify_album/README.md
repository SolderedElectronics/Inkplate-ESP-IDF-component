# Spotify Album

Show what's currently playing on Spotify — album art, track, artist, and a progress bar — on Soldered Inkplate 13SPECTRA's 6-color e-paper panel.

## Overview

Connects Inkplate 13SPECTRA to WiFi, refreshes a Spotify **OAuth2 access token** via the `refresh_token` grant (`POST https://accounts.spotify.com/api/token`, authenticated with an HTTP **Basic** auth header — `base64("clientId:clientSecret")`, built with `mbedtls_base64_encode`), then polls the Spotify Web API's ["currently playing"](https://developer.spotify.com/documentation/web-api/reference/get-the-users-currently-playing-track) endpoint (`GET https://api.spotify.com/v1/me/player/currently-playing`, authenticated with `Authorization: Bearer <access_token>`). The JSON responses are parsed with cJSON, and the current track/artist/album plus a playback progress bar and the album cover art are rendered on the e-paper panel. Both HTTPS requests use `esp_http_client` verified against the ESP-IDF certificate bundle (`esp_crt_bundle_attach`), since both `accounts.spotify.com` and `api.spotify.com` are signed by a well-known public CA.

Networking (`NetworkFunctions.cpp/.h`) and drawing (`Gui.cpp/.h`) are split into their own files, mirroring the original `Inkplate13SPECTRA_Spotify_Album` Arduino sketch's `src/` layout.

Between polls, the board deep sleeps for `POLL_SECONDS` (`main/includes.h`, default 120 s) and restarts from `app_main()` on wake. The last-drawn album ID and "nothing playing" state are kept in RTC memory across deep sleep, so the e-paper is only redrawn when what's playing actually changes — e-paper refreshes are slow and visible, so this avoids needless flicker.

## Hardware Required

- Soldered Inkplate 13SPECTRA
- USB cable
- Stable WiFi (2.4 GHz) connection with Internet access
- A Spotify account (free or Premium both expose the "currently playing" endpoint) and a Spotify Developer app

## Setup

### 1. Create a Spotify Developer app

Go to the [Spotify Developer Dashboard](https://developer.spotify.com/dashboard), log in, and click **Create app**. Fill in a name/description, and set a **Redirect URI** — this only needs to work in step 3 below, on your computer, not on the device. Spotify accepts loopback addresses for this:

```
http://127.0.0.1:8888/callback
```

Save the app, then open its **Settings** to copy the **Client ID** and **Client Secret**.

### 2. Understand why a browser step is required

The Spotify Web API uses OAuth2's **Authorization Code flow**, which requires a human to log in and approve access in a browser at least once. There is no way to skip this from the device — the device only ever performs the much simpler *refresh token* step afterwards. You do steps 2-4 below **once, on your computer**, to obtain a long-lived `refresh_token`; the device then exchanges that refresh token for short-lived access tokens on every poll, indefinitely (until you revoke access in your Spotify account settings).

### 3. Get an authorization code

With your Client ID and Redirect URI from step 1, open this URL in a browser (replace `YOUR_CLIENT_ID` and, if you changed it, the redirect URI — both must match exactly what you set in the Dashboard):

```
https://accounts.spotify.com/authorize?client_id=YOUR_CLIENT_ID&response_type=code&redirect_uri=http%3A%2F%2F127.0.0.1%3A8888%2Fcallback&scope=user-read-currently-playing%20user-read-playback-state
```

Log in and click **Agree**. Spotify redirects your browser to something like:

```
http://127.0.0.1:8888/callback?code=AQD...long-string...
```

The page itself will fail to load (nothing is listening on port 8888) — that's fine. Copy the `code=...` value from the address bar; that is your one-time authorization code.

### 4. Exchange the code for a refresh token

Run this once from a terminal (requires `curl`), filling in your Client ID, Client Secret, redirect URI, and the `code` from step 3:

```bash
curl -X POST https://accounts.spotify.com/api/token \
  -H "Authorization: Basic $(printf '%s' "YOUR_CLIENT_ID:YOUR_CLIENT_SECRET" | base64)" \
  -d grant_type=authorization_code \
  -d code="YOUR_AUTHORIZATION_CODE" \
  -d redirect_uri="http://127.0.0.1:8888/callback"
```

The JSON response includes a `refresh_token` field — copy its value. This is the token the device will use forever (until revoked), so keep it as secret as a password.

### 5. Fill in your credentials

In `main/main.cpp`:

```cpp
#define SPOTIFY_CLIENT_ID "your_spotify_client_id"
#define SPOTIFY_CLIENT_SECRET "your_spotify_client_secret"
#define SPOTIFY_REFRESH_TOKEN "your_spotify_refresh_token"
```

### 6. Configure WiFi and board

Run `idf.py menuconfig` and navigate to:
- **Inkplate Boards → Inkplate13** (used for both Inkplate 13 and Inkplate 13SPECTRA)
- **WiFi Configuration → Enter your SSID and password**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

- Display: while something is playing, the album cover art (dithered to the 6-color palette), track title, artist, album name, a playback progress bar with elapsed/remaining time, and static previous/play/next icons (the icons are not interactive — this board has no touch input in this example). When Spotify reports no active playback, a "Nothing playing" screen is shown instead.
- Serial Monitor: WiFi connection status, token-refresh status, and any HTTP/API error messages.
- The board deep sleeps for `POLL_SECONDS` (default 120 s / 2 minutes) between polls. The e-paper is only redrawn when the currently-playing album (or the playing/not-playing state) actually changes since the last wake, to avoid unnecessary refreshes.

## Notes

- Display mode: Inkplate 13SPECTRA is a 6-color e-paper board (black, white, yellow, red, blue, green via the `INKPLATE_*` color macros — see `Inkplate13.h`). There is no `setDisplayMode()` call on this board — it always renders in its native color mode. **Color note:** unlike Inkplate 6Color, there is **no `INKPLATE_ORANGE`** on this board, but this example only uses `INKPLATE_BLACK`/`INKPLATE_WHITE`.
- Orientation: the board defaults to rotation 3 (landscape) right after construction (`Inkplate::Inkplate()` calls `setRotation(3)` for `CONFIG_INKPLATE_BOARD_INKPLATE13`). Both screens in `Gui.cpp` explicitly call `display.setRotation(0)` before drawing, switching to the 1200x1600 *portrait* canvas the single-column layout is designed for. The original Arduino sketch's `Gui.cpp` also calls `setRotation(0)` before drawing, but its inline comments claim this yields a 1600x1200 *landscape* canvas — that isn't actually what `setRotation(0)` produces on this board's native 1200x1600 panel. This port keeps the same explicit `setRotation(0)` call (since it's the only way to reach the intended single-column portrait layout on this board), but every pixel coordinate in `Gui.cpp` was re-derived against the real 1200x1600 canvas rather than the original's mislabeled comments, and fits comfortably within it.
- Album art: `display.image.draw(url, x, y, dither, invert)` downloads and JPEG-decodes the cover art, following HTTP redirects automatically via `esp_http_client`, and applies Jarvis-Judice-Ninke dithering (`display.image.setDitherKernel(JarvisJudiceNinke)`) to approximate the photo in the 6-color palette. Spotify's `images` array is sorted largest-first (typically 640, 300, and 64 px); since this example draws into a 640x640 slot, it requests index 0 (the ~640px image) — a bigger slot than the Inkplate 6Color Spotify Dashboard port, which uses a ~300x300 slot and picks index 1 instead.
- Token handling: the access token is refreshed on **every** poll rather than being cached and reused until it expires (Spotify access tokens are normally valid for about an hour). This is simpler and safer against clock-skew/expiry edge cases, at the cost of one extra HTTPS request per poll — the refresh token itself never expires from normal use.
- TLS: both `accounts.spotify.com` and `api.spotify.com` are verified against the ESP-IDF certificate bundle (`esp_crt_bundle_attach`, requires `CONFIG_MBEDTLS_CERTIFICATE_BUNDLE=y` in `sdkconfig.defaults`) rather than using an insecure/no-verify connection.
- `CONFIG_ESP_TLS_INSECURE` / `CONFIG_ESP_TLS_SKIP_SERVER_CERT_VERIFY` are kept enabled in `sdkconfig.defaults` for parity with other ported examples in this repo, but are not relied on here since `crt_bundle_attach` is set explicitly on every request.
- Buffer sizes: track/artist/album names, the access token, and the image URL are all fixed-size `char[]` buffers (see `main/includes.h`) rather than dynamically-sized strings; unusually long metadata is truncated via `snprintf`.
- Protect your Client Secret and refresh token — anyone with both can read (and, with a broader `scope`, control) your Spotify playback. Do not commit real credentials to a public repository.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
- Spotify Web API - Authorization Code flow: https://developer.spotify.com/documentation/web-api/tutorials/code-flow
- Spotify Web API - Get Currently Playing Track: https://developer.spotify.com/documentation/web-api/reference/get-the-users-currently-playing-track
