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

#define DEFAULT_HTML_FILENAME "output.html"

int main(int argc, char **argv) {

  char *imgFilepath;  // input path
  char *outFilepath;  // regular output (txt) path
  char *htmlFilepath; // optional output (html) path

  // flags
  int isImageFilepathSet = 0;
  int isOutFileSet = 0;
  int genHtmlFile = 0;

  if (argc < 2) {
    printf("[ERROR] not enough arguments %d\n", -8);
    displayHelpPage();
    return -8;
  }

  // Handling command-line arguments
  for (int i = 1; i < argc; ++i) {
    if (argv[i][0] == '-') {
      switch (argv[i][1]) {
      case 'w': {
        char *filename = argv[i + 1];

        if (i + 1 > argc) {
          filename = DEFAULT_HTML_FILENAME;
          printf("[WARNING] html filename not specified, defaulting to %s \n",
                 filename);
        }

        htmlFilepath = filename;
        genHtmlFile = 1;

        ++i;
        continue;
      }

      case 'h': {
        displayHelpPage();
        return 0;
      }

      default:
        printf("[ERROR] argument -%c not supported", argv[i][1]);
      }
    } else {
      if (isImageFilepathSet == 0) {
        imgFilepath = argv[i];
        isImageFilepathSet = 1;
        continue;
      }

      if (isOutFileSet == 0) {
        outFilepath = argv[i];
        isOutFileSet = 1;
      }
    }
  }

  // TODO: consider changing it to displaying some help page
  if (imgFilepath == NULL || isImageFilepathSet == 0) {
    printf("[ERROR] file not provided %d\n", -1);
    return -1;
  }

  FILE *image = fopen(imgFilepath, "rb");
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

  if (isOutFileSet == 1) {
    printf("[DEV] creating ascii file\n");
    asciiFileDump(outFilepath, asciiArr, source.IDAT.pxLen, source.IHDR.width);
  }

  if (genHtmlFile == 1) {
    printf("[DEV] creating html file\n");
    htmlFileDump(htmlFilepath, asciiArr, source.IDAT.pxLen, source.IHDR.width);
  }

  free(asciiArr);

  imageFree(&source);

  fclose(image);

  return 0;
}
