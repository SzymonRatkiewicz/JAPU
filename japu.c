#include "japu.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <zlib.h>

void hexDump(uint8_t *array, size_t arrLen, size_t width) {

  for (int i = 0; i < arrLen; i++) {
    if (i != 0 && i % (width == 0 ? WIDTH_DEFAULT : width) == 0) {
      printf("\n");
    }
    printf("%02X ", array[i]);
  }
}
void hexFileDump(const char *filename, uint8_t *array, size_t arrLen,
                 size_t width) {

  FILE *out = fopen(filename, "w");

  if (out == NULL) {
    printf("[ERROR] FILE NOT FOUND %d\n", 1);
    exit(1);
  }

  for (int i = 0; i < arrLen; i++) {
    if (i != 0 && i % (width == 0 ? WIDTH_DEFAULT : width) == 0) {
      fprintf(out, "\n");
    }
    fprintf(out, "%02X ", array[i]);
  }

  fclose(out);
}
int hexStreamValue(void *val, size_t hexSize, size_t arrLen, FILE *file) {

  if (file == NULL)
    return -1;
  if (val == NULL)
    return -1;

  void *hexArr = calloc(arrLen, hexSize);
  if (hexArr == NULL) {
    return -1;
  }

  fread(hexArr, hexSize, arrLen, file);

  uint64_t res = 0;
  for (int i = 0; i < hexSize * arrLen; i++) {
    res |= *((uint8_t *)hexArr + i) << ((hexSize * arrLen - 1 - i) * 8);
  }

  if (memcpy(val, &res, hexSize * arrLen) == NULL) {
    free(hexArr);
    return -1;
  }

  free(hexArr);
  return hexSize * arrLen;
}

int imageInit(imagePNG *image, FILE *file) {

  if (image == NULL)
    return -1;
  if (file == NULL)
    return -1;

  if (fseek(file, 0, SEEK_END) != 0) {
    return -1;
  }

  if ((image->byteLen = ftell(file)) < 0) {
    return -1;
  }

  rewind(file);
  IHDRDecoded IHDR;

  IHDR.width = 0;
  IHDR.height = 0;
  IHDR.bitDepth = 0;
  IHDR.colorType = 0;
  IHDR.compressionMethod = 0;
  IHDR.filterMethod = 0;
  IHDR.interlaceMethod = 0;

  image->bytesPerPx = 0;
  image->IHDR = IHDR;

  if (IHDRDecode(&image->IHDR, file) != 0) {
    printf("[ERROR] Image header decoding failed\n");
    exit(1);
  }

  image->IDATCount = hexStreamCountHeaders(IDAT, file);

  switch (image->IHDR.colorType) {
  case 0:
    image->bytesPerPx = 1; // grayscale
    break;
  case 2:
    image->bytesPerPx = 3; // RGB
    break;
  case 3:
    image->bytesPerPx = 1; // indexed colors
    break;
  case 4:
    image->bytesPerPx = 2; // grayscale + alpha channel
    break;
  case 6:
    image->bytesPerPx = 4; // RGB + alpha channel
    break;
  default:
    return -1;
  }

  image->scanlineLen =
      ((image->IHDR.bitDepth * image->IHDR.width * image->bytesPerPx) / 8) + 1;

  image->IDAT.pxLen =
      ((image->scanlineLen - 1) * image->IHDR.height) / image->bytesPerPx;
  image->IDAT.pxArr = (pixel *)calloc(image->IDAT.pxLen, sizeof(pixel));

  return 0;
}

void printIHDR(IHDRDecoded *IHDR) {

  printf("IHDR Details:\n");
  printf("  Width: %u\n", IHDR->width);
  printf("  Height: %u\n", IHDR->height);
  printf("  Bit Depth: %u\n", IHDR->bitDepth);
  printf("  Color Type: %u\n", IHDR->colorType);
  printf("  Compression Method: %u\n", IHDR->compressionMethod);
  printf("  Filter Method: %u\n", IHDR->filterMethod);
  printf("  Interlace Method: %u\n", IHDR->interlaceMethod);
}

int IHDRDecode(IHDRDecoded *IHDR, FILE *file) {

  if (file == NULL)
    return -1;
  if (IHDR == NULL)
    return -1;

  fseek(file, 8, SEEK_SET); // skip PNG header
  fseek(file, 4, SEEK_CUR); // skip IHDR length
  fseek(file, 4, SEEK_CUR); // skip IHDR header

  hexStreamValue(&IHDR->width, 1, 4, file);
  hexStreamValue(&IHDR->height, 1, 4, file);

  hexStreamValue(&IHDR->bitDepth, 1, 1, file);
  hexStreamValue(&IHDR->colorType, 1, 1, file);
  hexStreamValue(&IHDR->compressionMethod, 1, 1, file);
  hexStreamValue(&IHDR->filterMethod, 1, 1, file);
  hexStreamValue(&IHDR->interlaceMethod, 1, 1, file);
  return 0;
}

