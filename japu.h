#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define WIDTH_DEFAULT 16

typedef enum {
  IDAT = 1229209940,
} chunkHeadersUInt32;

typedef struct {
  u_int8_t bitDepth;
  u_int8_t colorType;
  u_int8_t compressionMethod;
  u_int8_t filterMethod;
  u_int8_t interlaceMethod;
  u_int32_t width, height;
} IHDRDecoded;

typedef struct {
  size_t byteLen;
  u_int8_t signature;
  IHDRDecoded IHDR;

} imagePNG;

void hexDump(u_int8_t *, size_t, size_t);

int hexStreamValue(void *, size_t, size_t, FILE *);

int imageInit(imagePNG *, FILE *);

int IHDRDecode(IHDRDecoded *, FILE *);

long hexStreamFindHeader(chunkHeadersUInt32, FILE *);
