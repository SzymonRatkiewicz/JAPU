#include "japu.h"
#include <errno.h>
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
    printf("[ERROR] Struct init error %d\n", errno);
    exit(errno);
  }

  fseek(image, 8, SEEK_SET); // skip PNG header
  fseek(image, 4, SEEK_CUR); // skip IHDR length
  fseek(image, 4, SEEK_CUR); // skip IHDR header
  IHDRDecode(&source.IHDR, image);

  fseek(image, 4, SEEK_CUR); // skip IHDR CRC

  hexStreamConcatIDAT(&source, image);

  hexDump(source.IDAT.IDATConcat, source.IDAT.byteLen, 0);

  // TODO: replace this manual free with some abstract
  free(source.IDAT.IDATConcat);

  fclose(image);

  return 0;
}
