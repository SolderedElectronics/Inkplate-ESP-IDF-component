/**
 * @file        Gui.cpp
 * @brief       Drawing code for the Spotify dashboard screens.
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
  // Inkplate 10 already defaults to 3-bit GRAYSCALE mode, but this sets it
  // explicitly for clarity, matching the original sketch's
  // Inkplate display(INKPLATE_3BIT) constructor argument - both select the
  // same 8-shade (0-7) grayscale mode, just via a different API shape on
  // ESP-IDF vs. Arduino.
  display.setDisplayMode(GRAYSCALE);
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
  // Landscape orientation (native panel is 1200x825) - a single centered
  // line of text doesn't need the portrait layout used below.
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
  // Rotating into portrait turns the native 1200x825 landscape panel into a
  // W=825 x H=1200 canvas below (setRotation(1)/(3) swap width/height on
  // this library, same as the original sketch's setRotation(3) call) - the
  // single centered column of cover art + text + controls needs the extra
  // vertical room portrait gives it.
  display.setRotation(3);
  display.fillRect(0, 0, display.width(), display.height(), 0); // 0 = black
  display.setTextColor(7);                                      // 7 = white

  const int W = display.width();
  const int H = display.height();

  // ---- Cover placement ----
  // Hardcoded cover art: 640x640, centered horizontally. coverY reuses the
  // same (W - coverSize) / 2 formula as coverX (not H) so the top margin
  // matches the side margins, same as the original sketch.
  const int coverSize = 640;
  const int coverX = (W - coverSize) / 2;
  const int coverY = (W - coverSize) / 2;

  // Header area (reserved, not currently used for any text - matches the
  // original sketch, which sets the font/cursor here but never prints).
  display.setFont(&FreeSans9pt7b);
  display.setCursor(80, 55);

  // Cover frame + image
  display.drawRect(coverX - 6, coverY - 6, coverSize + 12, coverSize + 12, 7);

  bool imgOk = display.image.draw(imageUrl, coverX, coverY, true, false);
  if (!imgOk) {
    ESP_LOGW(TAG, "Failed to draw cover image.");
    display.drawRect(coverX, coverY, coverSize, coverSize, 7);
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
  const int yShift = 50;

  // Helper: center + ellipsize to maxTextW at a given baseline Y.
  auto drawCenteredEllipsized = [&](const char *in, const GFXfont *font,
                                    int baselineY) {
    display.setFont(font);

    char s[192];
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

  const int titleY = coverY + coverSize + 85 + yShift;  // bold 24pt
  const int artistY = coverY + coverSize + 135 + yShift; // 18pt
  const int albumY = coverY + coverSize + 170 + yShift;  // 12pt

  drawCenteredEllipsized(title, &FreeSansBold24pt7b, titleY);
  drawCenteredEllipsized(artistName, &FreeSans18pt7b, artistY);
  drawCenteredEllipsized(albumName, &FreeSans12pt7b, albumY);

  // Progress bar under text
  const int barX = padX;
  const int barW = W - (padX * 2);
  const int barH = 10;
  const int barY = coverY + coverSize + 215 + yShift;

  display.drawRect(barX, barY, barW, barH, 7);

  float progress = 0.0f;
  if (durationMs > 0) {
    if (progressMs > durationMs)
      progressMs = durationMs; // clamp
    progress = (float)progressMs / (float)durationMs;
  }

  int fillW = (int)(barW * progress);
  display.fillRect(barX + 1, barY + 1, std::max(0, fillW - 2), barH - 2, 7);

  // Time labels
  display.setFont(&FreeSans9pt7b);
  char leftTime[16];
  char rightTime[16];
  formatMs(progressMs, leftTime, sizeof(leftTime));
  formatMs(durationMs, rightTime, sizeof(rightTime));

  const int timeY = barY + 30;
  display.setCursor(barX, timeY);
  display.print(leftTime);

  {
    int16_t x1, y1;
    uint16_t tw, th;
    display.getTextBounds(rightTime, 0, 0, &x1, &y1, &tw, &th);
    display.setCursor(barX + barW - (int)tw, timeY);
    display.print(rightTime);
  }

  // Controls (previous / play-pause / next icons - static, not interactive:
  // this board has no touch input in this example)
  const int controlsY = barY + 85;
  const int cx = W / 2;

  // Previous
  display.fillTriangle(cx - 220, controlsY, cx - 170, controlsY - 28,
                       cx - 170, controlsY + 28, 7);
  display.fillRect(cx - 232, controlsY - 28, 10, 56, 7);

  // Play (circle + bars, same "pause" icon as the original)
  display.drawCircle(cx, controlsY, 42, 7);
  {
    const int barW2 = 10;
    const int barH2 = 36;
    const int gap = 10;

    int leftBarX = cx - gap / 2 - barW2;
    int rightBarX = cx + gap / 2;
    int barTopY = controlsY - barH2 / 2;

    display.fillRect(leftBarX, barTopY, barW2, barH2, 7);
    display.fillRect(rightBarX, barTopY, barW2, barH2, 7);
  }

  // Next
  display.fillTriangle(cx + 220, controlsY, cx + 170, controlsY - 28,
                       cx + 170, controlsY + 28, 7);
  display.fillRect(cx + 222, controlsY - 28, 10, 56, 7);

  // ---- Footer (reserved, not currently used for any text - matches the
  // original sketch) ----
  display.setFont(&FreeSans9pt7b);
  display.setCursor(padX, H - 35);

  display.display();
}
