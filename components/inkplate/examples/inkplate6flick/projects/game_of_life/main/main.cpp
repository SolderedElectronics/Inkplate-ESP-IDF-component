/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Conway's Game of Life animation using partial updates for
 *              Soldered Inkplate 6 Flick.
 *
 * @details     Ports the community "Game of Life" example to ESP-IDF. The
 *              screen is divided into a grid of square cells (randomized cell
 *              size each run) and every generation updates the grid according
 *              to the classic Life rules. Only the cells that changed state
 *              are redrawn into the frame buffer, and the sketch uses
 *              partialUpdate() for most frames to keep the animation smooth on
 *              e-paper. A full refresh is performed periodically to reduce
 *              ghosting, and the grid is re-randomized automatically if the
 *              simulation stagnates (too little change over time).
 *
 * Requirements:
 * - Board:      Soldered Inkplate 6 Flick
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 6 Flick, USB cable
 * - Extra:      None
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate6 Flick
 *
 * How to use:
 * 1) Build and flash to Inkplate 6 Flick.
 * 2) The Game of Life simulation starts automatically after boot.
 * 3) Watch the evolving patterns; the sketch will occasionally re-randomize
 *    the grid when activity drops.
 *
 * Expected output:
 * - E-paper: Continuous Game of Life animation using black/white cells. New
 *   cells appear as filled black squares; older cells are drawn with a
 *   shrinking white interior to indicate age.
 *
 * Notes:
 * - This example uses 1-bit (black & white) display mode; partial updates are
 *   only supported in BLACK_AND_WHITE mode.
 * - A full refresh is performed every FULLREFRESH frames to reduce ghosting.
 * - The simulation uses two in-RAM grids sized for the minimum cell size; RAM
 *   usage increases with display resolution and the chosen cell size range.
 *   On the Inkplate 6 Flick's 1024x758 panel these grids are modestly sized
 *   and comfortably fit in RAM.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE6FLICK
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate6 Flick in the boards menu."
#endif

#include "Inkplate.h"
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Helper that mimics Arduino's random(min, max) using esp_random(). Returns
// an inclusive value in [min, max].
static int random(int min, int max) {
  return (esp_random() % (max - min + 1)) + min;
}

Inkplate display;

// Update the whole screen every FULLREFRESH partial updates to reduce
// ghosting on the e-paper panel.
#define FULLREFRESH 40

#define SCREEN_WIDTH E_INK_WIDTH
#define SCREEN_HEIGHT E_INK_HEIGHT

#define MIN_CELLSZ 8
#define MAX_CELLSZ 40

// Current cell/grid geometry, recomputed every time randomize() runs.
int cell_size = 0, cols = 0, rows = 0, cell_maxage = 0, offset_x = 0,
    offset_y = 0;

// Two grids sized for the smallest possible cell (i.e. the largest possible
// grid), so any randomly chosen cell size fits without reallocating. On the
// Inkplate 6 Flick's 1024x758 panel these grids are modestly sized and
// comfortably fit in RAM.
char grid_a[(SCREEN_WIDTH / MIN_CELLSZ) * (SCREEN_HEIGHT / MIN_CELLSZ)],
    grid_b[(SCREEN_WIDTH / MIN_CELLSZ) * (SCREEN_HEIGHT / MIN_CELLSZ)];

char *grid_curr = grid_a, *grid_next = grid_b, *grid_tmp = NULL, cell_curr,
     cell_next;

int dx = 0, dy = 0, nx = 0, ny = 0, neighbors = 0, cell_delta = 0,
    frame_count = 0;

