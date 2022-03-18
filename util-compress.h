#ifndef UTIL_COMPRESS_H
#define UTIL_COMPRESS_H 1

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

uint16_t compressLogger(const char *buffIn, char *buffOut, uint16_t len);
uint16_t decompressLogger(const char *buffIn, char *buffOut, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif
