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
// CHUNK is used by zlib
#define CHUNK 16384

// this is a collection of ascii chars that are sorted by approx. grayscale
// value
//  `.-':_,^=;><+!rc*/z?sLTv)J7(|Fi{C}fI31tlu[neoZ5Yxjya]2ESwqkP6h9d4VpOGbUAKXHm8RD#$Bg0MNWQ%&@

#define ASCII_ARR_LEN 42

#define GRAYSCALE_TRANSFORM(x)                                                 \
  ((0.005628 + (-0.820202) * (x) + 362.379369 * (x) * (x) +                    \
    (-1008.271666) * (x) * (x) * (x) + 893.523573 * (x) * (x) * (x) * (x) +    \
    (-209.142326) * (x) * (x) * (x) * (x) * (x) +                              \
    (-37.621346) * (x) * (x) * (x) * (x) * (x) * (x)) /                        \
   (1.0 + 321.611872 * (x) + (-730.539905) * (x) * (x) +                       \
    55.800239 * (x) * (x) * (x) + 1042.410369 * (x) * (x) * (x) * (x) +        \
    (-945.810982) * (x) * (x) * (x) * (x) * (x) +                              \
    255.596133 * (x) * (x) * (x) * (x) * (x) * (x)))
static const char grayscaleAscii[ASCII_ARR_LEN] =
    " `-':_,^=;>+!*/zTv({nyaSw9dpOKX8$B0MWQ%&@";

static const float grayscaleAsciiValues[ASCII_ARR_LEN] = {
    0,      0.0751, 0.0848, 0.1227, 0.1403, 0.1559, 0.185,  0.2183, 0.2417,
    0.2571, 0.2852, 0.2919, 0.3099, 0.3294, 0.3384, 0.3609, 0.3747, 0.3838,
    0.3993, 0.42,   0.4503, 0.4703, 0.4833, 0.4992, 0.5509, 0.565,  0.5776,
    0.587,  0.5972, 0.6099, 0.6465, 0.6631, 0.6816, 0.6925, 0.7086, 0.7235,
    0.7332, 0.7602, 0.7834, 0.8037, 0.9999};

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
  size_t pxLen;
  // heap allocated
  uint8_t *IDATConcat;
  pixel *pxArr;
  // TODO: change this shit up for something like bool or smth
  int isIDATConcatAllocd;
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

void asciiFileDump(const char *, uint8_t *, size_t, size_t);

void htmlFileDump(const char *, uint8_t *, size_t, size_t);

int hexStreamValue(void *, size_t, size_t, FILE *);

int imageInit(imagePNG *, FILE *);

void imageFree(imagePNG *);

void IDATConcatFree(imagePNG *);

int IHDRDecode(IHDRDecoded *, FILE *);

int IDATInflate(imagePNG *, uint8_t *);

long hexStreamFindHeader(chunkHeadersUInt32, FILE *);

int hexStreamSkipHeader(FILE *);

int hexStreamCountHeaders(chunkHeadersUInt32, FILE *);

int hexStreamConcatIDAT(imagePNG *, FILE *);

int scanlineFilterReconstruction(uint8_t *, uint8_t *, uint8_t *, size_t,
                                 uint8_t, size_t);

int IDATDefilter(imagePNG *, uint8_t *, uint8_t *);

int pxParseIDAT(uint8_t *, pixel *, size_t, uint8_t, size_t);

int asciiImageGenerate(uint8_t *, pixel *, size_t);

void displayHelpPage();