int IDATInflate(imagePNG *image, uint8_t *out) {

  z_stream stream;
  stream.zalloc = Z_NULL;
  stream.zfree = Z_NULL;
  stream.opaque = Z_NULL;

  stream.avail_in = image->IDAT.byteLen;
  stream.next_in = image->IDAT.IDATConcat;
  stream.avail_out = image->IDAT.byteLen * 4;
  stream.next_out = out;

  if (inflateInit(&stream) != Z_OK) {
    fprintf(stderr, "[ERROR] inflateInit failed!\n");
    return -1;
  }

  int ret = inflate(&stream, Z_NO_FLUSH);
  if (ret != Z_STREAM_END) {
    fprintf(stderr, "[ERROR] inflate failed: %d\n", ret);
    inflateEnd(&stream);
    return -1;
  }

  if (inflateEnd(&stream) == Z_STREAM_ERROR)
    return -1;

  return 0;
}

int hexStreamSkipHeader(FILE *file) {
  // This function requires file pointer to be at chunks length section

  if (file == NULL)
    return -1;

  uint32_t len = 0;
  uint32_t curHeader = 0;
  long curPos = ftell(file);
  if (hexStreamValue(&len, 1, 4, file) < 0) {
    return -1;
  }

  fseek(file, 8 + len, SEEK_CUR);
  return (ftell(file) - curPos);
}

long hexStreamFindHeader(chunkHeadersUInt32 header, FILE *file) {
  // This function requires file pointer to be at chunks length section

  if (file == NULL)
    return -1;

  long curPos = ftell(file);
  fseek(file, 0, SEEK_END);
  long fileSize = ftell(file);
  fseek(file, curPos, SEEK_SET);

  uint32_t len = 0;
  uint32_t curHeader = 0;
  long filePos = -1;

  while (curHeader != IEND) {
    hexStreamValue(&len, 1, 4, file);
    hexStreamValue(&curHeader, 1, 4, file);
    if (curHeader == header) {
      fseek(file, -4, SEEK_CUR);
      filePos = ftell(file);
      break;
    }

    fseek(file, 4 + len, SEEK_CUR);
  }

  return filePos;
}

int hexStreamCountHeaders(chunkHeadersUInt32 header, FILE *file) {

  if (file == NULL)
    return -1;

  long oldPos = ftell(file);
  int headerCount = 0;
  uint32_t curHeader = 0;

  fseek(file, 8, SEEK_SET);
  while (hexStreamFindHeader(header, file) != -1) {
    hexStreamValue(&curHeader, 1, 4, file);
    fseek(file, -8, SEEK_CUR);
    if (curHeader == header) {
      ++headerCount;
      hexStreamSkipHeader(file);
    }
  }

  fseek(file, oldPos, SEEK_SET);

  return headerCount;
}

int hexStreamConcatIDAT(imagePNG *img, FILE *file) {

  if (img == NULL)
    return -1;
  if (file == NULL)
    return -1;

  long oldPos = ftell(file);
  size_t concatLen = 0;
  uint32_t len = 0;

  fseek(file, 8, SEEK_SET); // skip PNG header

  long IDATPosCache[img->IDATCount];
  uint32_t IDATLenCache[img->IDATCount];

  for (int i = 0; i < img->IDATCount; i++) {
    IDATPosCache[i] = hexStreamFindHeader(IDAT, file);

    fseek(file, -4, SEEK_CUR);
    hexStreamValue(&len, 1, 4, file);
    fseek(file, -4, SEEK_CUR);
    hexStreamSkipHeader(file);
    IDATLenCache[i] = len;
    concatLen += len;
  }

  img->IDAT.byteLen = concatLen;
  uint8_t *IDATs = (uint8_t *)calloc(concatLen, sizeof(uint8_t));
  size_t IDATsPointer = 0;

  for (int i = 0; i < img->IDATCount; i++) {
    fseek(file, IDATPosCache[i] + 4, SEEK_SET);
    fread(&IDATs[IDATsPointer], 1, IDATLenCache[i], file);
    IDATsPointer += IDATLenCache[i];
  }

  img->IDAT.IDATConcat = IDATs;

  fseek(file, oldPos, SEEK_SET);

  return 0;
}

static uint8_t paethFilter(uint8_t a, uint8_t b, uint8_t c) {

  int p = (int)a + (int)b - (int)c;
  int pa = abs(p - (int)a);
  int pb = abs(p - (int)b);
  int pc = abs(p - (int)c);
  int Pr;
  if (pa <= pb && pa <= pc)
    Pr = a;
  else if (pb <= pc)
    Pr = b;
  else
    Pr = c;
  return (uint8_t)Pr;
}

