/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Render a Mandelbrot set view on Soldered Inkplate 6COLOR in
 *              black/white by computing each pixel on the ESP32.
 *
 * @details     Computes and renders a Mandelbrot set region by iterating the
 *              complex function z = z^2 + c for each pixel on the display.
 *              For every (x, y) coordinate mapped into the complex plane,
 *              the code runs up to MAXITERATIONS iterations and decides
 *              whether the point escapes. Points that do not escape are
 *              drawn black (inside the set) and others are drawn white.
 *
 *              Rendering is performed fully in the ESP32 frame buffer and
 *              then pushed to the e-paper panel with a full refresh via
 *              display.display(). Because the computation is heavy and uses
 *              double-precision math, this example can take a long time to
 *              complete on an MCU.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 6COLOR
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 6COLOR, USB cable
 * - Extra:      None
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate6Color
 *
 * How to use:
 * 1) Build and flash to Inkplate 6COLOR.
 * 2) Wait for the render to complete (may take several minutes).
 * 3) The final Mandelbrot image stays on the display; the example then stays
 *    idle.
 * 4) To explore other regions, change xFrom/xTo/yFrom/yTo below and reflash.
 *
 * Expected output:
 * - A black/white Mandelbrot set rendering of the selected coordinate
 *   window on the Inkplate 6COLOR panel.
 *
 * Notes:
 * - The original Inkplate6COLOR Arduino sketch draws in black/white only,
 *   even though the panel is capable of 7 colors (black, white, green,
 *   blue, red, yellow, orange): its colorAt() helper returns a boolean
 *   (i == MAXITERATIONS), not one of the INKPLATE_* color constants. This
 *   port keeps that behavior and maps the boolean result straight to
 *   INKPLATE_BLACK / INKPLATE_WHITE instead of using color bands.
 * - Rendering is CPU-intensive (nested loops over the full resolution with
 *   iterative complex math). Expect long runtimes and higher power draw
 *   while computing.
 * - MAXITERATIONS controls detail vs. speed. Increasing it improves boundary
 *   detail but increases render time.
 * - Partial updates are not used here; the whole image is generated before
 *   the first refresh. Inkplate 6COLOR does not support partial updates
 *   anyway (partialUpdate() is a no-op on this board).
 * - The Inkplate 6COLOR panel is 600x448 (~1.339:1). The original sketch's
 *   viewing window (xFrom/xTo/yFrom/yTo) is close to square, which would
 *   stretch noticeably on a wider panel if mapped directly. Starting from
 *   the original sketch's own window, the vertical span (yFrom/yTo) is kept
 *   unchanged and the horizontal span (xFrom/xTo) is widened around the same
 *   center point (-0.7943) so that (xTo - xFrom) / (yTo - yFrom) matches
 *   E_INK_WIDTH / E_INK_HEIGHT (600 / 448 ~= 1.339286), keeping the fractal
 *   undistorted:
 *     center x = (-0.7423 + -0.8463) / 2 = -0.7943
 *     ySpan    = 0.2102 - 0.1092 = 0.1010            (unchanged)
 *     xSpan    = ySpan * (600 / 448) = 0.135268
 *     xFrom    = center x + xSpan / 2 = -0.726666
 *     xTo      = center x - xSpan / 2 = -0.861934
 * - The pixel-to-complex-plane mapping below divides by E_INK_WIDTH /
 *   E_INK_HEIGHT (600 / 448), this board's actual resolution, rather than
 *   the literal 800.0 / 600.0 divisors used in the original .ino (those
 *   were left over from an 800x600 board and do not match this panel).
 * - The original Arduino sketch redrew the same static view every 5 seconds
 *   forever and printed per-row progress over Serial. Since the view never
 *   changes between redraws, this port renders it once, skips the row
 *   progress printing, and stays idle instead of repeating the multi-minute
 *   computation pointlessly.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 */

#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE6COLOR
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate6Color in the boards menu."
#endif

#include "Inkplate.h"

#define MAXITERATIONS 150

// Explore different positions to draw.
// Some interesting ones can be found here:
// http://www.cuug.ab.ca/dewara/mandelbrot/Mandelbrowser.html
//
// The vertical window (yFrom/yTo) is taken unchanged from the original
// Arduino sketch. The horizontal window (xFrom/xTo) is widened around the
// same center point (-0.7943) so that the window's aspect ratio matches the
// Inkplate 6COLOR panel's 600x448 (~1.339:1) resolution instead of the
// original near-square window, which would otherwise appear stretched.
static const double xFrom = -0.726666, xTo = -0.861934;
static const double yFrom = 0.1092, yTo = 0.2102;

struct complex {
  double re;
  double im;
};

static void addComplex(struct complex *z, struct complex *c) {
  z->re += c->re;
  z->im += c->im;
}

static void squareComplex(struct complex *z) {
  double re = z->re;
  double im = z->im;
  z->re = re * re - im * im;
  z->im = 2 * re * im;
}

static double modulusComplexSqr(struct complex *z) {
  return z->re * z->re + z->im * z->im;
}

// Returns INKPLATE_BLACK if the point (x, y) in the complex plane stays
// bounded after MAXITERATIONS iterations (i.e. it belongs to the Mandelbrot
// set), or INKPLATE_WHITE if it escapes earlier.
static uint8_t colorAt(double x, double y) {
  struct complex z = {0.0, 0.0};
  struct complex c = {x, y};

  int i;
  for (i = 0; i < MAXITERATIONS && modulusComplexSqr(&z) <= 4.0; ++i) {
    squareComplex(&z);
    addComplex(&z, &c);
  }
  return (i == MAXITERATIONS) ? INKPLATE_BLACK : INKPLATE_WHITE;
}

extern "C" void app_main(void) {
  Inkplate display;

  display.clearDisplay();

  for (int j = 0; j < E_INK_HEIGHT; ++j) {
    for (int i = 0; i < E_INK_WIDTH; ++i) {
      display.drawPixel(i, j,
                        colorAt(xFrom + (double)i * (xTo - xFrom) /
                                            (double)E_INK_WIDTH,
                                yFrom + (double)j * (yTo - yFrom) /
                                            (double)E_INK_HEIGHT));
    }
    // For the whole set instead of the zoomed-in region above, use:
    // colorAt(-2.0 + (3.0 * (double)i / (double)E_INK_WIDTH),
    //         -1.0 + 2.0 * (double)j / (double)E_INK_HEIGHT)

    // Yield per row so the idle task can run and feed the task watchdog;
    // without this, the tight per-pixel compute loop starves IDLE0 and
    // task_wdt fires.
    vTaskDelay(1);
  }

  display.display();
}
