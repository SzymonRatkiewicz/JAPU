#include "japu.h"
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

/*
 * JAPU ERR Code table
 * -1 => file not provided
 * -2 => image init failure
 * -3 => IDAT concatenation error
 * -4 => IDAT inflation error
 * -5 => IDAT defiltering error
 * -6 => pixel parsing error
 * -7 => ascii generation error
 * -8 => not enough arguments are given
 */

int main(int argc, char **argv) {

  // TODO: consider changing it to displaying some help page
  if (argc < 1) {
    printf("[ERROR] not enough arguments %d\n", -8);
    return -8;
  }

  if (argv[1] == NULL) {
    printf("[ERROR] file not provided %d\n", -1);
    return -1;
  }

  FILE *image = fopen(argv[1], "rb");
  if (image == NULL) {
    printf("[ERROR] FILE %s NOT FOUND %d\n", argv[1], errno);
    exit(errno);
  }

  int ret;
  imagePNG source;
  if ((ret = imageInit(&source, image)) != 0) {
    printf("[ERROR] Struct init error %d\n", ret);
    fclose(image);
    return -2;
  }
  fseek(image, 4, SEEK_CUR); // skip IHDR CRC

  if ((ret = hexStreamConcatIDAT(&source, image)) != 0) {
    printf("[ERROR] IDAT concat error %d\n", ret);
    IDATConcatFree(&source);
    fclose(image);
    return -3;
  }

  uint8_t *IDATInfl =
      (uint8_t *)calloc(source.scanlineLen * source.IHDR.height, 1);

  if ((ret = IDATInflate(&source, IDATInfl)) != 0) {
    printf("[ERROR] IDAT inflate error %d\n", ret);
    fclose(image);
    IDATConcatFree(&source);
    free(IDATInfl);
    return -4;
  }

  IDATConcatFree(&source);

  uint8_t *IDATRecon =
      (uint8_t *)calloc((source.scanlineLen - 1) * source.IHDR.height, 1);

  if ((ret = IDATDefilter(&source, IDATRecon, IDATInfl)) != 0) {
    printf("[ERROR] IDAT defilter error %d\n", ret);
    fclose(image);
    IDATConcatFree(&source);
    free(IDATInfl);
    free(IDATRecon);
    return -5;
  }

  free(IDATInfl);

  if ((ret = pxParseIDAT(IDATRecon, source.IDAT.pxArr, source.IDAT.pxLen,
                         source.IHDR.colorType, source.bytesPerPx)) != 0) {
    printf("[ERROR] pixel parse IDAT error %d\n", ret);
    fclose(image);
    IDATConcatFree(&source);
    free(IDATRecon);
    return -6;
  }

  free(IDATRecon);

  uint8_t *asciiArr = (uint8_t *)calloc(source.IDAT.pxLen, sizeof(uint8_t));

  if ((ret = asciiImageGenerate(asciiArr, source.IDAT.pxArr,
                                source.IDAT.pxLen)) != 0) {
    printf("[ERROR] ascii image generate error %d\n", ret);
    fclose(image);
    IDATConcatFree(&source);
    free(IDATRecon);
    return -7;
  }

  // will exit in case of file system failure
  asciiFileDump("output.txt", asciiArr, source.IDAT.pxLen, source.IHDR.width);

  // will exit in case of file system failure
  htmlFileDump("output.html", asciiArr, source.IDAT.pxLen, source.IHDR.width);

  free(asciiArr);

  imageFree(&source);

  fclose(image);

  return 0;
}
