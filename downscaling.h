#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define PRINT_MAP_STRUCT(map)                                                  \
  {                                                                            \
    printf("\noldWidth: %ld\n", map->preScaledWidth);                          \
    printf("newWidth: %ld\n", map->scaledWidth);                               \
    printf("oldHeight: %ld\n", map->preScaledHeight);                          \
    printf("newHeight: %ld\n", map->scaledHeight);                             \
    printf("scaleWidth: %lf\n", map->scaleWidth);                              \
    printf("scaleHeight: %lf\n", map->scaleHeight);                            \
    printf("pxMapLen: %ld\n", map->pxMapLen);                                  \
    printf("pxMap: %p\n", map->pxMap);                                         \
  }

// Struct used for mapping pixels
typedef struct {
  size_t preScaledWidth;
  size_t preScaledHeight;

  size_t scaledWidth;
  size_t scaledHeight;

  double scaleWidth;
  double scaleHeight;

  // an array of indices, this  assumes your array has one index per pixel
  size_t (*pxMap)[16];

  size_t pxMapLen;

} mapPixel;

int pixelMapInit(mapPixel **, size_t, size_t, size_t, size_t);

int pixelMapFree(mapPixel *);

int pixelMapDownscaled(mapPixel *);
