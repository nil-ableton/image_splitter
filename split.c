#define USAGE_FMT "%s <inputfile> <output_basename>\nSaves every subimage in <inputfile> as .pngs"

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

char* aprintf(char const *fmt, ...);
char* vaprintf(char const *fmt, va_list input_args);

void die(char const *fmt, ...);
#define guard(expr) if (!(expr)) { exit(-2); }

void split_images(char const *filename, char const* output_basename);

int main(int argc, char **argv)
{
    int argi = 1;
    if (argi == argc) die(USAGE_FMT, argv[0]);
    char *inputfile = argv[argi++];
    if (argi == argc) die(USAGE_FMT, argv[0]);
    char *basename = argv[argi++];
    printf("input: %s\n", inputfile);
    printf("basename: %s\n", basename);
    split_images(inputfile, basename);
    return 0;
}

void die(char const *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    char* buf = vaprintf(fmt, args);
    va_end(args);
    
    printf("ERROR: %s", buf);
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

#include "loadimg_win32.c"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"