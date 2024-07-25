#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#define WIDTH 96
#define HEIGHT 96

int main() {

  FILE *image = fopen("./resources/img1.jpeg", "r");

  if (image == NULL) {

    printf("[ERROR] 404 FILE NOT FOUND\n");
    exit(404);
  }

  fseek(image, 0, SEEK_END);
  size_t fileLength = ftell(image);
  fseek(image, 0, SEEK_SET);

  u_int8_t buffer[fileLength];
  size_t bytesRead = fread(buffer, 8, fileLength / 8, image);

  for (int i = 0; i < fileLength / 8; i++) {

    if (i != 0 && i % 12 == 0) {
      printf("\n");
    }
    printf("%02X, ", buffer[i]);
  }

  size_t bytesLost = fileLength - bytesRead * 8;
  printf("%02X, ", buffer[bytesRead - 1]);
  // TODO: Fix this shit tommorow
  //     /\  /|
  //    / \_/ |
  //   | *  * /
  //   \__^__/
  //
  printf("\n");
  printf("bytes lost: %zu\n", bytesLost);

  fclose(image);

  return 0;
}
