#include "japu.h"
#include <errno.h>
#include <math.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>

#define WIDTH_DEFAULT 16

int main() {

  FILE *image = fopen("./resources/img1.png", "rb");
  if (image == NULL) {
    printf("[ERROR] FILE NOT FOUND %d\n", errno);
    exit(errno);
  }

  imagePNG source;
  if ((imageInit(&source, image)) != 0) {
    printf("[ERROR] Struct init error %d\n", errno);
    exit(errno);
  }

  fseek(image, 8, SEEK_SET); // skip PNG header
  fseek(image, 4, SEEK_CUR); // skip IHDR length
  fseek(image, 4, SEEK_CUR); // skip IHDR header

  hexStreamValue(&source.width, 1, 4, image);
  hexStreamValue(&source.height, 1, 4, image);

  printf("Width: %d\n", source.width);
  printf("Heigh: %d\n", source.height);

  u_int8_t randomShit[300];
  fread(randomShit, 1, 300, image);

  hexDump(randomShit, 300, source.width);

  fclose(image);

  return 0;
}

void hexDump(u_int8_t *array, size_t arrayLen, size_t width) {
  for (int i = 0; i < arrayLen; i++) {
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

  u_int64_t res = 0;
  for (int i = 0; i < hexSize * arrLen; i++) {
    res |= *((u_int8_t *)hexArr + i) << ((hexSize * arrLen - 1 - i) * 8);
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

  image->width = 0;
  image->height = 0;
  image->bitDepth = 0;
  image->colorType = 0;
  image->compressionMethod = 0;
  image->filterMethod = 0;
  image->interlaceMethod = 0;

  return 0;
}
