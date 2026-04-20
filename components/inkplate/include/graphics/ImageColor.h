#ifndef IMAGE_COLOR_H
#define IMAGE_COLOR_H

#include <stdint.h>
#include <stdbool.h>

#include "BMP.h"
#include "JPEG.h"
#include "PNG.h"

#include "DitherKernels.h"

static constexpr uint8_t DITHER_ROW_COUNT = 4; // ring buffer; covers all kernels (max height 3)
static constexpr uint8_t DITHER_ROW_MASK  = DITHER_ROW_COUNT - 1;

#define RED8(a)   (((a) >> 16) & 0xff)
#define GREEN8(a) (((a) >> 8) & 0xff)
#define BLUE8(a)  (((a)) & 0xff)

typedef enum
{
  FloydSteinberg = 0,
  JarvisJudiceNinke,
  Atkinson,
  Burkes,
  Stucki,
  SierraLite,
  ReducedDiffusion // Floyd-Steinberg pattern at ~69% strength — more vibrant, less washed-out
} DitherKernel;

class Inkplate;

class ImageColor
{
public:
  ImageColor(Inkplate *inkplate);

  bool draw(uint8_t *buf, int x, int y, bool invert = false, bool dither = false);
  bool draw(uint8_t *buf, int32_t len, int x, int y, bool invert = false, bool dither = false);
  bool draw(const char *src, int x, int y, bool invert = false, bool dither = false);
  bool draw(const uint8_t *buf, int w, int h, int x, int y, bool invert = false, bool dither = false);

  void setDitherKernel(DitherKernel kernel);
  uint8_t findClosestPalette(int16_t r, int16_t g, int16_t b);
  // called by format decoders when dithering is enabled
  uint8_t  getDitheredPixel(uint8_t r, uint8_t g, uint8_t b, int i, int w);
  void     ditherSwap(int w);
  uint8_t  *m_palette;
  uint32_t pallete[7];    // board color palette, packed 0x00RRGGBB
  uint8_t  palletteSize;  // number of valid entries in pallete[]

private:
  void beginDither();
  void endDither();

  Inkplate *m_inkplate;
  BMP       m_bmp;
  JPEG      m_jpeg;
  PNG       m_png;
  const DitherKernelDef *m_currentKernel = &DITHER_KERNELS[0];

  int16_t *m_ditherR[DITHER_ROW_COUNT]; // ring-buffer of row error accumulators — red channel
  int16_t *m_ditherG[DITHER_ROW_COUNT]; // ring-buffer of row error accumulators — green channel
  int16_t *m_ditherB[DITHER_ROW_COUNT]; // ring-buffer of row error accumulators — blue channel
  int      m_rowIdx;                    // current row position in the ring buffer
};

#endif
