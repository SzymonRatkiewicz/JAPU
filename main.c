#include <math.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#define WIDTH 96
#define HEIGHT 96

int main() {

  FILE *image = fopen("./resources/img1.jpeg", "rb");

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
    printf("%02X, ", buffer[i]);
  }

  fclose(image);

  return 0;
}
