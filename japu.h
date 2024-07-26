#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define WIDTH_DEFAULT 16

typedef struct {
  size_t byteLen;
  u_int8_t signature[8];
  u_int32_t width, height;
  u_int8_t bitDepth, colorType, compressionMethod, filterMethod,
      interlaceMethod;
  u_int8_t IDAT[];

} imagePNG;

void hexDump(u_int8_t *, size_t, size_t);

int hexStreamValue(void *, size_t, size_t, FILE *);

int imageInit(imagePNG *, FILE *);
