/**
 * @file        includes.h
 * @brief       Shared includes, fonts, layout/color constants, and the
 *              SpotifyNowPlaying struct used by NetworkFunctions and Gui.
 */
#pragma once

#include "Inkplate.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

// mbedTLS base64, used by NetworkFunctions to build the Basic auth header
// for the Spotify token-refresh request.
#include "mbedtls/base64.h"

// FreeFonts (Adafruit_GFX-compatible) used by Gui.cpp.
#include "fonts/FreeSans9pt7b.h"
#include "fonts/FreeSans12pt7b.h"
#include "fonts/FreeSans18pt7b.h"
#include "fonts/FreeSansBold18pt7b.h"
#include "fonts/FreeSansBold24pt7b.h"

// How long (in seconds) the board deep sleeps between Spotify polls.
static const uint32_t POLL_SECONDS = 120;

// Colors for the "Nothing playing" screen. Inkplate 13SPECTRA is a 6-color
// e-paper board (INKPLATE_BLACK/WHITE/YELLOW/RED/BLUE/GREEN - see
// Inkplate13.h). It has no INKPLATE_ORANGE, unlike Inkplate 6Color.
// The plain BLACK/WHITE macros (GraphicsDefs.h) are NOT used here: they are
// only correct on the grayscale/B&W Inkplate boards - on this component
// they resolve to 1/0 regardless of board (the color-board branch is dead
// code, gated on a macro that is never defined), which would be the wrong
// color index on a 6-color panel.
static const uint8_t BG_DARK = INKPLATE_BLACK;
static const uint8_t FG_LIGHT = INKPLATE_WHITE;

// Fixed buffer sizes for the strings pulled out of the Spotify API JSON
// responses (replacing the original sketch's Arduino String fields).
#define SPOTIFY_ID_MAX_LEN 64     // Spotify IDs are 22 base62 chars.
#define SPOTIFY_NAME_MAX_LEN 160  // Track/artist/album names.
#define SPOTIFY_URL_MAX_LEN 256   // Album art image URL.
#define SPOTIFY_TOKEN_MAX_LEN 512 // OAuth2 access token.

/**
 * @brief Snapshot of the Spotify "currently playing" response, filled by
 *        NetworkFunctions::spotifyGetCurrentlyPlaying().
 */
struct SpotifyNowPlaying {
  bool isPlaying = false;
  char albumId[SPOTIFY_ID_MAX_LEN] = {0};
  char albumName[SPOTIFY_NAME_MAX_LEN] = {0};
  char artistName[SPOTIFY_NAME_MAX_LEN] = {0};
  char trackName[SPOTIFY_NAME_MAX_LEN] = {0};
  char imageUrl[SPOTIFY_URL_MAX_LEN] = {0};
  uint32_t durationMs = 0;
  uint32_t progressMs = 0;
};