// (Re)initializes the grid geometry with a random cell size and seeds a new
// random starting population. Also called automatically whenever the
// simulation stagnates.
void randomize() {
  cell_size = random(MIN_CELLSZ, MAX_CELLSZ);

  // Compute the (rounded-down) number of rows and columns
  cols = SCREEN_WIDTH / cell_size;
  rows = SCREEN_HEIGHT / cell_size;

  cell_maxage = (cell_size / 2) - 1;
  // Compute the "extra" space not covered by this grid, offset by half
  offset_x = (SCREEN_WIDTH - cols * cell_size) / 2;
  offset_y = (SCREEN_HEIGHT - rows * cell_size) / 2;

  // Push the last frame of the previous run to the panel with a full
  // refresh before clearing the frame buffer for the new grid.
  display.display();

  display.clearDisplay();

  // Compute a random density...
  int density = random(5, 14);

  // And for that density, populate the initial grid
  for (int j = 0; j < cols; j++) {
    for (int i = 0; i < rows; i++)
      grid_curr[j + i * cols] = random(0, density - 1) == 0;
  }

  frame_count = 0;
}

// Advances the simulation by one generation: counts neighbors for every
// cell, applies the classic Life rules, draws only the cells that changed
// state directly into the frame buffer, and swaps the current/next grids.
// Note: this example draws while it computes (there is no separate
// offscreen model), matching the original sketch's single-pass design.
void stepGeneration() {
  cell_delta = 0;
  for (int j = 0; j < cols; j++) {
    for (int i = 0; i < rows; i++) {
      // Count neighboring cells (grid wraps around at the edges)
      neighbors = 0;
      for (dx = -1; dx < 2; dx++) {
        for (dy = -1; dy < 2; dy++) {
          if (dx == 0 && dy == 0)
            continue; // Skip "me"

          nx = j + dx;
          if (nx < 0)
            nx = cols - 1;
          else if (nx >= cols)
            nx = 0;
          ny = i + dy;
          if (ny < 0)
            ny = rows - 1;
          else if (ny >= rows)
            ny = 0;

          if (grid_curr[nx + ny * cols])
            neighbors++;
        }
      }

      cell_curr = grid_curr[j + i * cols];
      cell_next = 0;
      switch (neighbors) {
      case 2: // Alive with 2 neighbors remains alive
        if (!cell_curr)
          break;
        // Else cell is alive, drop through
      case 3:                     // 3 neighbors == alive
        cell_next = cell_curr + 1;
        if (cell_next > cell_maxage)
          cell_next = cell_maxage;
      }

      if ((cell_next != 0 && cell_curr == 0) ||
          (cell_next == 0 && cell_curr != 0))
        cell_delta++;

      if (cell_next) {
        // If this is a new cell, paint it black
        if (cell_next == 1)
          display.fillRect(j * cell_size + offset_x, i * cell_size + offset_y,
                            cell_size, cell_size, BLACK);
        // Otherwise paint the inside white depending on how old it is
        else
          display.fillRect(j * cell_size + cell_size / 2 - cell_next + offset_x,
                            i * cell_size + cell_size / 2 - cell_next + offset_y,
                            cell_next * 2, cell_next * 2, WHITE);
      } else if (cell_curr) {
        // Otherwise it's died, paint the whole cell white
        display.fillRect(j * cell_size + offset_x, i * cell_size + offset_y,
                          cell_size, cell_size, WHITE);
      }

      grid_next[j + i * cols] = cell_next;
    }
  }
  // Swap which grid is current
  grid_tmp = grid_next;
  grid_next = grid_curr;
  grid_curr = grid_tmp;

  // The longer this goes, the more cells this has,
  // the more change is required or we reset
  if (cell_delta * cell_size < frame_count)
    randomize();
  else
    frame_count++;
}

extern "C" void app_main(void) {
  // Partial updates are only supported in BLACK_AND_WHITE mode.
  display.setDisplayMode(BLACK_AND_WHITE);

  randomize();

  while (true) {
    stepGeneration();

    // Update the whole screen after FULLREFRESH partials to reduce ghosting,
    // otherwise use a fast partial update to keep the animation smooth.
    if (frame_count % FULLREFRESH == 0)
      display.display();
    else
      display.partialUpdate();

    vTaskDelay(pdMS_TO_TICKS(50)); // Small delay between generations.
  }
}
