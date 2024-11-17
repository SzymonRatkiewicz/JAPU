#include "downscaling.h"

int pixelMapInit(mapPixel **map, size_t oldWidth, size_t oldHeight,
                 size_t newWidth, size_t newHeight) {

  *map = (mapPixel *)calloc(1, sizeof(mapPixel));

  if (map == NULL) {
    printf("[ERROR] pixel map allocation failure %d\n", -1);
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
    printf("[ERROR] pixel map allocation failure %d\n", -1);
    return -1;
  }

  return 0;
}

int pixelMapFree(mapPixel *mapArr) {

  if (mapArr == NULL) {
    printf("[ERROR] pixel map cannot be NULL %d\n", -1);
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

  int x, y;
  int scaledX, scaledY;
  long calcIndex;

  for (int i = 0; i < mp->pxMapLen; ++i) {

    // x and y in the smaller image
    scaledX = i % mp->scaledWidth;
    scaledY = i / mp->scaledHeight;

    // x and y in the bigger, scaled up relatively image
    x = (int)round(scaledX / mp->scaleWidth) - 1;
    y = (int)round(scaledY / mp->scaleHeight) - 1;

    int XVals[4] = {x - 1, x, x + 1, x + 2};
    int YVals[4] = {y - 1, y, y + 1, y + 2};

    // Boundary clamping
    for (size_t n = 0; n < 4; ++n) {
      CHECK_BOUND(XVals[n], 0, mp->preScaledWidth - 1);
      CHECK_BOUND(YVals[n], 0, mp->preScaledHeight - 1);
    }
    for (size_t n = 0; n < 4; ++n) {
      for (size_t m = 0; m < 4; ++m) {
        mp->pxMap[i][n * 4 + m] = YVals[n] * mp->preScaledHeight + XVals[m];
      }
    }
  }
  return 0;
}
