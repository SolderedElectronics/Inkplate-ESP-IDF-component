/**
 * @file        main.cpp
 * @author      Fran Fodor for Soldered
 * @brief       SD-card e-reader UI for Soldered Inkplate 6 Flick that displays
 *              a preprocessed EPUB as a sequence of page images.
 *
 * @details     Implements a simple, open-source eBook reader for Inkplate 6
 *              Flick. Instead of parsing EPUB files on-device, a companion PC
 *              tool (epubToImg.py, see below) converts an .epub into a
 *              sequence of page images sized for the UI, which are then
 *              copied onto the SD card under /books/<book_name>/.
 *
 *              On boot, this example scans /books/ on the SD card for
 *              subfolders (books), shows a touchscreen list, and lets you
 *              pick one. Pages are loaded from the SD card and rendered with
 *              the Inkplate image drawer. Navigation includes PREV/NEXT
 *              (book list and page turning), HOME (back to the book list),
 *              and a GOTO overlay with an on-screen numeric keypad to jump
 *              directly to a page number.
 *
 *              The display runs in 1-bit (black & white) mode and uses
 *              partial updates for responsive touch interaction.
 *              setFullUpdateThreshold() triggers an automatic full refresh
 *              after a configured number of partial updates to help reduce
 *              ghosting.
 *
 * Requirements:
 * - Board:      Soldered Inkplate 6 Flick
 * - Framework:  ESP-IDF v6.x
 * - Hardware:   Inkplate 6 Flick, microSD card, USB cable
 * - Extra:      A PC with Python 3 to pre-render an EPUB into page images
 *               (see "PC-side EPUB preprocessing" below)
 *
 * Configuration:
 * - Menuconfig -> Inkplate Boards -> Inkplate6 Flick
 * - SD card format: FAT / FAT32
 * - SD card content:
 *   - /books/<book_name>/ must contain page images (BMP/JPG/JPEG/PNG), one
 *     file per page, named so that they sort in reading order (e.g. 0001.png,
 *     0002.png, ...). Filenames are sorted with a "natural" numeric sort, so
 *     plain numeric names like 2.bmp / 10.bmp also sort correctly.
 *   - Every page image should be 758x930 pixels (see "PC-side EPUB
 *     preprocessing" below) — this is the size the UI is laid out for.
 *
 * PC-side EPUB preprocessing:
 * - The companion Python tool epubToImg/epubToImg.py (shipped alongside the
 *   original Arduino example this was ported from) is a HOST-side tool: it
 *   runs on a PC, not on the Inkplate, and turns an .epub file into a folder
 *   of page images. It is not part of this ESP-IDF project and is not built
 *   or run on the device — copy it to your PC if you need to prepare book
 *   images.
 * - PC setup:
 *   1) Install Python 3 and the tool's dependencies:
 *      pip install -r requirements.txt
 *      (the script also relies on Playwright's Chromium browser; after
 *      installing dependencies run `playwright install chromium` once)
 *   2) Run it against an EPUB, writing into an empty output folder:
 *      python epubToImg.py mybook.epub ./output --width 758 --height 930
 *   3) Copy the resulting output folder onto the SD card as
 *      /books/<book_name>/ (e.g. /books/MyBook/).
 *
 * How to use:
 * 1) Prepare the SD card with a /books/ folder containing one subfolder per
 *    book, each filled with page images as described above.
 * 2) Insert the SD card into Inkplate 6 Flick and flash this example.
 * 3) Tap PREV/NEXT to browse the book list, then tap SELECT to open the
 *    highlighted book.
 * 4) In page view, tap PREV/NEXT to turn pages, HOME to return to the book
 *    list, or GOTO to open the on-screen numeric keypad and jump to a page.
 *
 * Expected output:
 * - Display: a book-list UI (from /books/), then full-page images with
 *   PREV/NEXT/HOME/GOTO buttons and a page counter ("current / total").
 * - Log output: SD card / "no books found" errors are logged with ESP_LOGE
 *   and also shown on the e-paper screen.
 *
 * Notes:
 * - Display mode is 1-bit (black & white); partial updates are only
 *   supported in that mode.
 * - Page images are decoded from the SD card on every page turn; larger
 *   images and some formats decode more slowly. Prefer consistently sized
 *   758x930 pages.
 * - This example is fully interactive and does not use deep sleep.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 * Image tool:   https://tools.soldered.com/tools/image-converter/
 */

