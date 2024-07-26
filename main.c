#include "japu.h"
#include <errno.h>
#include <stdlib.h>

int main() {

  FILE *image = fopen("./resources/gatto.png", "rb");
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

  IHDRDecode(&source.IHDR, image);

  fclose(image);

  return 0;
}
