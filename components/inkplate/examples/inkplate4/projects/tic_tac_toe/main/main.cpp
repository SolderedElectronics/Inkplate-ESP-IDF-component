/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       Touchscreen Tic-Tac-Toe with optional AI opponent using
 *              partial updates on Soldered Inkplate 4TEMPERA.
 *
 * @details     Demonstrates building an interactive touchscreen UI using the
 *              touchInArea() helper and rendering updates efficiently on
 *              e-paper. The example shows a menu to select the game mode
 *              (human vs AI with multiple difficulty levels, or 2-player),
 *              who plays first, and whether X or O starts.
 *
 *              During gameplay, taps on the 3x3 grid place X or O. In
 *              single-player mode, the AI computes its move using a minimax
 *              search (depth controlled by difficultyDepth[], see ai.h/ai.cpp
 *              for the search itself). The UI is redrawn after each action
 *              and updated primarily via partialUpdate() to reduce flashing.
 *              A "Go Back" touch area returns to the menu and resets the
 *              board.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 4TEMPERA
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 4TEMPERA, USB-C cable
 * - Extra:      None
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate4
 *
 * How to use:
 * 1) Build and flash to Inkplate 4TEMPERA.
 * 2) Use the touchscreen menu to choose difficulty (or 2-player), who
 *    starts, and whether X or O goes first.
 * 3) Tap "Start" to begin.
 * 4) Tap a board cell to place your mark. In AI mode, the device responds
 *    with its move automatically.
 * 5) Tap "Go Back" to return to the menu and reset the board.
 *
 * Expected output:
 * - E-paper: Menu screen with selectable options, followed by a
 *   Tic-Tac-Toe board. The top status line shows whose turn it is and the
 *   game result (ongoing / X won / O won / tie).
 *
 * Notes:
 * - This example uses 1-bit (black & white) display mode; partial updates
 *   are only supported in BLACK_AND_WHITE mode.
 * - Partial updates are used for most UI interactions; a full refresh is
 *   performed when returning to the menu to reduce ghosting.
 * - AI difficulty is controlled by minimax search depth (difficultyDepth[]);
 *   higher depths increase CPU time per move.
 * - This example depends on additional project files:
 *   - ai.h / ai.cpp (minimax implementation)
 *   - generatedUIMenu.h (menu UI geometry/constants and draw routine)
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE4
#error                                                                         \
    "Wrong board selection for this example, please select Inkplate4 in the boards menu."
#endif

#include "Inkplate.h"
#include "ai.h"
#include "generatedUIMenu.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <cstring>

// `display` is NOT a global here: it's constructed as a local in app_main()
// and passed by reference into every function that needs it. A global
// `Inkplate display;` would race the library's own global I2C/PCAL
// peripheral objects (in BoardCommon.cpp) — C++ leaves cross-translation-unit
// static init order unspecified, so the Inkplate ctor can run before the I2C
// bus/expander objects it depends on, leaving the touchscreen controller I2C
// handle uninitialized.

// How far to search for the best move, indexed by difficulty (0=Easy,
// 1=Medium, 2=Hard).
int difficultyDepth[] = {6, 7, 10};

// Global game state.
int difficulty = -1; // 0/1/2 = AI difficulty, 3 = 2-player, -1 = unset.
int firstHuman = -1;  // 0 = computer plays first, 1 = human plays first.
int firstXO = -1;     // 0 = X plays first, 1 = O plays first.

bool menu = true;
bool game = false;
int move = 0;

// Global board array.
char board[3][3] = {
    {'_', '_', '_'},
    {'_', '_', '_'},
    {'_', '_', '_'},
};

// Draws X's and O's to the screen.
void drawBoard(Inkplate &display) {
  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j) {
      int x = 74 + 164 * j, y = 77 + 164 * i;
      if (board[i][j] == 'x') {
        display.drawThickLine(x, y, x + 123, y + 123, BLACK, 4);
        display.drawThickLine(x + 123, y, x, y + 123, BLACK, 4);
      }
      if (board[i][j] == 'o') {
        for (int k = 0; k < 7; ++k)
          display.drawCircle(x + 61, y + 61, 60 - k, BLACK);
      }
    }
}