#include "sdkconfig.h"

#ifndef CONFIG_INKPLATE_BOARD_INKPLATE6FLICK
#error                                                                        \
    "Wrong board selection for this example, please select Inkplate6 Flick in the boards menu."
#endif

#include "Inkplate.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <string>
#include <strings.h>
#include <vector>

static const char *TAG = "E_READER";

// Global display object (matches the pattern used by other ported examples
// for this board, e.g. projects/calculator).
Inkplate display;

// Folder on the SD card root that holds one subfolder per book.
#define BOOKS_FOLDER_NAME "books"

// Number of partial updates allowed before Inkplate forces a full refresh
// (mirrors the original sketch's `partialUpdate = 3` + setFullUpdateThreshold
// call). The threshold is enforced internally by the Inkplate component, so
// no manual counter is needed in this file.
#define FULL_UPDATE_THRESHOLD 3

// Debounce delay applied after every touch handled, to avoid a single
// physical tap being registered multiple times.
#define TAP_DEBOUNCE_MS 200

// Maximum digits accepted in the GOTO page-number keypad.
#define PAGE_INPUT_MAX_DIGITS 5

// --------------------------------------------------------------------------
// UI layout (identical geometry to the original Arduino sketch)
// --------------------------------------------------------------------------
#define MARGIN_X 10
#define MARGIN_Y 30
#define HEADER_MARGIN 50

#define BUTTON_Y 940
#define BUTTON_H 60
#define BUTTON_W 150

// Main (book list) view buttons.
#define LEFT_BTN_X ((display.width() / 4) - (BUTTON_W / 2))
#define MIDDLE_BTN_X ((display.width() * 3 / 4) - MARGIN_X - BUTTON_W)
#define RIGHT_BTN_X ((display.width() * 3 / 4) + MARGIN_X)

// Picture (page) view buttons.
#define HOME_BTN_X ((display.width() * 1 / 8) - (BUTTON_W / 2))
#define GOTO_BTN_X ((display.width() * 3 / 8) - (BUTTON_W / 2))
#define PREV_BTN_X ((display.width() * 5 / 8) - (BUTTON_W / 2))
#define NEXT_BTN_X ((display.width() * 7 / 8) - (BUTTON_W / 2))

// --------------------------------------------------------------------------
// State
// --------------------------------------------------------------------------

// Book/page names are kept as plain vectors of std::string instead of the
// original sketch's hand-rolled doubly-linked lists (Book/Picture structs) —
// see the README for why this was simplified.
static std::vector<std::string> bookNames;
static std::vector<std::string> pageNames;

static int currentBookIndex = 0;
static int currentPageIndex = 0;
static int firstVisibleBookIndex = 0;
static int visibleRows = 0;

static bool inPictureView = false;
static bool inGotoUI = false;

// Digits typed so far in the GOTO keypad overlay.
static std::string pageInput;

// --------------------------------------------------------------------------
// Function prototypes
// --------------------------------------------------------------------------
static bool isImageFile(const char *name);
static bool naturalLess(const std::string &a, const std::string &b);
static void listSubdirectories(const char *path);
static void listPictures(const char *folderPath);

static void displayMainPage();
static void displayButtons();
static void displayPicture();
static void displayPictureButtons();
static void invertButton(int x, int y, int w, int h, const char *label);
static void displayPageCounter();
static void displayGotoUI();

static void handleMainViewTouch();
static void handlePictureViewTouch();
static void handleTouch();

