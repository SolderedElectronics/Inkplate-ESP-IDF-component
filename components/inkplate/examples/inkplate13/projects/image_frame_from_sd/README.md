# Image Frame From SD

Turn Soldered Inkplate 13SPECTRA into a digital picture frame that cycles through images stored on a microSD card.

## Overview

Reads image files from a folder on a FAT32-formatted microSD card and displays them one at a time, dithered to the Inkplate 13SPECTRA's 6-color palette. After drawing an image, the board deep sleeps for `SECS_BETWEEN_PICTURES` (default: 60 seconds) and wakes up (by timer, or early by pressing the wake button) to show the next image. The current position in the folder is kept in RTC memory, so the slideshow resumes where it left off across deep sleep cycles and wraps back to the first image after the last one has been shown.

Supports BMP (1/4/8/24-bit), JPEG, and PNG files. Files that can't be decoded are skipped automatically, matching the original `Inkplate13SPECTRA_Image_Frame_From_SD` Arduino sketch's behavior. Because deep sleep resets the ESP32, execution always restarts from `app_main()` on every wake; only the RTC-memory image index survives.

## Hardware Required

- Soldered Inkplate 13SPECTRA
- USB cable
- MicroSD card (FAT32 formatted), loaded with images

## Setup

### 1. Prepare the SD card

Format the microSD card as FAT32. Create a folder named `images` at the card root and copy your BMP/JPEG/PNG files into it (change the folder name via `FOLDER_PATH` in `main/main.cpp` if you'd rather use a different one).

### 2. Configure the slideshow (optional)

In `main/main.cpp`:

```cpp
#define SECS_BETWEEN_PICTURES 60 // Time between images, in seconds
#define FOLDER_PATH "images"     // Folder on the SD card, relative to the card root
```

### 3. Select the board

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate13**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## Expected Output

- Display: each image in the folder shown in turn, one per wake cycle, dithered to the 6-color palette.
- "SD Card error!" if the microSD card can't be initialized.
- "Error opening folder!" if `FOLDER_PATH` doesn't exist on the card.
- "The folder is empty" if the folder has no files in it.
- "No drawable images found" if every file in the folder failed to decode.
- Serial Monitor: which file is being drawn each cycle, and a warning for any file that gets skipped.
- The board deep sleeps for `SECS_BETWEEN_PICTURES` between images; pressing the wake button skips ahead immediately instead of waiting out the timer.

## Notes

- Color palette: Inkplate 13SPECTRA is a 6-color e-paper board (black, white, yellow, red, blue, green via the `INKPLATE_*` color macros). There is no `INKPLATE_ORANGE` on this board, unlike Inkplate 6Color. `display.image.draw()` dithers each image down to this palette.
- Orientation: the board defaults to rotation 3 (landscape) right after construction; this port doesn't call `setRotation()`, matching the original sketch.
- File ordering: files are listed in the order the SD card's FAT directory returns them (via `opendir()`/`readdir()`), not sorted alphabetically. This matches the original sketch, which also walked files in raw on-disk directory order (via SdFat) rather than sorting them.
- No extension filtering: like the original sketch, every non-hidden, non-subdirectory file in the folder is attempted; files that fail to decode (wrong format, corrupted, or an actual subfolder) are logged and skipped rather than filtered out ahead of time by file extension.
- Deviation from the original sketch: the Arduino version tracks position with SdFat's raw on-disk directory index and an `openLastFile()`/`openNext()` resume dance, including a special case for index 0 that SdFat can't open directly. This port instead builds a plain in-memory list of file names each wake cycle and indexes into it directly (`nextImageIndex`, `RTC_DATA_ATTR`) - same end result (resumable, wrapping slideshow), without the ESP-IDF component needing an SdFat-equivalent API. Also, when the whole folder fails to draw (e.g. only unsupported files are present), this port stops and deep sleeps waiting for the wake button instead of looping indefinitely, since the original sketch has no such guard and could otherwise busy-loop through a bad folder.
- Capacity: up to `MAX_FILES` (default 300) files per folder and `MAX_NAME_LEN` (default 128) characters per file name are tracked, in static (non-stack) storage; raise these in `main/main.cpp` if needed.
- Wake button: tied to GPIO 18 on this board (see the `wake_up_button` advanced example), used both to skip ahead early and as the sole wakeup source when an error/empty-folder screen is shown.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