// Draws game elements to the screen (status line, board, "Go Back" button).
void mainDrawGame(Inkplate &display) {
  // Draw board lines.
  display.drawThickLine(61, 220, 539, 220, BLACK, 5);
  display.drawThickLine(61, 381, 539, 381, BLACK, 5);

  display.drawThickLine(220, 61, 220, 539, BLACK, 5);
  display.drawThickLine(381, 61, 381, 539, BLACK, 5);

  // Draw whose turn it is.
  display.setFont(&FreeSerifBold9pt7b);
  display.setCursor(30, 30);
  display.print((move + firstXO + 1) % 2 ? "X's turn:" : "O's turn:");

  // Draw game state.
  display.setCursor(260, 30);
  switch (result(board)) {
  case 0:
    display.print("Game on!");
    break;
  case 1:
    display.print("X won!");
    break;
  case 2:
    display.print("O won!");
    break;
  case 3:
    display.print("Game is a tie");
    break;
  }

  // Draw "Go Back" button.
  display.setCursor(510, 35);
  display.print("Go Back");
  display.drawRoundRect(500, 10, 90, 40, 5, BLACK);

  drawBoard(display);
}

// Simulates faded text by drawing a white checkerboard pattern over the
// "Computer first" option, used when 2-player mode is selected (that option
// does not apply in 2-player mode).
void crossOutHumanFirst(Inkplate &display) {
  for (int i = rect6_a_y; i < rect6_b_y; ++i)
    for (int j = rect6_a_x + (i % 2); j < rect6_b_x; j += 2)
      display.drawPixel(j, i, WHITE);
}

// Draws the choice radio buttons for the currently selected menu options.
void drawChoices(Inkplate &display) {
  switch (difficulty) {
  case 0:
    display.fillCircle(circle2_center_x, circle2_center_y, 7, BLACK);
    break;
  case 1:
    display.fillCircle(circle3_center_x, circle3_center_y, 7, BLACK);
    break;
  case 2:
    display.fillCircle(circle4_center_x, circle4_center_y, 7, BLACK);
    break;
  case 3:
    display.fillCircle(circle5_center_x, circle5_center_y, 7, BLACK);
    break;
  case 4:
    display.fillCircle(circle6_center_x, circle6_center_y, 7, BLACK);
    break;
  }
  switch (firstHuman) {
  case 0:
    display.fillCircle(circle7_center_x, circle7_center_y, 7, BLACK);
    break;
  case 1:
    display.fillCircle(circle6_center_x, circle6_center_y, 7, BLACK);
    break;
  }
  switch (firstXO) {
  case 0:
    display.fillCircle(circle8_center_x, circle8_center_y, 7, BLACK);
    break;
  case 1:
    display.fillCircle(circle9_center_x, circle9_center_y, 7, BLACK);
    break;
  }
  // If 2-player mode is selected, the "who plays first" option doesn't
  // apply, so cross it out.
  if (difficulty == 3)
    crossOutHumanFirst(display);
}

