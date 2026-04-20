#ifndef DITHER_KERNELS_H
#define DITHER_KERNELS_H

#include "stdlib.h"

struct DitherKernelDef
{
  uint8_t width;
  uint8_t height;
  uint8_t x;
  uint16_t coef;
  const unsigned char *data;
};

static const unsigned char kernelFloydSteinberg[] = {
  0, 0, 7, 3, 5, 1,
};

static const unsigned char kernelJarvisJudiceNinke[] = {
  0, 0, 0, 7, 5, 3, 5, 7, 5, 3, 1, 3, 5, 3, 1,
};

static const unsigned char kernelAtkinson[] = {
  0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0,
};

static const unsigned char kernelBurkes[] = {
  0, 0, 0, 8, 4, 2, 4, 8, 4, 2, 0, 0, 0, 0, 0,
};

static const unsigned char kernelStucki[] = {
  0, 0, 0, 8, 4, 2, 4, 8, 4, 2, 1, 2, 4, 2, 1,
};

static const unsigned char kernelSierraLite[] = {
  0, 0, 2, 1, 1, 0, 0, 0, 0,
};

static const unsigned char kernelReducedDiffusion[] = {
  0, 0, 5, 2, 3, 1,
};

static const DitherKernelDef DITHER_KERNELS[] = {
  {3, 2, 1, 16, kernelFloydSteinberg},   {5, 3, 2, 48, kernelJarvisJudiceNinke},
  {4, 3, 1, 8,  kernelAtkinson},         {5, 3, 2, 32, kernelBurkes},
  {5, 3, 2, 42, kernelStucki},           {3, 3, 1, 4,  kernelSierraLite},
  {3, 2, 1, 26, kernelReducedDiffusion},
};

static const uint8_t DITHER_KERNEL_COUNT = sizeof DITHER_KERNELS / sizeof DITHER_KERNELS[0];
#endif
