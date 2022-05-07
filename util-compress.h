#ifndef UTIL_COMPRESS_H
#define UTIL_COMPRESS_H 1

#include <inttypes.h>

#ifdef __cplusplus
extern "C++" {

/**
 * Encode stream
 * @param outStrm output stream
 * @param inStrm input stream
 * @return
 */
size_t encodeHuffmanStream(
        std::ostream &outStrm,
        std::istream &inStrm
);

/**
 * Encode buffer
 * @param outStrm output stream
 * @param srcBuffer input buffer
 * @param srcSize input buffer size in bytes
 * @return
 */
size_t encodeHuffman(
    std::ostream &outStrm,
    const char *srcBuffer,
    size_t srcSize
);

/**
 * Decompress buffer using Huffman tree
 * @param outStrm output stream
 * @param srcBuffer source
 * @param srcSize size in bytes
 * @return bits in the stream
 */
size_t decodeHuffman(
    std::ostream &outStrm,
    const char *srcBuffer,
    size_t srcSize
);

/**
 * Decompress stream using Huffman tree
 * @param outStrm output stream
 * @param inStrm source stream
 * @return bits in the stream
 */
size_t decodeHuffmanStream(
    std::ostream &outStrm,
    std::istream &inStrm
);

/**
 * Logger compress
 * @param outStrm output stream
 * @param srcBuffer source
 * @param srcSize size in bytes
 * @return bits in the stream
 */
size_t compressLogger(
    std::ostream &outStrm,
    const void *srcBuffer,
    size_t srcSize
);

/**
 * Decompress logger buffer using Huffman tree
 * @param outStrm output stream
 * @param srcBuffer source
 * @param srcSize size in bytes
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