// Handles all touchscreen events while the menu is shown.
void menuEvents(Inkplate &display) {
  if (display.touchscreen.touchInArea(89, 124, 78, 31)) { // Easy difficulty
    difficulty = 0;
    display.clearDisplay();
    mainDrawMenu(display);
    drawChoices(display);
    display.partialUpdate();
  }

  if (display.touchscreen.touchInArea(89, 204, 112, 31)) { // Medium difficulty
    difficulty = 1;
    display.clearDisplay();
    mainDrawMenu(display);
    drawChoices(display);
    display.partialUpdate();
  }

  if (display.touchscreen.touchInArea(89, 284, 112, 31)) { // Hard difficulty
    difficulty = 2;
    display.clearDisplay();
    mainDrawMenu(display);
    drawChoices(display);
    display.partialUpdate();
  }

  if (display.touchscreen.touchInArea(89, 363, 112, 31)) { // 2-player game
    difficulty = 3;
    display.clearDisplay();
    mainDrawMenu(display);
    drawChoices(display);
    display.partialUpdate();
  }

  if (display.touchscreen.touchInArea(363, 286, 158, 40) &&
      difficulty != 3) { // Computer plays first
    firstHuman = 0;
    display.clearDisplay();
    mainDrawMenu(display);
    drawChoices(display);
    display.partialUpdate();
  }

  if (display.touchscreen.touchInArea(363, 358, 158, 40) &&
      difficulty != 3) { // Human plays first
    firstHuman = 1;
    display.clearDisplay();
    mainDrawMenu(display);
    drawChoices(display);
    display.partialUpdate();
  }

  if (display.touchscreen.touchInArea(364, 125, 155, 40)) { // First player is X
    firstXO = 0;
    display.clearDisplay();
    mainDrawMenu(display);
    drawChoices(display);
    display.partialUpdate();
  }

  if (display.touchscreen.touchInArea(364, 201, 155, 40)) { // First player is O
    firstXO = 1;
    display.clearDisplay();
    mainDrawMenu(display);
    drawChoices(display);
    display.partialUpdate();
  }

  if (display.touchscreen.touchInArea(230, 442, 140, 84)) { // Start game
    // Check if all settings are legal, and if so, start the game.
    if (difficulty == 3 && (firstXO == 0 || firstXO == 1)) {
      menu = false;
      game = true;
      display.clearDisplay();
      mainDrawGame(display);
      display.display();
    } else if (difficulty != -1 && firstXO != -1 && firstHuman != -1) {
      menu = false;
      game = true;
      display.clearDisplay();
      mainDrawGame(display);
      display.display();

      // Make the first move, if the computer is to move first.
      if (difficulty != 3 && firstHuman == 1) {
        struct best bm = minimax((move + firstXO + 1) % 2 ? 'x' : 'o',
                                  (move + firstXO + 1) % 2 ? 'x' : 'o', board,
                                  10);
        board[bm.move / 3][bm.move % 3] = (move + firstXO + 1) % 2 ? 'x' : 'o';
        ++move;

        display.clearDisplay();
        mainDrawGame(display);
        display.partialUpdate();
      }
    } else { // Not all settings are legal.
      text13_content = "Please select all options!";
      display.clearDisplay();
      mainDrawMenu(display);
      drawChoices(display);
      display.partialUpdate();
      text13_content = "";
    }
  }
}

// Handles all touchscreen events during a normal game.
void gameEvents(Inkplate &display) {
  if (display.touchscreen.touchInArea(510, 0, 40, 100)) { // Go back
    memset(board, '_', sizeof board);
    menu = true;
    game = false;
    move = 0;
    display.clearDisplay();
    mainDrawMenu(display);
    drawChoices(display);
    display.display();
  }

  // Check if any board field was pressed.
  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
      if (display.touchscreen.touchInArea(61 + 155 * j, 51 + 155 * i, 155,
                                           155)) {
        // If the field is already taken, or the game is over, skip it.
        if (board[i][j] != '_' || result(board) != 0)
          continue;
        board[i][j] = (move + firstXO + 1) % 2 ? 'x' : 'o';

        ++move;
        display.clearDisplay();
        mainDrawGame(display);
        display.partialUpdate();

        // If the game is against the computer, let it make a move.
        if (result(board) == 0 && difficulty != 3) {
          vTaskDelay(pdMS_TO_TICKS(1000));
          struct best bm =
              minimax((move + firstXO + 1) % 2 ? 'x' : 'o',
                      (move + firstXO + 1) % 2 ? 'x' : 'o', board,
                      difficultyDepth[difficulty]);
          board[bm.move / 3][bm.move % 3] =
              (move + firstXO + 1) % 2 ? 'x' : 'o';

          // Draw the board again.
          ++move;
          display.clearDisplay();
          mainDrawGame(display);
          display.partialUpdate();
        }
      }
}

extern "C" void app_main(void) {
  Inkplate display;

  // Partial updates are only supported in BLACK_AND_WHITE mode.
  display.setDisplayMode(BLACK_AND_WHITE);

  display.clearDisplay();
  mainDrawMenu(display);
  display.display();

  while (true) {
    if (menu)
      menuEvents(display);
    else if (game)
      gameEvents(display);

    vTaskDelay(pdMS_TO_TICKS(15));
  }
}