// --------------------------------------------------------------------------
// Filename helpers
// --------------------------------------------------------------------------

// Returns true if name ends in a supported page-image extension.
static bool isImageFile(const char *name) {
  const char *dot = strrchr(name, '.');
  if (!dot)
    return false;
  return strcasecmp(dot, ".bmp") == 0 || strcasecmp(dot, ".jpg") == 0 ||
         strcasecmp(dot, ".jpeg") == 0 || strcasecmp(dot, ".png") == 0;
}

// "Natural" numeric comparator so filenames like "2.bmp" sort before
// "10.bmp" instead of using plain lexicographic order. If either name
// doesn't start with a number, falls back to a plain string comparison.
static bool naturalLess(const std::string &a, const std::string &b) {
  char *aEnd = nullptr, *bEnd = nullptr;
  long aNum = strtol(a.c_str(), &aEnd, 10);
  long bNum = strtol(b.c_str(), &bEnd, 10);
  if (aEnd != a.c_str() && bEnd != b.c_str()) {
    if (aNum != bNum)
      return aNum < bNum;
  }
  return a < b;
}

// --------------------------------------------------------------------------
// SD card folder scanning
//
// The original Arduino sketch used SdFat's File::openNextFile()/isDirectory()
// to walk folders. On this ESP-IDF port the SD card is mounted as a regular
// FATFS volume (see display.getMountPoint()), so folders are walked with the
// POSIX opendir()/readdir() VFS API instead (the same technique used by the
// inkplate4 image_frame_gesture example).
// --------------------------------------------------------------------------

// Scans `path` (an absolute path including the SD mount point) for
// subdirectories and fills bookNames with their names, in whatever order
// the filesystem returns them (matches the original sketch, which also did
// not sort the book list).
static void listSubdirectories(const char *path) {
  bookNames.clear();

  DIR *dir = opendir(path);
  if (!dir) {
    ESP_LOGE(TAG, "Failed to open folder: %s", path);
    return;
  }

  struct dirent *entry;
  while ((entry = readdir(dir)) != nullptr) {
    if (entry->d_name[0] == '.')
      continue; // skip ".", "..", and hidden entries
    if (entry->d_type == DT_DIR)
      bookNames.emplace_back(entry->d_name);
  }
  closedir(dir);
}

// Scans `folderPath` (an absolute path including the SD mount point) for
// supported page image files, fills pageNames, and sorts them in natural
// numeric order so pages display in reading order.
static void listPictures(const char *folderPath) {
  pageNames.clear();

  DIR *dir = opendir(folderPath);
  if (!dir) {
    ESP_LOGE(TAG, "Failed to open folder: %s", folderPath);
    return;
  }

  struct dirent *entry;
  while ((entry = readdir(dir)) != nullptr) {
    if (entry->d_name[0] == '.')
      continue;
    if (entry->d_type != DT_REG)
      continue;
    if (!isImageFile(entry->d_name))
      continue;
    pageNames.emplace_back(entry->d_name);
  }
  closedir(dir);

  std::sort(pageNames.begin(), pageNames.end(), naturalLess);
}

// --------------------------------------------------------------------------
// Drawing
// --------------------------------------------------------------------------

static void displayMainPage() {
  display.clearDisplay();
  display.setTextSize(3);
  display.setTextColor(BLACK);
  display.setCursor(MARGIN_X, MARGIN_Y);
  display.println("Home - Select a book");

  int y = MARGIN_Y + HEADER_MARGIN;
  int listH = visibleRows * MARGIN_Y;
  display.fillRect(MARGIN_X, y, display.width() - 2 * MARGIN_X, listH, WHITE);

  display.setTextSize(2);
  for (int row = 0; row < visibleRows; row++) {
    int idx = firstVisibleBookIndex + row;
    if (idx >= (int)bookNames.size())
      break;

    if (idx == currentBookIndex) {
      display.drawRect(MARGIN_X, y + row * MARGIN_Y,
                       display.width() - 2 * MARGIN_X, MARGIN_Y, BLACK);
    }

    display.setCursor(MARGIN_X + 4, y + row * MARGIN_Y + (MARGIN_Y / 4));
    display.setTextColor(BLACK);
    display.println(bookNames[idx].c_str());
  }

  displayButtons();
  display.partialUpdate(false, true);
}

