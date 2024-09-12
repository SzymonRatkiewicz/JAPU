#include <math.h>
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
  uint8_t bitDepth;
  uint8_t colorType;
  uint8_t compressionMethod;
  uint8_t filterMethod;
  uint8_t interlaceMethod;
  uint32_t width, height;
} IHDRDecoded;

typedef struct {
  uint8_t red;
  uint8_t green;
  uint8_t blue;
  uint8_t alpha;
  uint8_t grayscale;

} pixel;

typedef struct {
  size_t byteLen;
  // heap allocated
  uint8_t *IDATConcat;
  pixel *imagedata;

} IDATData;

typedef struct {
  size_t IDATCount;
  IDATData IDAT;
  size_t byteLen;
  uint8_t signature;
  IHDRDecoded IHDR;
  size_t scanlineLen;
  size_t bytesPerPx;
} imagePNG;

void hexDump(uint8_t *, size_t, size_t);

void hexFileDump(const char *, uint8_t *, size_t, size_t);

int hexStreamValue(void *, size_t, size_t, FILE *);

int imageInit(imagePNG *, FILE *);

int IHDRDecode(IHDRDecoded *, FILE *);

int IDATInflate(imagePNG *, uint8_t *);

long hexStreamFindHeader(chunkHeadersUInt32, FILE *);

int hexStreamSkipHeader(FILE *);

int hexStreamCountHeaders(chunkHeadersUInt32, FILE *);

int hexStreamConcatIDAT(imagePNG *, FILE *);

int scanlineFilterReconstruction(uint8_t *, uint8_t *, uint8_t *, size_t,
                                 uint8_t, size_t);
int IDATDefilter(imagePNG *, uint8_t *, uint8_t *);
