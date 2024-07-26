#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

void hexDump(u_int8_t *, size_t);

int main() {

  FILE *image = fopen("./resources/gatto.png", "rb");

  if (image == NULL) {

    printf("[ERROR] 404 FILE NOT FOUND\n");
    exit(404);
  }

  fseek(image, 0, SEEK_END);
  size_t fileLength = ftell(image);
  fseek(image, 0, SEEK_SET);

  u_int8_t buffer[fileLength];
  size_t bytesRead = fread(buffer, 8, fileLength, image);
  u_int8_t additionalBytes = 0;

  for (int i = 0; i < fileLength; i++) {
    if (i != 0 && i % 12 == 0) {
      printf("\n");
    }
    printf("[%d] %02X, ", i, buffer[i]);
  }

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
