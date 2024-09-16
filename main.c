#include "japu.h"
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

int main(int argc, char **argv) {
  FILE *image = fopen(argv[1], "rb");
  if (image == NULL) {
    printf("[ERROR] FILE %s NOT FOUND %d\n", argv[1], errno);
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

  free(IDATInfl);

  pxParseIDAT(IDATRecon, source.IDAT.pxArr, source.IDAT.pxLen,
              source.IHDR.colorType, source.bytesPerPx);

  free(IDATRecon);

  uint8_t *asciiArr = (uint8_t *)calloc(source.IDAT.pxLen, sizeof(uint8_t));

  asciiImageGenerate(asciiArr, source.IDAT.pxArr, source.IDAT.pxLen);

  // Output raw pixel data in hexes
  asciiFileDump("output.txt", asciiArr, source.IDAT.pxLen, source.IHDR.width);

  htmlFileDump("output.html", asciiArr, source.IDAT.pxLen, source.IHDR.width);

  free(asciiArr);

  imageFree(&source);
  fclose(image);

  return 0;
}