static void displayButtons() {
  const char *labels[3] = {"SELECT", "PREV", "NEXT"};
  const int xs[3] = {LEFT_BTN_X, MIDDLE_BTN_X, RIGHT_BTN_X};

  display.setTextSize(2);
  display.setTextColor(BLACK);
  for (int i = 0; i < 3; i++) {
    int x = xs[i], y = BUTTON_Y;
    display.drawRect(x, y, BUTTON_W, BUTTON_H, BLACK);
    int textW = strlen(labels[i]) * 6 * 2;
    int textH = 8 * 2;
    int16_t tx = x + (BUTTON_W - textW) / 2;
    int16_t ty = y + (BUTTON_H - textH) / 2;
    display.setCursor(tx, ty);
    display.print(labels[i]);
  }
}

static void displayPicture() {
  display.clearDisplay();

  // Full path includes the SD mount point explicitly (e.g.
  // "/sdcard/books/MyBook/0001.png"); Image::draw() uses paths starting
  // with '/' verbatim instead of prepending the mount point itself.
  char fullPath[256];
  snprintf(fullPath, sizeof(fullPath), "%s/%s/%s/%s", display.getMountPoint(),
           BOOKS_FOLDER_NAME, bookNames[currentBookIndex].c_str(),
           pageNames[currentPageIndex].c_str());

  // x=0, y=11 (moved down by 10px, as in the original sketch), dithered.
  if (!display.image.draw(fullPath, 0, 11, true))
    ESP_LOGE(TAG, "Failed to draw page image: %s", fullPath);

  displayPictureButtons();
  displayPageCounter();
  display.partialUpdate(false, true);
}

static void displayPictureButtons() {
  const char *labels[4] = {"HOME", "GOTO", "PREV", "NEXT"};
  const int xs[4] = {HOME_BTN_X, GOTO_BTN_X, PREV_BTN_X, NEXT_BTN_X};

  display.setTextSize(2);
  display.setTextColor(BLACK);
  for (int i = 0; i < 4; i++) {
    int x = xs[i], y = BUTTON_Y;
    display.drawRect(x, y, BUTTON_W, BUTTON_H, BLACK);
    int textW = strlen(labels[i]) * 6 * 2;
    int textH = 8 * 2;
    int16_t tx = x + (BUTTON_W - textW) / 2;
    int16_t ty = y + (BUTTON_H - textH) / 2;
    display.setCursor(tx, ty);
    display.print(labels[i]);
  }
}

// Draws a button inverted (filled black, white outline/text) as immediate
// tap feedback, then pushes it with a fast partial update. The caller is
// still responsible for performing the actual action and redrawing.
static void invertButton(int x, int y, int w, int h, const char *label) {
  display.fillRect(x, y, w, h, BLACK);
  display.drawRect(x, y, w, h, WHITE);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  int textW = strlen(label) * 6 * 2;
  int textH = 8 * 2;
  int16_t tx = x + (w - textW) / 2;
  int16_t ty = y + (h - textH) / 2;
  display.setCursor(tx, ty);
  display.print(label);
  display.partialUpdate(false, true);
}

static void displayPageCounter() {
  char buf[32];
  snprintf(buf, sizeof(buf), "%d / %d", currentPageIndex + 1,
           (int)pageNames.size());

  display.setTextSize(2);
  display.setTextColor(BLACK);
  int textW = strlen(buf) * 6 * 2;
  int textH = 8 * 2;
  int16_t tx = (display.width() - textW) / 2;
  int16_t ty = BUTTON_Y + BUTTON_H + 5;
  display.setCursor(tx, ty);
  display.print(buf);
}

