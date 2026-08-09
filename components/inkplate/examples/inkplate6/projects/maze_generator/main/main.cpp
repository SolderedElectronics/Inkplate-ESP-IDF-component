/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Generates and draws a random maze in 1-bit (BW) mode on
 *              Soldered Inkplate 6.
 *
 * @details     Creates a new random maze on every boot and renders it to the
 *              Inkplate 6 e-paper display. A simple maze generation
 *              algorithm carves passages in a grid of cells, and the
 *              resulting connectivity is drawn as line segments between
 *              adjacent open cells.
 *
 *              The maze is drawn fully into the frame buffer and then shown
 *              on the panel with a full refresh (display.display()). The
 *              generated maze is intended to be a printable/puzzle-style
 *              layout - you can solve it directly on the screen with an
 *              erasable marker or soft pencil, provided you avoid permanent
 *              inks.
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
 * 2) After boot, a maze is generated and displayed.
 * 3) Press reset (or power-cycle) to generate a new random maze.
 * 4) Optionally solve it on the screen using erasable tools.
 *
 * Expected output:
 * - A maze drawn in black lines with an entry and exit opening.
 *
 * Notes:
 * - This example uses 1-bit (black & white) display mode.
 * - This example performs a single full refresh and then stays idle.
 * - cellSize controls maze density; a smaller cellSize increases detail but
 *   may increase generation and drawing time.
 * - If writing on the panel, use only non-permanent tools (whiteboard
 *   marker, soft graphite). Avoid permanent markers to prevent staining.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE6
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate6 in the boards menu."
#endif

#include "Inkplate.h"
#include "esp_random.h"

// Size (in pixels) of one maze cell. Smaller values increase detail but also
// increase generation and drawing time.
static const int cellSize = 20;

// Maze grid dimensions, derived from the panel resolution and the cell size.
// Inkplate 6's 800x600 panel yields a 37x27 cell grid, filling the display
// edge-to-edge with an even 30px margin on all sides.
static const int mazeWidth = (E_INK_WIDTH - 60) / cellSize;
static const int mazeHeight = (E_INK_HEIGHT - 60) / cellSize;

// Maze cell states: 1 = wall, 0 = open passage.
static char maze[mazeWidth * mazeHeight];

// Neighbour offsets, used both while carving passages and while drawing
// them.
static const int kDirX[] = {-1, 0, 0, 1};
static const int kDirY[] = {0, -1, 1, 0};

// Returns a pseudo-random integer in [0, max), using the ESP32 hardware RNG.
// No explicit seeding is required (unlike Arduino's randomSeed()/srand()).
static int randomInt(int max) { return (int)(esp_random() % (uint32_t)max); }

// Carves passages starting from cell (x, y) using a randomized walk that
// only connects cells two steps apart, leaving a wall between unconnected
// cells.
static void carveMaze(char *mazeGrid, int width, int height, int x, int y) {
  int dir = randomInt(4);
  int count = 0;

  while (count < 4) {
    int dx = 0, dy = 0;
    switch (dir) {
    case 0:
      dx = 1;
      break;
    case 1:
      dy = 1;
      break;
    case 2:
      dx = -1;
      break;
    default:
      dy = -1;
      break;
    }

    int x1 = x + dx, y1 = y + dy;
    int x2 = x1 + dx, y2 = y1 + dy;

    if (x2 > 0 && x2 < width && y2 > 0 && y2 < height &&
        mazeGrid[y1 * width + x1] == 1 && mazeGrid[y2 * width + x2] == 1) {
      mazeGrid[y1 * width + x1] = 0;
      mazeGrid[y2 * width + x2] = 0;
      x = x2;
      y = y2;
      dir = randomInt(4);
      count = 0;
    } else {
      dir = (dir + 1) % 4;
      count += 1;
    }
  }
}

// Generates a new random maze into mazeGrid, sized width x height.
static void generateMaze(char *mazeGrid, int width, int height) {
  for (int i = 0; i < width * height; i++) {
    mazeGrid[i] = 1;
  }
  mazeGrid[1 * width + 1] = 0;

  // Carve the maze.
  for (int y = 1; y < height; y += 2) {
    for (int x = 1; x < width; x += 2) {
      carveMaze(mazeGrid, width, height, x, y);
    }
  }

  // Set up the entry (top) and exit (bottom) openings.
  mazeGrid[0 * width + 1] = 0;
  mazeGrid[(height - 1) * width + (width - 2)] = 0;
}

// Draws the maze by connecting adjacent open cells with line segments, then
// pushes the result to the e-paper panel.
static void showMaze(Inkplate &display, const char *mazeGrid, int width,
                      int height) {
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      if (mazeGrid[x + y * width] != 1) {
        continue;
      }
      for (int i = 0; i < 4; ++i) {
        int xx = x + kDirX[i];
        int yy = y + kDirY[i];
        if (xx >= 0 && xx < width && yy >= 0 && yy < height &&
            mazeGrid[yy * width + xx] == 1) {
          display.drawLine(
              3 + x * cellSize + cellSize / 2 + 30,
              3 + y * cellSize + cellSize / 2 + 30,
              3 + x * cellSize + cellSize / 2 + (kDirX[i] * cellSize / 2) + 30,
              3 + y * cellSize + cellSize / 2 + (kDirY[i] * cellSize / 2) + 30,
              BLACK);
        }
      }
    }
  }

  display.display();
}

extern "C" void app_main(void) {
  Inkplate display;

  display.setDisplayMode(BLACK_AND_WHITE);
  display.clearDisplay();

  // Generate and display the maze.
  generateMaze(maze, mazeWidth, mazeHeight);
  showMaze(display, maze, mazeWidth, mazeHeight);
}
