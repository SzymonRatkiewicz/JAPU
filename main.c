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

  z_stream stream;
  stream.zalloc = Z_NULL;
  stream.zfree = Z_NULL;
  stream.opaque = Z_NULL;

  stream.avail_in = source.IDAT.byteLen;
  stream.next_in = source.IDAT.IDATConcat;
  stream.avail_out = source.IDAT.byteLen * 4;
  stream.next_out = IDATInfl;

  if (inflateInit(&stream) != Z_OK) {
    fprintf(stderr, "[ERROR] inflateInit failed!\n");
    return -1;
  }

  int ret = inflate(&stream, Z_NO_FLUSH);
  if (ret != Z_STREAM_END) {
    fprintf(stderr, "[ERROR] inflate failed: %d\n", ret);
    inflateEnd(&stream);
    return -1;
  }

  inflateEnd(&stream);

  //  divide inflated data into scanlines

  for (int i = 0; i < source.IHDR.height; i++) {
    uint8_t *prevFilteredScanline =
        (i > 0) ? &IDATInfl[(i - 1) * source.scanlineLen] : NULL;
    uint8_t *currentScanline = &IDATInfl[i * source.scanlineLen];
    uint8_t filterMethod = currentScanline[0];
    scanlineFilterReconstruction(prevFilteredScanline, currentScanline,
                                 source.scanlineLen, filterMethod);
  }
  printf("%zu\n", source.scanlineLen);
  hexDump(IDATInfl, source.scanlineLen * source.IHDR.height,
          source.scanlineLen);
  //   TODO: replace this manual free with some abstract
  free(IDATInfl);
  free(source.IDAT.IDATConcat);
  fclose(image);

  return 0;
}