static void displayGotoUI() {
  // Hide the underlying picture-view buttons.
  display.fillRect(0, BUTTON_Y, display.width(), BUTTON_H, WHITE);

  // Draw the BACK button in place of HOME.
  display.setTextSize(2);
  display.setTextColor(BLACK);
  display.drawRect(HOME_BTN_X, BUTTON_Y, BUTTON_W, BUTTON_H, BLACK);
  const char *backLbl = "BACK";
  int bw = strlen(backLbl) * 6 * 2;
  int bh = 8 * 2;
  int16_t bx = HOME_BTN_X + (BUTTON_W - bw) / 2;
  int16_t by = BUTTON_Y + (BUTTON_H - bh) / 2;
  display.setCursor(bx, by);
  display.print(backLbl);

  // Keypad overlay panel.
  int ovX = MARGIN_X, ovY = MARGIN_Y;
  int ovW = display.width() - 2 * MARGIN_X;
  int ovH = BUTTON_Y - 2 * MARGIN_Y;
  display.fillRect(ovX, ovY, ovW, ovH, WHITE);
  display.drawRect(ovX, ovY, ovW, ovH, BLACK);

  // Prompt showing digits typed so far.
  char buf[48];
  snprintf(buf, sizeof(buf), "Go to the page: %s", pageInput.c_str());
  display.setTextSize(2);
  display.setTextColor(BLACK);
  int labelW = strlen(buf) * 6 * 2;
  int16_t lx = (display.width() - labelW) / 2;
  display.setCursor(lx, ovY + 10);
  display.print(buf);

  // 4x3 numeric keypad (1-9, CLR, 0, OK).
  static const char *keys[4][3] = {
      {"1", "2", "3"}, {"4", "5", "6"}, {"7", "8", "9"}, {"CLR", "0", "OK"}};
  int spacing = 10, headerH = 40;
  int totalW = ovW, totalH = ovH - headerH;
  int btnW = (totalW - spacing * 2) / 3;
  int btnH = (totalH - spacing * 3) / 4;
  int startY = ovY + headerH;
  for (int r = 0; r < 4; r++) {
    for (int c = 0; c < 3; c++) {
      int x = ovX + c * (btnW + spacing);
      int y = startY + r * (btnH + spacing);
      display.drawRect(x, y, btnW, btnH, BLACK);
      int len = strlen(keys[r][c]);
      int textW = len * 6 * 2;
      int textH = 8 * 2;
      int16_t tx = x + (btnW - textW) / 2;
      int16_t ty = y + (btnH - textH) / 2;
      display.setCursor(tx, ty);
      display.print(keys[r][c]);
    }
  }

  display.partialUpdate(false, true);
}

// --------------------------------------------------------------------------
// Touch handling
//
// touchscreen.touchInArea(x, y, w, h) polls the touch controller and does
// the hit-test in one call (confirmed against
// basic/touch_in_area/main/main.cpp and TouchCypress::touchInArea()), so
// there is no separate "read touch point, then hit-test" step — each button
// is simply one touchInArea() check. The touchscreen itself is initialised
// automatically while constructing the global `display` object; no explicit
// touchscreen.init()/begin() call is needed on this board.
// --------------------------------------------------------------------------

