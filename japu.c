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
    return 1;
  }

  if ((image->byteLen = ftell(file)) < 0) {
    return 1;
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

  image->IDATCount = hexStreamCountHeaders(IDAT, file);

  return 0;
}

int IHDRDecode(IHDRDecoded *IHDR, FILE *file) {

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
