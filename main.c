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

  //  divide inflated data into scanlines

  for (int i = 0; i < source.IHDR.height; i++) {
    uint8_t *prevFilteredScanline =
        (i > 0) ? &IDATInfl[(i - 1) * source.scanlineLen] : NULL;
    uint8_t *currentScanline = &IDATInfl[i * source.scanlineLen];
    uint8_t filterMethod = currentScanline[0];
    scanlineFilterReconstruction(prevFilteredScanline, currentScanline,
                                 source.scanlineLen, filterMethod);
  }

  hexDump(IDATInfl, source.scanlineLen * source.IHDR.height,
          source.scanlineLen);
  //   TODO: replace this manual free with some abstract
  free(IDATInfl);
  free(source.IDAT.IDATConcat);
  fclose(image);

  return 0;
}