static void handleMainViewTouch() {
  // LEFT = SELECT -> open the highlighted book.
  if (display.touchscreen.touchInArea(LEFT_BTN_X, BUTTON_Y, BUTTON_W,
                                      BUTTON_H)) {
    invertButton(LEFT_BTN_X, BUTTON_Y, BUTTON_W, BUTTON_H, "SELECT");
    if (!bookNames.empty()) {
      char folder[192];
      snprintf(folder, sizeof(folder), "%s/%s/%s", display.getMountPoint(),
               BOOKS_FOLDER_NAME, bookNames[currentBookIndex].c_str());
      listPictures(folder);
      if (!pageNames.empty()) {
        inPictureView = true;
        currentPageIndex = 0;
        displayPicture();
      } else {
        ESP_LOGW(TAG, "No images in folder: %s", folder);
      }
    }
    vTaskDelay(pdMS_TO_TICKS(TAP_DEBOUNCE_MS));
    return;
  }

  // MIDDLE = PREV book (cyclic if more than one book exists).
  if (display.touchscreen.touchInArea(MIDDLE_BTN_X, BUTTON_Y, BUTTON_W,
                                      BUTTON_H)) {
    invertButton(MIDDLE_BTN_X, BUTTON_Y, BUTTON_W, BUTTON_H, "PREV");
    if (bookNames.size() > 1) {
      currentBookIndex = (currentBookIndex == 0) ? (int)bookNames.size() - 1
                                                   : currentBookIndex - 1;
      if (currentBookIndex < firstVisibleBookIndex)
        firstVisibleBookIndex = currentBookIndex;
      else if (currentBookIndex >= firstVisibleBookIndex + visibleRows)
        firstVisibleBookIndex = currentBookIndex - visibleRows + 1;
      displayMainPage();
    }
    vTaskDelay(pdMS_TO_TICKS(TAP_DEBOUNCE_MS));
    return;
  }

  // RIGHT = NEXT book (cyclic if more than one book exists).
  if (display.touchscreen.touchInArea(RIGHT_BTN_X, BUTTON_Y, BUTTON_W,
                                      BUTTON_H)) {
    invertButton(RIGHT_BTN_X, BUTTON_Y, BUTTON_W, BUTTON_H, "NEXT");
    if (bookNames.size() > 1) {
      currentBookIndex = (currentBookIndex + 1) % (int)bookNames.size();
      if (currentBookIndex >= firstVisibleBookIndex + visibleRows)
        firstVisibleBookIndex = currentBookIndex - visibleRows + 1;
      else if (currentBookIndex < firstVisibleBookIndex)
        firstVisibleBookIndex = currentBookIndex;
      displayMainPage();
    }
    vTaskDelay(pdMS_TO_TICKS(TAP_DEBOUNCE_MS));
    return;
  }
}