#define A ((i >= bpp) ? output[i - bpp] : 0)
#define B ((prevScanline != NULL) ? prevScanline[i] : 0)
#define C ((i >= bpp && prevScanline != NULL) ? prevScanline[i - bpp] : 0)

int scanlineFilterReconstruction(uint8_t *output, uint8_t *prevScanline,
                                 uint8_t *currentScanline, size_t scanlineLen,
                                 uint8_t filterMethod, size_t bpp) {
  //   0       None
  //   1       Sub
  //   2       Up
  //   3       Average
  //   4       Paeth

  if (prevScanline == NULL && (filterMethod != 0 && filterMethod != 1))
    return -1;
  if (currentScanline == NULL)
    return -1;

  switch (filterMethod) {
  case 0: {
    memcpy(output, currentScanline, scanlineLen);
    break;
  }
  case 1: {
    for (int i = 0; i < scanlineLen; i++) {
      output[i] = (uint8_t)(A + currentScanline[i]);
    }
    break;
  }
  case 2: {
    for (int i = 0; i < scanlineLen; i++) {
      output[i] = (uint8_t)(B + currentScanline[i]);
    }
    break;
  }
  case 3: {

    for (int i = 0; i < scanlineLen; i++) {
      output[i] = (uint8_t)((A + B) / 2 + currentScanline[i]);
    }
    break;
  }
  case 4: {
    for (int i = 0; i < scanlineLen; i++) {
      output[i] = (uint8_t)(currentScanline[i] + paethFilter(A, B, C));
    }
    break;
  }
  default:
    return -1; // invalid filter method
  }

  return 0;
};

int IDATDefilter(imagePNG *image, uint8_t *IDATRecon, uint8_t *IDATInfl) {

  if (image == NULL) {
    printf("[ERROR] image struct cannot be NULL");
    return -1;
  }
  if (IDATRecon == NULL) {
    printf("[ERROR] defiltering output cannot be NULL");
    return -1;
  }
  if (IDATInfl == NULL) {
    printf("[ERROR] defiltering input cannot be NULL");
    return -1;
  }

  for (int i = 0; i < image->IHDR.height; i++) {
    uint8_t *prevScanline =
        (i > 0) ? &IDATRecon[(i - 1) * (image->scanlineLen - 1)] : NULL;

    uint8_t *currentOut = &IDATRecon[i * (image->scanlineLen - 1)];
    uint8_t *currentScanline = &IDATInfl[i * image->scanlineLen];

    uint8_t filterMethod = currentScanline[0];

    int ret = scanlineFilterReconstruction(
        currentOut, prevScanline, currentScanline + 1, image->scanlineLen - 1,
        filterMethod, image->bytesPerPx);
    if (ret != 0) {
      printf("[ERROR] scanline defiltering failed");
      return -1;
    }
  }

  return 0;
}

#define RED (IDATRecon[i * bpp])
#define GREEN (IDATRecon[(i * bpp) + 1])
#define BLUE (IDATRecon[(i * bpp) + 2])

int pxParseIDAT(uint8_t *IDATRecon, pixel *pxArr, size_t pxLen,
                uint8_t colorType, size_t bpp) {

  if (IDATRecon == NULL) {
    printf("[ERROR] pixel parsing input cannot be NULL");
    return -1;
  }

  if (IDATRecon == NULL) {
    printf("[ERROR] pixel parsing output cannot be NULL");
    return -1;
  }

  int i;
  switch (colorType) {
  case 0:
    for (i = 0; i < pxLen; i++)
      pxArr[i].grayscale = IDATRecon[i];
    break;
  case 2:
    for (i = 0; i < pxLen; i++) {
      pxArr[i].red = RED;
      pxArr[i].green = GREEN;
      pxArr[i].blue = BLUE;
      pxArr[i].grayscale =
          (uint8_t)(0.299f * RED + 0.587f * GREEN + 0.114f * BLUE);
      printf("grayscale: %d\n", pxArr[i].grayscale);
    }
    break;
  case 3:
    // TODO: Add PLTE support
    break;
  case 4:
    for (i = 0; i < pxLen; i++) {
      pxArr[i].grayscale = IDATRecon[i * bpp];
      pxArr[i].alpha = IDATRecon[(i * bpp) + 1];
    }
    break;
  case 6:
    for (i = 0; i < pxLen; i++) {
      pxArr[i].red = RED;
      pxArr[i].green = GREEN;
      pxArr[i].blue = BLUE;
      pxArr[i].grayscale =
          (uint8_t)(0.299f * RED + 0.587f * GREEN + 0.114f * BLUE);
      pxArr[i].alpha = IDATRecon[(i * bpp) + 3];
    }
    break;

  default:
    printf("[ERROR] invalid color type");
    return -1;
  }
  return 0;
}
