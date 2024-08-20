#include "japu.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void hexDump(uint8_t *array, size_t arrLen, size_t width) {

  for (int i = 0; i < arrLen; i++) {
    if (i != 0 && i % (width == 0 ? WIDTH_DEFAULT : width) == 0) {
      printf("\n");
    }
    printf("%02X ", array[i]);
  }
}

int hexStreamValue(void *val, size_t hexSize, size_t arrLen, FILE *file) {

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

  image->IHDR = IHDR;

  if (IHDRDecode(&image->IHDR, file) != 0) {
    printf("[ERROR] Image header decoding failed\n");
    exit(1);
  }

  image->IDATCount = hexStreamCountHeaders(IDAT, file);

  size_t channelSize = 0;

  switch (image->IHDR.colorType) {
  case 0:
    channelSize = 1; // grayscale
    break;
  case 2:
    channelSize = 3; // RGB
    break;
  case 3:
    channelSize = 1; // indexed colors
    break;
  case 4:
    channelSize = 2; // grayscale + alpha channel
    break;
  case 6:
    channelSize = 4; // RGB + alpha channel
    break;
  default:
    return -1;
  }

  image->scanlineLen =
      ((image->IHDR.bitDepth * image->IHDR.width * channelSize) / 8) + 1;

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

int hexStreamSkipHeader(FILE *file) {
  // This function requires file pointer to be at chunks length section

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

#define A ((i - 1 == 0) ? 0 : preFilterScanline[i - 1])
#define B (prevFilteredScanline[i])
#define C ((i - 1 == 0) ? 0 : prevFilteredScanline[i - 1])

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

int scanlineFilterReconstruction(uint8_t *prevFilteredScanline,
                                 uint8_t *preFilterScanline, size_t scanlineLen,
                                 uint8_t filterMethod) {
  //   0       None
  //   1       Sub
  //   2       Up
  //   3       Average
  //   4       Paeth

  if (prevFilteredScanline == NULL && (filterMethod != 0 || filterMethod != 1))
    return -1;
  if (preFilterScanline == NULL)
    return -1;

  switch (filterMethod) {
  case 0: {
    return 0;
    break;
  }
  case 1: {
    for (int i = 1; i < scanlineLen; i++) {
      preFilterScanline[i] = A + preFilterScanline[i];
    }
    break;
  }
  case 2: {
    for (int i = 1; i < scanlineLen; i++) {
      preFilterScanline[i] = B + preFilterScanline[i];
    }
    break;
  }
  case 3: {
    for (int i = 1; i < scanlineLen; i++) {
      preFilterScanline[i] = (A + B) / 2 + preFilterScanline[i];
    }
    break;
  }
  case 4: {
    for (int i = 1; i < scanlineLen; i++) {
      preFilterScanline[i] = paethFilter(A, B, C);
    }
    break;
  }
  default:
    return -1; // invalid filter method
  }

  return 0;
};
