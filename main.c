#include "japu.h"
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

int main() {

  FILE *image = fopen("./resources/img1.png", "rb");
  if (image == NULL) {
    printf("[ERROR] FILE NOT FOUND %d\n", errno);
    exit(errno);
  }

  imagePNG source;
  if ((imageInit(&source, image)) != 0) {
    printf("[ERROR] Struct init error \n");
    exit(1);
  }
  fseek(image, 4, SEEK_CUR); // skip IHDR CRC

  hexStreamConcatIDAT(&source, image);

  uint8_t *IDATInfl =
      (uint8_t *)calloc(source.scanlineLen * source.IHDR.height, 1);

  IDATInflate(&source, IDATInfl);

  uint8_t *IDATRecon =
      (uint8_t *)calloc((source.scanlineLen - 1) * source.IHDR.height, 1);

  IDATDefilter(&source, IDATRecon, IDATInfl);

  // Output raw pixel data in hexes
  hexFileDump("output.txt", IDATRecon,
              (source.scanlineLen - 1) * source.IHDR.height,
              source.scanlineLen - 1);
  //   TODO: replace this manual free with some abstract
  free(IDATRecon);
  free(IDATInfl);
  free(source.IDAT.IDATConcat);
  fclose(image);

  return 0;
}
