/**
 * @file        Gui.cpp
 * @brief       Drawing code for the Spotify album screens.
 */
#include "Gui.h"

#include "esp_log.h"

#include <algorithm>
#include <cstring>

static const char *TAG = "SPOTIFY_GUI";

Gui::Gui(Inkplate &disp) : display(disp) {}

void Gui::begin() {
  // The ESP-IDF Inkplate class initializes the panel in its constructor
  // (unlike the Arduino library, which needed an explicit display.begin()).
  // This just clears the framebuffer before the first draw, matching the
  // original sketch's gui.begin() call order.
  display.clearDisplay();
}

void Gui::formatMs(uint32_t ms, char *out, size_t outSize) {
  uint32_t totalSeconds = ms / 1000;
  uint32_t minutes = totalSeconds / 60;
  uint32_t seconds = totalSeconds % 60;
  snprintf(out, outSize, "%lu:%02lu", (unsigned long)minutes,
           (unsigned long)seconds);
}

void Gui::drawWrappedText(const char *text, int x, int y, int w, int h,
                          int lineGapPx) {
  int16_t x1, y1;
  uint16_t tw, th;

  int cursorX = x;
  int cursorY = y;

  // Work on a mutable copy, and skip leading spaces (matches String::trim()
  // on the original's leading edge).
  char buf[512];
  snprintf(buf, sizeof(buf), "%s", text);
  char *remaining = buf;
  while (*remaining == ' ')
    remaining++;

  char line[512];

  while (*remaining != '\0') {
    display.getTextBounds("Ag", 0, 0, &x1, &y1, &tw, &th);
    if (cursorY + (int)th > y + h)
      break;

    size_t remLen = strlen(remaining);
    bool hasSpace = strchr(remaining, ' ') != nullptr;

    int bestCut = -1;
    size_t cut = 0;

    while (cut < remLen) {
      const char *spacePos = strchr(remaining + cut, ' ');
      size_t nextSpace = spacePos ? (size_t)(spacePos - remaining) : remLen;

      size_t candidateLen = nextSpace;
      while (candidateLen > 0 && remaining[candidateLen - 1] == ' ')
        candidateLen--;

      snprintf(line, sizeof(line), "%.*s", (int)candidateLen, remaining);
      display.getTextBounds(line, 0, 0, &x1, &y1, &tw, &th);

      if ((int)tw <= w) {
        bestCut = (int)nextSpace;
        cut = nextSpace + 1;
        if (nextSpace >= remLen)
          break;
      } else {
        break;
      }
    }

    if (bestCut < 0) {
      size_t take;
      if (!hasSpace) {
        take = std::min(remLen, (size_t)20);
      } else {
        const char *spacePos = strchr(remaining, ' ');
        take = spacePos ? (size_t)(spacePos - remaining) : remLen;
      }
      snprintf(line, sizeof(line), "%.*s", (int)take, remaining);
      display.setCursor(cursorX, cursorY);
      display.print(line);
      remaining += take;
      while (*remaining == ' ')
        remaining++;
    } else {
      snprintf(line, sizeof(line), "%.*s", bestCut, remaining);
      size_t lineLen = strlen(line);
      while (lineLen > 0 && line[lineLen - 1] == ' ')
        line[--lineLen] = '\0';
      display.setCursor(cursorX, cursorY);
      display.print(line);
      remaining += bestCut;
      while (*remaining == ' ')
        remaining++;
    }

    cursorY += (int)th + lineGapPx;
  }
}

void Gui::renderNothingPlaying() {
  // Portrait orientation - see the long comment at the top of
  // renderAlbumScreen() for why this differs from the board's default
  // rotation.
  display.setRotation(0);
  ESP_LOGI(TAG, "Rendering: Nothing playing screen");
  display.clearDisplay();

  display.fillRect(0, 0, display.width(), display.height(), BG_DARK);

  display.setTextColor(FG_LIGHT);
  display.setFont(&FreeSansBold24pt7b);

  const char *msg = "Nothing playing";
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(msg, 0, 0, &x1, &y1, &w, &h);

  int cx = (display.width() - (int)w) / 2;
  int cy = (display.height() + (int)h) / 2;

  display.setCursor(cx, cy);
  display.print(msg);

  display.display();
}

