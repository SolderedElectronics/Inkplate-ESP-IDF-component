/**
 * @file        Gui.h
 * @brief       Draws the Spotify dashboard ("now playing" and "nothing
 *              playing" screens) on the Inkplate 6Color e-paper panel.
 */
#pragma once
#include "includes.h"

class Gui {
public:
  explicit Gui(Inkplate &disp);

  void begin();

  void renderNothingPlaying();

  void renderAlbumScreen(const char *albumName, const char *artistName,
                          const char *trackName, const char *imageUrl,
                          uint32_t progressMs, uint32_t durationMs);

private:
  Inkplate &display;

  // Simple greedy word-wrap helper, ported from the original sketch for API
  // parity. Not currently called by either screen above (the album screen
  // uses its own center+ellipsize layout instead) - same as the original.
  void drawWrappedText(const char *text, int x, int y, int w, int h,
                       int lineGapPx);

  // Formats milliseconds as "M:SS" into `out`.
  void formatMs(uint32_t ms, char *out, size_t outSize);
};
