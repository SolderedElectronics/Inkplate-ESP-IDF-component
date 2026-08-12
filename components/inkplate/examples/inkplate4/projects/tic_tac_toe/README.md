# Tic Tac Toe

Touchscreen Tic-Tac-Toe with an optional AI opponent for Soldered Inkplate 4TEMPERA.

## Overview

Implements a full touchscreen Tic-Tac-Toe game on the Inkplate 4TEMPERA e-paper display. A menu screen lets you pick the game mode (play against the AI at Easy/Medium/Hard difficulty, or 2-player), choose who plays first, and choose whether X or O starts. During gameplay, tapping a cell on the 3x3 grid places your mark; in single-player mode the AI answers automatically using a minimax search (`ai.h`/`ai.cpp`), with difficulty controlled by search depth. The UI is redrawn using `partialUpdate()` after most interactions to keep the screen responsive, and a "Go Back" touch area returns to the menu and resets the board. The menu layout and drawing routine (`generatedUIMenu.h`) were exported from Soldered's UI generator tool and are used as-is.

## Hardware Required

- Soldered Inkplate 4TEMPERA
- USB-C cable

## Setup

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate4**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

A menu screen with selectable difficulty/mode/turn-order options and a "Start" button. After starting, a Tic-Tac-Toe board is shown with a status line indicating whose turn it is and the game result (ongoing / X won / O won / tie). Tapping a cell places a mark; in single-player mode the AI responds with its own move shortly after.

## Notes

- This example uses 1-bit (black & white) display mode; partial updates are only supported in `BLACK_AND_WHITE` mode.
- Partial updates are used for most UI interactions; a full refresh is performed when the game starts and when returning to the menu, to reduce ghosting.
- AI difficulty is controlled by minimax search depth (`difficultyDepth[]` in `main.cpp`); higher depths increase CPU time per move.
- The minimax search and board-result logic live in `ai.h`/`ai.cpp`; the menu geometry and draw routine live in `generatedUIMenu.h`. Both were carried over from the original sketch unchanged.
- The touchscreen is initialized automatically as part of constructing `Inkplate display;` — no explicit `touchscreen.init()` call is needed on this port.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
