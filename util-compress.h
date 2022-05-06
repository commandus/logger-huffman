#ifndef UTIL_COMPRESS_H
#define UTIL_COMPRESS_H 1

#include <inttypes.h>

#ifdef __cplusplus
extern "C++" {
/**
 * Logger compress
 * @param outStrm output stream
 * @param srcBuffer source
 * @param srcSize szie
 * @return bits in the stream
 */
size_t compressLogger(
        std::ostream &outStrm,
        const void *srcBuffer,
        size_t srcSize
);

/**
 * Logger decompress
 * @param outStrm output stream
 * @param srcBuffer source
 * @param srcSize szie
 * @return bits in the stream
 */
size_t decompressLogger(
        std::ostream &outStrm,
        const void *srcBuffer,
        size_t srcSize
);

/**
 * Logger compress string
 * @param value source
 * @return compressed string
 */
std::string compressLoggerString(
        const std::string &value
);

/**
 * Logger decompress string
 * @param value source
 * @return decompressed string
 */
std::string decompressLoggerString(
        const std::string &value
);

}
#endif

#ifdef __cplusplus
extern "C" {
#endif

size_t compressLoggerBuffer(
    char *outBuffer,
    size_t outSize,
    const char *srcBuffer,
    uint16_t srcSize
);

size_t decompressLoggerBuffer(
    char *outBuffer,
    size_t outSize,
    const char *srcBuffer,
    uint16_t srcSize
);

#ifdef __cplusplus
}
#endif

#endif
