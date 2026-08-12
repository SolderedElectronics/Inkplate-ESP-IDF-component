/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Render a Mandelbrot set view on Soldered Inkplate 6 in 1-bit
 *              (BW) mode by computing each pixel on the ESP32.
 *
 * @details     Computes and renders a Mandelbrot set region by iterating the
 *              complex function z = z^2 + c for each pixel on the display.
 *              For every (x, y) coordinate mapped into the complex plane,
 *              the code runs up to MAXITERATIONS iterations and decides
 *              whether the point escapes. Points that do not escape are
 *              drawn as black (inside the set) and others remain white.
 *
 *              Rendering is performed fully in the ESP32 frame buffer and
 *              then pushed to the e-paper panel with a full refresh via
 *              display.display(). Because the computation is heavy and uses
 *              double-precision math, this example can take a long time to
 *              complete on an MCU.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 6
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 6, USB cable
 * - Extra:      None
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate6
 *
 * How to use:
 * 1) Build and flash to Inkplate 6.
 * 2) Wait for the render to complete (may take several minutes).
 * 3) The final Mandelbrot image stays on the display; the example then stays
 *    idle.
 * 4) To explore other regions, change xFrom/xTo/yFrom/yTo below and reflash.
 *
 * Expected output:
 * - A black/white Mandelbrot set rendering of the selected coordinate
 *   window.
 *
 * Notes:
 * - This example uses 1-bit (black & white) display mode.
 * - Rendering is CPU-intensive (nested loops over the full resolution with
 *   iterative complex math). Expect long runtimes and higher power draw
 *   while computing.
 * - MAXITERATIONS controls detail vs. speed. Increasing it improves boundary
 *   detail but increases render time.
 * - Partial updates are not used here; the whole image is generated before
 *   the first refresh.
 * - The Inkplate 6 panel is 800x600 (4:3). The original Arduino sketch's
 *   viewing window was close to square, which would stretch noticeably on
 *   a non-square panel if mapped directly. xFrom/xTo below have been widened
 *   around the same window center used by the original sketch so that
 *   (xTo - xFrom) / (yTo - yFrom) matches E_INK_WIDTH / E_INK_HEIGHT,
 *   keeping the fractal undistorted.
 * - The original Arduino sketch redrew the same static view every 5 seconds
 *   forever. Since the view never changes between redraws, this port
 *   renders it once and stays idle instead of repeating the multi-minute
 *   computation pointlessly.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 */

#include "sdkconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE6
#error                                                                        \
    "Wrong board selection for this example, please select Inkplate6 in the boards menu."
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
// Inkplate 6 panel's 800x600 (4:3) resolution instead of the original
// near-square window, which would otherwise appear vertically stretched.
static const double xFrom = -0.726967, xTo = -0.861633;
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

// Returns BLACK if the point (x, y) in the complex plane stays bounded after
// MAXITERATIONS iterations (i.e. it belongs to the Mandelbrot set), or WHITE
// if it escapes earlier.
static uint8_t colorAt(double x, double y) {
  struct complex z = {0.0, 0.0};
  struct complex c = {x, y};

  int i;
  for (i = 0; i < MAXITERATIONS && modulusComplexSqr(&z) <= 4.0; ++i) {
    squareComplex(&z);
    addComplex(&z, &c);
  }
  return (i == MAXITERATIONS) ? BLACK : WHITE;
}

extern "C" void app_main(void) {
  Inkplate display;

  display.setDisplayMode(BLACK_AND_WHITE);
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
