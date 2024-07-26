#include <errno.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

typedef struct {

  size_t byteLen;
  u_int8_t signature[8];
  u_int32_t width, height, bitDepth, colorType, compressionMethod, filterMethod,
      interlaceMethod;
  u_int8_t IDAT[];

} imagePNG;

void hexDump(u_int8_t *, size_t);

int hexStreamValue(void *, size_t, size_t, FILE *);

int imageInit(imagePNG *, FILE *);

int main() {

  FILE *image = fopen("./resources/gatto.png", "rb");
  if (image == NULL) {

    printf("[ERROR] FILE NOT FOUND %d\n", errno);
    exit(errno);
  }

  imagePNG source;
  int err;
  if ((err = imageInit(&source, image)) != 0) {
    printf("[ERROR] Struct init error %d\n", errno);
    exit(errno);
  }

  fseek(image, 8, SEEK_SET); // skip PNG header
  fseek(image, 4, SEEK_CUR); // skip IHDR length
  fseek(image, 4, SEEK_CUR); // skip IHDR header

  printf("\n%d\n", source.width);
  hexStreamValue(&source.width, 1, 4, image);
  hexStreamValue(&source.height, 1, 4, image);

  printf("\n%d\n", source.width);
  printf("\n%d\n", source.height);
  printf("\n");

  fclose(image);

  return 0;
}

void hexDump(u_int8_t *array, size_t arrayLen) {
  for (int i = 0; i < arrayLen; i++) {
    if (i != 0 && i % 12 == 0) {
      printf("\n");
    }
    printf("%02X, ", array[i]);
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
