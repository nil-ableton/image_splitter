#define USAGE_FMT "%s <inputfile> <output_basename>\nSaves every subimage in <inputfile> as .pngs"

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* aprintf(char const *fmt, ...);
char* vaprintf(char const *fmt, va_list input_args);
void bcatf_delim(char** output_str, char const *fmt, ...);
void bfree(char **strbuf);
void die(char const *fmt, ...);

#define guard(expr) if (!(expr)) { exit(-2); }

bool split_images(char const *filename, char const* output_basename, char **error_buf);

int main(int argc, char **argv)
{
    int argi = 1;
    if (argi == argc) die(USAGE_FMT, argv[0]);
    char *inputfile = argv[argi++];
    if (argi == argc) die(USAGE_FMT, argv[0]);
    char *basename = argv[argi++];
    printf("input: %s\n", inputfile);
    printf("basename: %s\n", basename);
    char* error_desc = NULL;
    if (!split_images(inputfile, basename, &error_desc)) {
      printf("ERROR: %s\n", error_desc);
      bfree(&error_desc);
    }
    return 0;
}

void die(char const *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char* buf = vaprintf(fmt, args);
    va_end(args);
    
    printf("ERROR: %s\n", buf);
    free(buf);
    exit(-1);
}

char* vaprintf(char const *fmt, va_list input_args)
{
    va_list args;
    va_copy(args, input_args);
    int len = vsnprintf(NULL, 0, fmt, args);
    guard(len >= 0);
    va_end(args);

    char *buf = calloc(len + 1, 1);
    guard(buf != NULL);

    va_copy(args, input_args);
    vsnprintf(buf, len+1, fmt, args);
    va_end(args);
    return buf;
}


char* aprintf(char const *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char *res = vaprintf(fmt, args);
    va_end(args);
    return res;
}

struct BufHeader
{
  int len;
  char bytes[1];
};

void bfree(char **strbuf)
{
  struct BufHeader *hdr = *strbuf? (struct BufHeader*)(*strbuf - offsetof(struct BufHeader, bytes)) : 0;
  free(hdr);
  *strbuf = NULL;
}

void vbcatf(char **strbuf, char const *fmt, va_list input_args)
{
    va_list args;
    va_copy(args, input_args);
    int len = vsnprintf(NULL, 0, fmt, args);
    guard(len >= 0);
    va_end(args);

    struct BufHeader *hdr = *strbuf? (struct BufHeader*)(*strbuf - offsetof(struct BufHeader, bytes)) : 0;
    if (!hdr) {
      hdr = calloc(sizeof(*hdr) + len + 1, 1);
      guard(hdr);
    } else {
      hdr = realloc(hdr, sizeof(hdr) + hdr->len + len + 1);
      guard(hdr);
      memset(&hdr->bytes[hdr->len], 0, len + 1);
    }
    *strbuf = &hdr->bytes[0];
    va_copy(args, input_args);
    len = vsnprintf(&hdr->bytes[hdr->len], len + 1, fmt, args);
    hdr->len += len>0? len:0;
    va_end(args);
}


void bcatf(char **strbuf, char const* fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  vbcatf(strbuf, fmt, args);
  va_end(args);
}

void bcatf_delim(char** output_str, char const *fmt, ...)
{
  if (*output_str) {
    bcatf(output_str, ", ");
  }
  va_list args;
  va_start(args, fmt);
  vbcatf(output_str, fmt, args);
  va_end(args);
}

#include "loadimg_win32.c"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
