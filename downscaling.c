#include "downscaling.h"
#include <stdint.h>

int pixelMapInit(mapPixel **map, size_t oldWidth, size_t oldHeight,
                 size_t newWidth, size_t newHeight) {

  *map = (mapPixel *)calloc(1, sizeof(mapPixel));

  if (map == NULL) {
    fprintf(stderr, "[ERROR] pixel map allocation failure %d\n", -1);
    return -1;
  }

  mapPixel *mapObj = *map;

  mapObj->preScaledWidth = oldWidth;
  mapObj->scaledWidth = newWidth;

  mapObj->preScaledHeight = oldHeight;
  mapObj->scaledHeight = newHeight;

  mapObj->scaleWidth = newWidth / (double)oldWidth;

  mapObj->scaleHeight = newHeight / (double)oldHeight;

  mapObj->pxMapLen = newWidth * newHeight;

  mapObj->pxMap = (size_t(*)[16])calloc(mapObj->pxMapLen, sizeof(size_t[16]));

  if (mapObj->pxMap == NULL) {
    fprintf(stderr, "[ERROR] pixel map allocation failure %d\n", -1);
    return -1;
  }

  return 0;
}

int pixelMapFree(mapPixel *mapArr) {

  if (mapArr == NULL) {
    fprintf(stderr, "[ERROR] pixel map cannot be NULL %d\n", -1);
    return -1;
  }
  free(mapArr->pxMap);
  free(mapArr);

  return 0;
}

#define CHECK_BOUND(x, xmin, xmax)                                             \
  {                                                                            \
    if (x < xmin)                                                              \
      x = xmin;                                                                \
    else if (x > xmax)                                                         \
      x = xmax;                                                                \
  }

int pixelMapDownscaled(mapPixel *mp) {

  if (mp == NULL) {
    fprintf(stderr, "[ERROR] pixel map cannot be NULL %d\n", -1);
    return -1;
  }

  int x, y;

  for (int i = 0; i < mp->pxMapLen; ++i) {

    // x and y in the bigger, scaled up relatively image
    x = (int)round((i % mp->scaledWidth) / mp->scaleWidth) - 1;
    y = (int)round(((float)i / mp->scaledHeight) / mp->scaleHeight) - 1;

    int XVals[4] = {x - 1, x, x + 1, x + 2};
    int YVals[4] = {y - 1, y, y + 1, y + 2};

    // Boundary clamping
    for (size_t n = 0; n < 4; ++n) {
      CHECK_BOUND(XVals[n], 0, mp->preScaledWidth - 1);
      CHECK_BOUND(YVals[n], 0, mp->preScaledHeight - 1);
    }
    for (size_t n = 0; n < 4; ++n) {
      for (size_t m = 0; m < 4; ++m) {
        mp->pxMap[i][n * 4 + m] = YVals[n] * mp->preScaledWidth + XVals[m];
      }
    }
  }
  return 0;
}

static float bicubicBasis(float t, float f0, float f1, float f2, float f3) {
  return 0.5f *
         (-f0 * (t * t * t - 2 * t * t + t) +
          f1 * (3 * t * t * t - 5 * t * t + 2) -
          f2 * (3 * t * t * t - 4 * t * t - t) + f3 * (t * t * t - t * t));
}

int pixelMapInterpolate(pixel *outArr, pixel *inArr, mapPixel *mp) {

  if (mp == NULL) {
    fprintf(stderr, "[ERROR] pixel map cannot be NULL %d\n", -1);
    return -1;
  }

  if (outArr == NULL) {
    fprintf(stderr, "[ERROR] output array cannot be NULL %d\n", -1);
    return -1;
  }

  if (inArr == NULL) {
    fprintf(stderr, "[ERROR] input array cannot be NULL %d\n", -1);
    return -1;
  }

  for (size_t i = 0; i < mp->pxMapLen; ++i) {
    float values[4] = {
        0.0f,
        0.0f,
        0.0f,
        0.0f,
    };

    for (int n = 0; n < 4; ++n) {
      values[n] =
          bicubicBasis(abs(n - 1) / 2.0f, inArr[mp->pxMap[i][n * 4]].grayscale,
                       inArr[mp->pxMap[i][n * 4 + 1]].grayscale,
                       inArr[mp->pxMap[i][n * 4 + 2]].grayscale,
                       inArr[mp->pxMap[i][n * 4 + 3]].grayscale);
    }

    outArr[i].grayscale =
        (uint8_t)bicubicBasis(0, values[0], values[1], values[2], values[3]);
  }
  return 0;
}
