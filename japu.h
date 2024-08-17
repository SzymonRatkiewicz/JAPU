#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "zlib.h"
#include <assert.h>
#include <string.h>
#include <sys/types.h>
#define WIDTH_DEFAULT 16

#if defined(MSDOS) || defined(OS2) || defined(WIN32) || defined(__CYGWIN__)
#include <fcntl.h>
#include <io.h>
#define SET_BINARY_MODE(file) _setmode(_fileno(file), O_BINARY)
#else
#define SET_BINARY_MODE(file)
#endif

#define CHUNK 16384

typedef enum {
  IDAT = 1229209940,
  IEND = 1229278788,
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
  // heap allocated
  uint8_t *IDATConcat;
} IDATData;

typedef struct {
  size_t IDATCount;
  IDATData IDAT;
  size_t byteLen;
  u_int8_t signature;
  IHDRDecoded IHDR;

} imagePNG;

void hexDump(u_int8_t *, size_t, size_t);

int hexStreamValue(void *, size_t, size_t, FILE *);

int imageInit(imagePNG *, FILE *);

int IHDRDecode(IHDRDecoded *, FILE *);

long hexStreamFindHeader(chunkHeadersUInt32, FILE *);

int hexStreamSkipHeader(FILE *);

int hexStreamCountHeaders(chunkHeadersUInt32, FILE *);

int hexStreamConcatIDAT(imagePNG *, FILE *);