void Gui::renderAlbumScreen(const char *albumName, const char *artistName,
                            const char *trackName, const char *imageUrl,
                            uint32_t progressMs, uint32_t durationMs) {
  ESP_LOGI(TAG, "Rendering Album Screen");

  // ---- Base canvas ----
  display.clearDisplay();
  display.image.setDitherKernel(JarvisJudiceNinke);

  // The original Inkplate13SPECTRA_Spotify_Album.ino calls
  // display.setRotation(0) before drawing and its comments claim that
  // yields a 1600x1200 *landscape* canvas. That is not what setRotation(0)
  // actually produces on this board (native panel is 1200x1600 - see
  // E_INK_WIDTH/E_INK_HEIGHT in Inkplate13.h - and the Inkplate constructor
  // already defaults to setRotation(3), the landscape orientation, before
  // app_main() runs). setRotation(0) here instead gives a 1200x1600
  // *portrait* canvas, matching the Google Calendar Inkplate13SPECTRA port's
  // use of the same explicit call. Every coordinate below was re-derived
  // against the real 1200x1600 portrait canvas (not the original's
  // mislabeled comments) and fits comfortably within it, so the explicit
  // call is kept exactly as in the original sketch.
  display.setRotation(0);
  display.fillRect(0, 0, display.width(), display.height(), INKPLATE_BLACK);
  display.setTextColor(INKPLATE_WHITE);

  const int W = display.width();  // 1200 (portrait)
  const int H = display.height(); // 1600 (portrait)

  // Hardcoded cover art: 640x640, centered horizontally.
  const int coverSize = 640;
  const int coverX = (W - coverSize) / 2;
  // NOTE: the original sketch computes coverY from W too (not H) - almost
  // certainly a copy/paste slip from the coverX line above. It is kept
  // as-is: with W=1200 this still lands at a sensible position (280px from
  // the top, leaving ~680px below the cover for text/progress/controls,
  // all of which fit within H=1600 - see the layout below), so "fixing" it
  // to use H (which would push everything ~200px further down) is not
  // needed and isn't a bug that affects correctness the way the original
  // Google Calendar port's clearEvents() bug did.
  const int coverY = (W - coverSize) / 2;

  // Header. Kept for parity with the original sketch, which sets a cursor
  // here but never actually prints anything - i.e. this is a no-op in the
  // original too.
  display.setFont(&FreeSans9pt7b);
  display.setCursor(80, 55);

  // Cover frame + image
  display.drawRect(coverX - 6, coverY - 6, coverSize + 12, coverSize + 12,
                   INKPLATE_WHITE);

  bool imgOk = display.image.draw(imageUrl, coverX, coverY, true, false);
  if (!imgOk) {
    ESP_LOGW(TAG, "Failed to draw cover image.");
    display.drawRect(coverX, coverY, coverSize, coverSize, INKPLATE_WHITE);
    display.setFont(&FreeSans12pt7b);

    int16_t x1, y1;
    uint16_t tw, th;
    const char *msg = "Cover image failed";
    display.getTextBounds(msg, 0, 0, &x1, &y1, &tw, &th);
    int tx = (W - (int)tw) / 2 - x1;
    int ty = coverY + coverSize / 2;
    display.setCursor(tx, ty);
    display.print(msg);
  }

  // ---- Text area under cover (Spotify-like hierarchy) ----
  const int padX = 80;
  const int maxTextW = W - (padX * 2);
  const int yShift = 100;

  // Helper: center + ellipsize to maxTextW at a given baseline Y.
  auto drawCenteredEllipsized = [&](const char *in, const GFXfont *font,
                                    int baselineY) {
    display.setFont(font);

    char s[196];
    snprintf(s, sizeof(s), "%s", in);

    int16_t x1, y1;
    uint16_t tw, th;

    display.getTextBounds(s, 0, 0, &x1, &y1, &tw, &th);
    if ((int)tw > maxTextW) {
      char base[192];
      snprintf(base, sizeof(base), "%s", in);
      size_t baseLen = strlen(base);

      char candidate[196];
      while (baseLen > 1) {
        snprintf(candidate, sizeof(candidate), "%s...", base);
        display.getTextBounds(candidate, 0, 0, &x1, &y1, &tw, &th);
        if ((int)tw <= maxTextW) {
          snprintf(s, sizeof(s), "%s", candidate);
          break;
        }
        base[--baseLen] = '\0';
      }
    }

    display.getTextBounds(s, 0, 0, &x1, &y1, &tw, &th);
    int tx = (W - (int)tw) / 2 - x1;
    display.setCursor(tx, baselineY);
    display.print(s);
  };

  // Track title (primary). If trackName empty, fall back to albumName.
  const char *title = (trackName && trackName[0]) ? trackName : albumName;

  const int titleY = coverY + coverSize + 85 + yShift;
  const int artistY = coverY + coverSize + 135 + yShift;
  const int albumY = coverY + coverSize + 170 + yShift;

  drawCenteredEllipsized(title, &FreeSansBold24pt7b, titleY);
  drawCenteredEllipsized(artistName, &FreeSans18pt7b, artistY);
  drawCenteredEllipsized(albumName, &FreeSans12pt7b, albumY);

  // ---- Progress bar ----
  const int barX = padX;
  const int barW = W - (padX * 2);
  const int barH = 10;
  const int barY = coverY + coverSize + 215 + yShift;

  display.drawRect(barX, barY, barW, barH, INKPLATE_WHITE);

  float progress = 0.0f;
  if (durationMs > 0) {
    if (progressMs > durationMs)
      progressMs = durationMs; // clamp
    progress = (float)progressMs / (float)durationMs;
  }

  int fillW = (int)(barW * progress);
  display.fillRect(barX + 1, barY + 1, std::max(0, fillW - 2), barH - 2,
                   INKPLATE_WHITE);

  // Time labels
  display.setFont(&FreeSans9pt7b);
  char leftTime[16];
  char rightTime[16];
  formatMs(progressMs, leftTime, sizeof(leftTime));
  formatMs(durationMs, rightTime, sizeof(rightTime));

  display.setCursor(barX, barY + 30);
  display.print(leftTime);

  {
    int16_t x1, y1;
    uint16_t tw, th;
    display.getTextBounds(rightTime, 0, 0, &x1, &y1, &tw, &th);
    display.setCursor(barX + barW - (int)tw, barY + 30);
    display.print(rightTime);
  }

  // ---- Playback controls (static icons - not interactive; this board has
  // no touch input in this example) ----
  const int controlsY = barY + 85;
  const int cx = W / 2;

  // Previous
  display.fillTriangle(cx - 220, controlsY, cx - 170, controlsY - 28,
                       cx - 170, controlsY + 28, INKPLATE_WHITE);
  display.fillRect(cx - 232, controlsY - 28, 10, 56, INKPLATE_WHITE);

  // Play (circle + bars)
  display.drawCircle(cx, controlsY, 42, INKPLATE_WHITE);
  {
    const int barW2 = 10;
    const int barH2 = 36;
    const int gap = 10;

    int leftBarX = cx - gap / 2 - barW2;
    int rightBarX = cx + gap / 2;
    int barTopY = controlsY - barH2 / 2;

    display.fillRect(leftBarX, barTopY, barW2, barH2, INKPLATE_WHITE);
    display.fillRect(rightBarX, barTopY, barW2, barH2, INKPLATE_WHITE);
  }

  // Next
  display.fillTriangle(cx + 220, controlsY, cx + 170, controlsY - 28,
                       cx + 170, controlsY + 28, INKPLATE_WHITE);
  display.fillRect(cx + 222, controlsY - 28, 10, 56, INKPLATE_WHITE);

  // ---- Footer ----. Kept for parity with the original sketch, which sets
  // a cursor here but never actually prints anything - i.e. this is a
  // no-op in the original too.
  display.setFont(&FreeSans9pt7b);
  display.setCursor(padX, H - 35);

  display.display();
}
