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

    CHECK_BOUND(x, 0, mp->preScaledHeight - 1)
    CHECK_BOUND(y, 0, mp->preScaledHeight - 1)

    int XBound[2], YBound[2];

    XBound[0] = x - 1;
    YBound[0] = y - 1;

    XBound[1] = x + 2;
    YBound[1] = y + 2;

    CHECK_BOUND(XBound[0], 0, mp->preScaledWidth);
    CHECK_BOUND(XBound[1], 0, mp->preScaledWidth);

    CHECK_BOUND(YBound[0], 0, mp->preScaledHeight);
    CHECK_BOUND(YBound[1], 0, mp->preScaledHeight);

    size_t coreIndex = y * mp->preScaledHeight + x;

    // Keep an eye on this POS code I am not confident enough to trust it
    // myself

    for (int j = 0; j < (YBound[1] - YBound[0] + 1); ++j) {
      for (int n = 0; n < 4; ++n) {

        calcIndex = coreIndex + ((j - 1) * mp->preScaledWidth + (n - 1));
        CHECK_BOUND(calcIndex,
                    YBound[0] * mp->preScaledWidth + j * mp->preScaledWidth +
                        XBound[0],
                    YBound[0] * mp->preScaledWidth + j * mp->preScaledWidth +
                        XBound[1]);

        mp->pxMap[i][j * 4 + n] = calcIndex;
      }
    }
  }
  return 0;
}
