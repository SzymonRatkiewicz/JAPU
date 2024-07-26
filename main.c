#include "japu.h"
#include <errno.h>
#include <stdlib.h>

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
