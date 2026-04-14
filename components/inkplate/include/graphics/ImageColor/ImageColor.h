#ifndef _IMAGE_COLOR_H_
#define _IMAGE_COLOR_H_

#include <stdint.h>
#include <stdbool.h>

#include "BMP/BMP.h"
#include "JPEG/JPEG.h"
#include "PNG/PNG.h"

#include "DitherKernels.h"

#define PALETTE_SIZE 3

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

  void setDitherKernel(DitherKernel kernel);
  uint8_t findClosestPalette(int16_t r, int16_t g, int16_t b);
  // called by format decoders when dithering is enabled
  uint8_t  getDitheredPixel(uint8_t r, uint8_t g, uint8_t b, int i, int w);
  void     ditherSwap(int w);
  uint32_t m_ditherPalette[256]; // packed 0x00RRGGBB per indexed-BMP palette entry
  uint8_t  *m_palette;

private:
  void beginDither();
  void endDither();

  Inkplate      *m_inkplate;
  ImageColorBMP  m_bmp;
  ImageColorJPEG m_jpeg;
  ImageColorPNG  m_png;
  const DitherKernelDef *m_currentKernel = &DITHER_KERNELS[0];

  int16_t *m_ditherR[2]; // per-row quantisation error — red channel
  int16_t *m_ditherG[2]; // per-row quantisation error — green channel
  int16_t *m_ditherB[2]; // per-row quantisation error — blue channel
};

#endif
