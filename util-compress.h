#ifndef UTIL_COMPRESS_H
#define UTIL_COMPRESS_H 1

#include <inttypes.h>

#ifdef __cplusplus
extern "C" {
#endif

uint16_t compressLogger(
    char *outBuffer,
    size_t outSize,
    const char *inBuffer,
    uint16_t inSize
);

uint16_t decompressLogger(
    char *outBuffer,
    size_t outSize,
    const char *inBuffer,
    uint16_t inSize
);

#ifdef __cplusplus
}
#endif

#endif