static void handlePictureViewTouch() {
  if (inGotoUI) {
    static const char *keys[4][3] = {
        {"1", "2", "3"}, {"4", "5", "6"}, {"7", "8", "9"}, {"CLR", "0", "OK"}};
    const int spacing = 10, headerH = 40;
    const int ovX = MARGIN_X, ovY = MARGIN_Y;
    const int ovW = display.width() - 2 * MARGIN_X;
    const int ovH = BUTTON_Y - 2 * MARGIN_Y;
    const int totalW = ovW, totalH = ovH - headerH;
    const int btnW = (totalW - spacing * 2) / 3;
    const int btnH = (totalH - spacing * 3) / 4;
    const int startY = ovY + headerH;

    for (int r = 0; r < 4; r++) {
      for (int c = 0; c < 3; c++) {
        int x = ovX + c * (btnW + spacing);
        int y = startY + r * (btnH + spacing);
        if (display.touchscreen.touchInArea(x, y, btnW, btnH)) {
          invertButton(x, y, btnW, btnH, keys[r][c]);

          if (strcmp(keys[r][c], "CLR") == 0) {
            pageInput.clear();
            displayGotoUI();
          } else if (strcmp(keys[r][c], "OK") == 0) {
            int p = pageInput.empty() ? 1 : atoi(pageInput.c_str());
            if (p < 1)
              p = 1;
            else if (p > (int)pageNames.size())
              p = (int)pageNames.size();
            currentPageIndex = p - 1;
            inGotoUI = false;
            displayPicture();
          } else {
            if (pageInput.length() < PAGE_INPUT_MAX_DIGITS)
              pageInput += keys[r][c];
            displayGotoUI();
          }

          vTaskDelay(pdMS_TO_TICKS(TAP_DEBOUNCE_MS));
          return;
        }
      }
    }
  }

  // HOME/BACK button: cancel the GOTO overlay, or return to the book list.
  if (display.touchscreen.touchInArea(HOME_BTN_X, BUTTON_Y, BUTTON_W,
                                      BUTTON_H)) {
    invertButton(HOME_BTN_X, BUTTON_Y, BUTTON_W, BUTTON_H,
                 inGotoUI ? "BACK" : "HOME");
    if (inGotoUI) {
      inGotoUI = false;
      displayPicture();
    } else {
      inPictureView = false;
      displayMainPage();
    }
    vTaskDelay(pdMS_TO_TICKS(TAP_DEBOUNCE_MS));
  }
  // GOTO button -> open the numeric keypad overlay.
  else if (display.touchscreen.touchInArea(GOTO_BTN_X, BUTTON_Y, BUTTON_W,
                                           BUTTON_H)) {
    invertButton(GOTO_BTN_X, BUTTON_Y, BUTTON_W, BUTTON_H, "GOTO");
    inGotoUI = true;
    pageInput.clear();
    displayGotoUI();
    vTaskDelay(pdMS_TO_TICKS(TAP_DEBOUNCE_MS));
  }
  // PREV page.
  else if (display.touchscreen.touchInArea(PREV_BTN_X, BUTTON_Y, BUTTON_W,
                                           BUTTON_H)) {
    invertButton(PREV_BTN_X, BUTTON_Y, BUTTON_W, BUTTON_H, "PREV");
    if (!inGotoUI && currentPageIndex > 0) {
      currentPageIndex--;
      displayPicture();
    }
    vTaskDelay(pdMS_TO_TICKS(TAP_DEBOUNCE_MS));
  }
  // NEXT page.
  else if (display.touchscreen.touchInArea(NEXT_BTN_X, BUTTON_Y, BUTTON_W,
                                           BUTTON_H)) {
    invertButton(NEXT_BTN_X, BUTTON_Y, BUTTON_W, BUTTON_H, "NEXT");
    if (!inGotoUI && currentPageIndex < (int)pageNames.size() - 1) {
      currentPageIndex++;
      displayPicture();
    }
    vTaskDelay(pdMS_TO_TICKS(TAP_DEBOUNCE_MS));
  }
}

static void handleTouch() {
  if (!inPictureView)
    handleMainViewTouch();
  else
    handlePictureViewTouch();
}

// --------------------------------------------------------------------------
// app_main
// --------------------------------------------------------------------------

extern "C" void app_main(void) {
  display.setDisplayMode(BLACK_AND_WHITE);
  display.setRotation(1);
  display.setFullUpdateThreshold(FULL_UPDATE_THRESHOLD);
  display.setTextColor(BLACK);
  display.clearDisplay();
  display.display();

  if (display.sdCardInit() != ESP_OK) {
    ESP_LOGE(TAG, "SD card init failed");
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(MARGIN_X, MARGIN_Y);
    display.print("SD card init failed.");
    display.display();
    return;
  }

  // Scan /books/ for one subfolder per book.
  char booksRoot[160];
  snprintf(booksRoot, sizeof(booksRoot), "%s/%s", display.getMountPoint(),
           BOOKS_FOLDER_NAME);
  listSubdirectories(booksRoot);

  // Figure out how many book-list rows fit above the button row.
  int lineH = MARGIN_Y + 4;
  visibleRows = (BUTTON_Y - (MARGIN_Y + HEADER_MARGIN)) / lineH;

  if (bookNames.empty()) {
    ESP_LOGE(TAG, "No books found under %s", booksRoot);
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(MARGIN_X, MARGIN_Y);
    display.print("No books found on SD card.");
    display.setCursor(MARGIN_X, MARGIN_Y + 40);
    display.print("Copy a /books/<name>/ folder of");
    display.setCursor(MARGIN_X, MARGIN_Y + 70);
    display.print("page images, then reset the board.");
    display.display();
    return;
  }

  currentBookIndex = 0;
  displayMainPage();

  while (true) {
    handleTouch();
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}
