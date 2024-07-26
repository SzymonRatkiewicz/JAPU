#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

typedef struct {

  size_t byteLen;
  u_int8_t signature[8];
  u_int width, height, bitDepth, colorType, compressionMethod, filterMethod,
      interlaceMethod;
  u_int8_t IDAT[];

} imagePNG;

void hexDump(u_int8_t *, size_t);

size_t hexRead(u_int *, size_t, size_t, FILE *);

int main() {

  imagePNG source;

  FILE *image = fopen("./resources/img1.png", "rb");
  if (image == NULL) {

    printf("[ERROR] 404 FILE NOT FOUND\n");
    exit(404);
  }

  fseek(image, 0, SEEK_END);
  source.byteLen = ftell(image);
  fseek(image, 0, SEEK_SET);

  size_t bytesRead =
      fread(source.signature, 8, 8, image) + fseek(image, 8, SEEK_CUR) +

      fread(&source.width, 8, 4, image) + fread(&source.height, 8, 4, image) +
      fread(&source.bitDepth, 8, 1, image) +
      fread(&source.colorType, 8, 1, image) +
      fread(&source.compressionMethod, 8, 1, image) +
      fread(&source.filterMethod, 8, 1, image) +
      fread(&source.interlaceMethod, 8, 1, image);

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

size_t hexRead(u_int *var, size_t size, size_t len, FILE *file) {

  for (int i = 0; i < size * len; i++) {

    var += fgetc(file);
  }
}
