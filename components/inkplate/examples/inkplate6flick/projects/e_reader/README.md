# E-Reader

SD-card eBook reader UI for Soldered Inkplate 6 Flick.

## Overview

Implements a simple, open-source eBook reader. Instead of parsing EPUB files
on-device, a companion PC-side Python tool converts an `.epub` into a
sequence of page images sized for the UI; those images are then copied onto
the SD card under `/books/<book_name>/`.

On boot, the example scans `/books/` on the SD card for subfolders (books),
shows a touchscreen list, and lets you pick one. Pages are loaded from the SD
card and rendered with the Inkplate image drawer. Navigation includes
PREV/NEXT (both for browsing the book list and for turning pages), HOME (back
to the book list), and a GOTO overlay with an on-screen numeric keypad to jump
directly to a page number.

The display runs in 1-bit (black & white) mode and uses partial updates for
responsive touch interaction. `setFullUpdateThreshold()` triggers an
automatic full refresh every few partial updates to reduce ghosting.

## Hardware Required

- Soldered Inkplate 6 Flick
- USB cable
- MicroSD card (FAT/FAT32 formatted)

## SD Card Content

```
/books/
  <Book Name 1>/
    0001.png
    0002.png
    ...
  <Book Name 2>/
    0001.bmp
    0002.bmp
    ...
```

- One subfolder per book under `/books/`.
- Each book folder holds one image file per page (BMP/JPG/JPEG/PNG).
- Filenames are sorted with a "natural" numeric sort, so plain numeric names
  like `2.bmp` / `10.bmp` still sort in the right order even without
  zero-padding — but zero-padded names (`0001.png`, `0002.png`, ...) are
  recommended for readability when browsing the card on a PC.
- **Every page image should be 758x930 pixels** — this is the size the UI
  layout (buttons, page counter) is designed for, and it matches the
  Inkplate 6 Flick's usable display area in portrait orientation.

## PC-Side EPUB Preprocessing

The `epubToImg/` folder (from the original Arduino example this was ported
from) contains a **host-side, PC-only** Python tool that renders an EPUB into
page images. It is not part of this ESP-IDF project and is never built or run
on the Inkplate itself — copy `epubToImg/` to your PC if you want to prepare
book images from an EPUB file.

1. Install Python 3, then install the tool's dependencies (in a virtual
   environment is recommended):
   ```
   pip install -r requirements.txt
   playwright install chromium
   ```
2. Run the tool against an EPUB, writing into an empty output folder:
   ```
   python epubToImg.py mybook.epub ./output --width 758 --height 930
   ```
3. Copy the resulting `./output` folder onto the SD card as
   `/books/<book_name>/` (e.g. `/books/MyBook/`).

See `epubToImg.py --help` for additional options (page-break threshold,
enforced font size, etc.).

## Setup

### 1. Prepare the SD card

Format the microSD card as FAT/FAT32 and copy one or more book folders under
`/books/` as described above.

### 2. Select the board

Run `idf.py menuconfig` and navigate to:
**Inkplate Boards → Inkplate6 Flick**

## Build and Flash

```
idf.py build
idf.py -p PORT flash monitor
```

## How to Use

1. Insert the prepared SD card and power on the board.
2. Tap **PREV** / **NEXT** to browse the book list, then tap **SELECT** to
   open the highlighted book.
3. In page view:
   - Tap **PREV** / **NEXT** to turn pages.
   - Tap **HOME** to return to the book list.
   - Tap **GOTO** to open the numeric keypad, type a page number, and tap
     **OK** to jump to it (**CLR** clears the current input, **BACK** cancels
     the overlay).

## Expected Output

- E-paper: a book-list UI (from `/books/`), then full-page images with
  PREV/NEXT/HOME/GOTO buttons and a page counter (`current / total`).
- Log output: SD card and "no books found" errors are logged and also shown
  on the e-paper screen.

## Notes

- Display mode is 1-bit (black & white); partial updates are only supported
  in that mode.
- Page images are decoded from the SD card on every page turn; large images
  or unusual formats decode more slowly. Prefer consistently sized 758x930
  pages.
- This example is fully interactive and does not use deep sleep.
- Ported from the Soldered Inkplate Arduino library's
  `Inkplate6FLICK_E_Reader` example. The original's hand-rolled `Book`/
  `Picture` doubly-linked lists were replaced with plain
  `std::vector<std::string>` lists indexed by an integer (see `main/main.cpp`
  for details); SdFat's `File::openNextFile()`/`isDirectory()` folder walk was
  replaced with the POSIX `opendir()`/`readdir()` VFS API used elsewhere in
  this component's SD card examples; `String` became `std::string`.

## Resources

- Docs: https://docs.soldered.com/inkplate
- Support: https://forum.soldered.com/
- Image tool: https://tools.soldered.com/tools/image-converter/
