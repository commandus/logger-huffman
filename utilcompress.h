#ifndef UTIL_COMPRESS_H
#define UTIL_COMPRESS_H 1

#include "inttypes.h"

uint16_t compressLogger(const unsigned char *buffIn, unsigned char *buffOut, uint16_t len);

uint16_t decompressLogger(const unsigned char *buffIn, unsigned char *buffOut, uint16_t len);

#endif
