# Spotify Dashboard

Show what's currently playing on Spotify — album art, track, artist, and a progress bar — on Soldered Inkplate 10's grayscale e-paper panel.

## Overview

Connects Inkplate 10 to WiFi, refreshes a Spotify **OAuth2 access token** via the `refresh_token` grant (`POST https://accounts.spotify.com/api/token`, authenticated with an HTTP **Basic** auth header — `base64("clientId:clientSecret")`, built with `mbedtls_base64_encode`), then polls the Spotify Web API's ["currently playing"](https://developer.spotify.com/documentation/web-api/reference/get-the-users-currently-playing-track) endpoint (`GET https://api.spotify.com/v1/me/player/currently-playing`, authenticated with `Authorization: Bearer <access_token>`). The JSON responses are parsed with cJSON, and the current track/artist/album plus a playback progress bar and the album cover art are rendered on the e-paper panel. Both HTTPS requests use `esp_http_client` verified against the ESP-IDF certificate bundle (`esp_crt_bundle_attach`), since both `accounts.spotify.com` and `api.spotify.com` are signed by a well-known public CA.

Networking (`NetworkFunctions.cpp/.h`) and drawing (`Gui.cpp/.h`) are split into their own files, mirroring the original `Inkplate10_Spotify_Dashboard` Arduino sketch's `src/` layout. The networking layer is board-agnostic and identical to the Inkplate6Color port of this example; only the drawing code in `Gui.cpp` differs, since Inkplate 10 is a grayscale/B&W panel rather than a 7-color one (see Notes below).

Between polls, the board deep sleeps for `POLL_SECONDS` (`main/includes.h`, default 120 s) and restarts from `app_main()` on wake. The last-drawn album ID and "nothing playing" state are kept in RTC memory across deep sleep, so the e-paper is only redrawn when what's playing actually changes — e-paper refreshes are slow and visible, so this avoids needless flicker.

## Hardware Required

- Soldered Inkplate 10
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
- **Inkplate Boards → Inkplate10**
- **WiFi Configuration → Enter your SSID and password**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

- Display: while something is playing, the album cover art (Floyd-Steinberg dithered to 8 grayscale shades), track title, artist, album name, a playback progress bar with elapsed/remaining time, and static previous/play-pause/next icons (the icons are not interactive — this board has no touch input in this example). When Spotify reports no active playback, a "Nothing playing" screen is shown instead.
- Serial Monitor: WiFi connection status, token-refresh status, and any HTTP/API error messages.
- The board deep sleeps for `POLL_SECONDS` (default 120 s / 2 minutes) between polls. The e-paper is only redrawn when the currently-playing album (or the playing/not-playing state) actually changes since the last wake, to avoid unnecessary refreshes.

## Notes

- Display mode: Inkplate 10 renders in 3-bit **GRAYSCALE** mode (8 shades, `0` = black .. `7` = white) — this is already the board's default mode, but `Gui::begin()` calls `display.setDisplayMode(GRAYSCALE)` explicitly for clarity, matching the original sketch's `Inkplate display(INKPLATE_3BIT)` constructor argument. Unlike the Inkplate6Color/13SPECTRA original, there is **no color palette involved** — the album screen is drawn directly with plain `0` (black) and `7` (white) shade values instead of a 7-color `INKPLATE_*` palette, and the cover art is dithered straight to grayscale rather than to a color palette (no `setDitherKernel()` call is needed or available on this board's `Image` class).
- Orientation: the album screen calls `setRotation(3)` in `Gui.cpp` to draw in portrait (825x1200) since the layout is a single centered column; the "nothing playing" screen stays in the native landscape orientation (`setRotation(0)`, 1200x825) since it's just one centered line of text. This matches the original Inkplate10 sketch, which also rotates only the album screen.
- Album art: `display.image.draw(url, x, y, dither, invert)` downloads and JPEG-decodes the cover art (Spotify's largest available image, typically ~640px), following HTTP redirects automatically via `esp_http_client`, and applies Floyd-Steinberg dithering (the `dither` flag) to render the photo in 8 grayscale shades.
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
