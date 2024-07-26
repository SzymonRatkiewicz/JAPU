#include <errno.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

typedef struct {

  size_t byteLen;
  u_int8_t signature[8];
  u_int8_t width, height, bitDepth, colorType, compressionMethod, filterMethod,
      interlaceMethod;
  u_int8_t IDAT[];

} imagePNG;

void hexDump(u_int8_t *, size_t);

int imageInit(imagePNG *, FILE *);

int main() {

  FILE *image = fopen("./resources/img1.png", "rb");
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
