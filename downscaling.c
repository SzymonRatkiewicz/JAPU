#include "downscaling.h"
#include <stddef.h>

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

int pixelMapDownscaled(mapPixel *mp) {

  int x, y;
  int scaledX, scaledY;
  long calcIndex;
  for (int i = 0; i < mp->pxMapLen; ++i) {
    // TODO: add boundary checking every mp index is
    // corresponding to 16 values which are indices of pixels mp is mapped to

    scaledX = i % mp->scaledWidth;
    scaledY = i / mp->scaledHeight;

    x = (int)round(scaledX / mp->scaleWidth);
    y = (int)round(scaledY / mp->scaleHeight);

    for (int j = 0; j < 4; ++j) {
      for (int n = 0; n < 4; ++n) {

        // in parenthesis - correction for boundary presence
        calcIndex = x * y + (j * mp->preScaledWidth + n);
        mp->pxMap[i][j * 4 + n] = calcIndex;
      }
    }
  }
  return 0;
}
