#include <vector>
#include <string>

#include <sstream>
#include <cstring>

#include "util-compress.h"

class HuffmanTreeNode {
public:
    uint8_t *symbol;	// symbol (if terminal), otherwise NULL
    HuffmanTreeNode* left;
    HuffmanTreeNode* right;
};

//  Index               0  1  2  3  4  5  6     7     8     9
uint8_t symbols[10] = { 0, 1, 2, 3, 4, 8, 0xfc, 0xfd, 0xfe, 0xff };
// Huffman canonical tree
HuffmanTreeNode n0000001 = {&symbols[6], nullptr, nullptr };    // fc
HuffmanTreeNode n0000000 = { &symbols[4], nullptr, nullptr };   // 4
HuffmanTreeNode n000001 = {&symbols[3], nullptr, nullptr };     // 3
HuffmanTreeNode n000000 = {nullptr, &n0000000, &n0000001 };
HuffmanTreeNode n00001 = {&symbols[2], nullptr, nullptr };      // 2
HuffmanTreeNode n00000 = {nullptr, &n000000, &n000001 };
HuffmanTreeNode n0000 = {nullptr, &n00000, &n00001 };
HuffmanTreeNode n00011 = {&symbols[8], nullptr, nullptr };      // fe
HuffmanTreeNode n00010 = {&symbols[7], nullptr, nullptr };      // fd
HuffmanTreeNode n0001 = { nullptr, &n00010, &n00011 };
HuffmanTreeNode n0011 = {&symbols[9], nullptr, nullptr };       // ff
HuffmanTreeNode n0010 = {&symbols[1], nullptr, nullptr };       // 1
HuffmanTreeNode n001 = {nullptr, &n0010, &n0011 };
HuffmanTreeNode n000 = {nullptr, &n0000, &n0001 };
HuffmanTreeNode n01 = {&symbols[5], nullptr, nullptr};          // prefix
HuffmanTreeNode n00 = {nullptr, &n000, &n001};
HuffmanTreeNode n0 = {nullptr, &n00, &n01};
HuffmanTreeNode n1 = {&symbols[0], nullptr, nullptr};
HuffmanTreeNode huffmanTreeRoot = {nullptr, &n0, &n1};

class BitBufferWriter {
public:
    size_t bitCount;  // size in bits
    std::vector<uint8_t> buffer;
    BitBufferWriter();
    void push(uint8_t value, uint8_t bits);
    /**
     * Return bytes ready to pop
     * @return byte count ready to pop
     */
    size_t size() const;
    /**
     * Return total size in bits
     * @return bits
     */
    size_t bitSize() const ;
    /**
     * pop out byte
     * @return popped byte, 0 if empty
     */
    uint8_t pop();
    /**
     * The last byte can contain 1..8 bits and 0..7 unused garbage bits.
     * To avoid unpacking dummy data, appendPaddingBits() adds
     * labeled byte prefix of 01<byte> sequence.
     * Since there are 7 free bits, there is no room to decode the tag byte, and
     * junk bits are not decoded.
     **/
    void appendPaddingBits();
};

class BitBufferReader {
public:
    size_t bitCount;  // size in bits
    const uint8_t *buffer;
    size_t bitPosition;  // in bits
    BitBufferReader(const void *src, size_t sizeInBits);
    /**
     * Peek 1..8 bits
     * @param bits 1..8
     * @param offset offset in bits
     * @param accum accumulator
     * @return byte value
     */
    uint32_t peek(uint8_t bitSize, size_t offset, uint32_t accum = 0);
    /**
     * Check does buffer has bits at current position
     * Move position if it does
     * @param bits 1..8
     * @param hasBits return has bits
     * @return true if bits exists
     */
    uint32_t pop(uint8_t bits, bool &hasBits);
    /**
     * pop 1 bit
     * @param bits 1..8
     * @return byte value
     */
    uint32_t pop1(bool &hasBits);
};

class BitStreamReader {
public:
    std::istream &strm;
    uint8_t src;
    size_t lastBitPosition;  // in bits
    size_t lastBytePosition;
    bool reachEof;
    BitStreamReader(std::istream &aStrm);
    /**
     * Peek 1..8 bits
     * @param bits 1..8
     * @param offset offset in bits
     * @param accum accumulator
     * @return byte value
     */
    uint32_t peek(uint8_t bitSize, size_t offset, uint32_t accum = 0);
    /**
     * Check does buffer has bits at current position
     * Move position if it does
     * @param bits 1..8
     * @return true if bits exists
     */
    uint32_t pop(uint8_t bits);
    /**
     * pop 1 bit
     * @return byte value
     */
    uint32_t pop1();
};

BitBufferWriter::BitBufferWriter()
    : bitCount(0)
{
    buffer.reserve(16);
}

BitBufferReader::BitBufferReader(
    const void *src,
    size_t sizeInBits
)
    : bitCount(sizeInBits), bitPosition(0)
{
    buffer = (const uint8_t *) src;
}

/**
 * Peek 1..8 bits
 * @param bitSize 1..8
 * @param offset offset in bits
 * @param accum accumulator
 * @return byte value
 */
uint32_t BitBufferReader::peek(uint8_t bitSize, size_t offset, uint32_t accum)
{
    if (bitSize == 0)                  // nothing to do
        return accum;
    if (offset + bitSize > bitCount)  // range out
        return accum;

    // Look at the source, how many bits there left in the current byte
    const uint8_t *src = buffer + offset / 8;
    uint8_t bitsLeft = 8 - offset % 8;
    const uint8_t cur_data = *src << (8 - bitsLeft);

    // How many bits we need and can write now
    uint8_t bits_to_use = bitsLeft < bitSize ? bitsLeft : bitSize;

    // Write the desired bits to the accumulator
    accum <<= bits_to_use;
    uint8_t mask = (1 << bits_to_use) - 1;
    uint8_t off = 8 - bits_to_use;
    accum |= (cur_data & (mask << off)) >> off;

    // Tail-recurse into the rest of required bits
    return peek(bitSize - bits_to_use, offset + bits_to_use, accum);
}

/**
 * pop 1..8 bits
 * @param bits 1..8
 * @return byte value
 */
uint32_t BitBufferReader::pop(uint8_t bits, bool &hasBits)
{
    hasBits = bitPosition + bits <= bitCount;
    if (!hasBits)
        return 0;
    uint32_t r = peek(bits, bitPosition, 0);
    bitPosition += bits;
    return r;
}

/**
 * pop one bit
 * @return byte value
 */
uint32_t BitBufferReader::pop1(bool &hasBits)
{
    hasBits = bitPosition < bitCount;
    if (!hasBits)
        return 0;
    uint32_t r = peek(1, bitPosition, 0);
    bitPosition++;
    return r;
}

BitStreamReader::BitStreamReader(
    std::istream &aStrm
)
    : strm(aStrm), lastBitPosition(0), lastBytePosition(0), reachEof(false)
{
    src = strm.get();   // make first step
}

/**
 * Peek 1..8 bits
 * @param bitSize 1..8
 * @param offset offset in bits
 * @param accum accumulator
 * @return byte value
 */
uint32_t BitStreamReader::peek(uint8_t bitSize, size_t offset, uint32_t accum)
{
    if (bitSize == 0)                  // nothing to do
        return accum;
    if ((offset / 8) > lastBytePosition) {
        src = strm.get();   // move on
        lastBytePosition++;
        if (strm.eof()) {
            reachEof = true;
            return accum;   // range out
        }
    }
    uint8_t bitsLeft = 8 - offset % 8;
    const uint8_t cur_data = src << (8 - bitsLeft);

    // How many bits we need and can write now
    uint8_t bits_to_use = bitsLeft < bitSize ? bitsLeft : bitSize;

    // Write the desired bits to the accumulator
    accum <<= bits_to_use;
    uint8_t mask = (1 << bits_to_use) - 1;
    uint8_t off = 8 - bits_to_use;
    accum |= (cur_data & (mask << off)) >> off;

    // Tail-recurse into the rest of required bits
    return peek(bitSize - bits_to_use, offset + bits_to_use, accum);
}

/**
 * pop 1..8 bits
 * @param bits 1..8
 * @return byte value
 */
uint32_t BitStreamReader::pop(uint8_t bits)
{
    uint32_t r = peek(bits, lastBitPosition, 0);
    lastBitPosition += bits;
    if (reachEof)
        return 0;
    return r;
}

/**
 * pop one bit
 * @return byte value
 */
uint32_t BitStreamReader::pop1()
{
    uint32_t r = peek(1, lastBitPosition, 0);
    lastBitPosition++;
    if (reachEof)
        return 0;
    return r;
}

/**
 * Add no more than 8 bits
 * @param value value
 * @param bits bit size
 * @return bytes to pop
 */
void BitBufferWriter::push(uint8_t value, uint8_t bits)
{
    if (bits == 0)
        return;
    // How many bits there left in the current target byte
    uint8_t sz = size();
    while (buffer.size() <= sz)
        buffer.push_back(0);
    uint8_t *cur_dst = &buffer[sz];  // ?!!
    uint8_t bits_left = 8 - bitCount % 8;

    // How many bits are necessary and the mask for that count of bits
    uint8_t bits_to_use = bits_left < bits ? bits_left : bits;
    const uint8_t mask = (1 << bits_to_use) - 1;

    // The desired range of bits from the source value
    uint8_t value_off = bits - bits_to_use;
    const uint8_t value_bits = (value >> value_off) & mask;

    // Writing the bits to the destination
    uint8_t cur_data = *cur_dst;
    uint8_t off = bits_left - bits_to_use;
    cur_data &= ~(mask << off);
    cur_data |= value_bits << off;
    *cur_dst = cur_data;

    // Tail recursion to the rest of data
    bitCount += bits_to_use;
    push(value, bits - bits_to_use);
}

/**
 * Return bytes ready to pop
 * @return byte count ready to pop
 */
size_t BitBufferWriter::size() const
{
    return bitCount / 8;
}

/**
 * Return total size in bits
 * @return bits
 */
size_t BitBufferWriter::bitSize() const
{
    return bitCount;
}

/**
 * pop out byte
 * @return byte
 */
uint8_t BitBufferWriter::pop()
{
    uint8_t sz = size();
    uint8_t r;
    if (sz) {
        r = buffer[0];
        bitCount -= 8;
        // shift array to the left
        buffer.erase(buffer.begin());
    } else
        r = 0;
    return r;
}

/**
 * add trailing bits meaning nothing
 */
void BitBufferWriter::appendPaddingBits()
{
    size_t rsz = bitCount % 8;
    if (rsz) {
        size_t sz = size();
        uint8_t m = 0x55 >> rsz;
        buffer[sz] |= m;
    }
}

/*
 * Huffman canonical codes
 * 0  1
 * 1  0010
 * 2  00001
 * 3  000001
 * 4  0000000
 * fc 0000001
 * fd 00010
 * fe 00011
 * ff 0011
 * 8 01 prefix of the byte itself (which is not in the table)
 */

// Huffman code length in bits
static uint8_t huffmanCodeBitLength[10] = { 1, 4, 5, 6, 7, 7, 5, 5, 4, 2 };
// Huffman codes
static uint8_t huffmanCodes[10] = { 1, 2, 1, 1, 0, 1, 2, 3, 3, 1 };

/**
 * Encode stream
 * @param outStrm output stream
 * @param inStrm input stream
 * @return
 */
size_t encodeHuffmanStream(
    std::ostream &outStrm,
    std::istream &inStrm
)
{
    BitBufferWriter bf;
    uint8_t taggedSymbol;
    while(!inStrm.eof()) {
        uint8_t symbol = inStrm.get();
        bool symbolInTable = (symbol <= 4) || (symbol >= 0xfc);
        if (symbolInTable) {
            if (symbol > 0xFB)
                symbol = (uint8_t) (8 - (uint8_t) (0xFF - symbol));
        } else {
            taggedSymbol = symbol;
            symbol = 9;
        }
        uint8_t symbolLength = huffmanCodeBitLength[symbol];
        uint8_t symbolCode = huffmanCodes[symbol];
        bf.push(symbolCode, symbolLength);
        if (!symbolInTable)
            bf.push(taggedSymbol, 8);
    }
    bf.appendPaddingBits();
    for (int i = 0; i < bf.buffer.size(); i++)
        outStrm.put(bf.buffer[i]);
    return bf.bitSize();
}

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
)
{
    BitBufferWriter bf;
    uint8_t taggedSymbol;
    for (int i = 0; i < srcSize; i++) {
        uint8_t symbol = srcBuffer[i];
        bool symbolInTable = (symbol <= 4) || (symbol >= 0xfc);
        if (symbolInTable) {
            if (symbol > 0xFB)
                symbol = (uint8_t) (8 - (uint8_t) (0xFF - symbol));
        } else {
            taggedSymbol = symbol;
            symbol = 9;
        }
        uint8_t symbolLength = huffmanCodeBitLength[symbol];
        uint8_t symbolCode = huffmanCodes[symbol];
        bf.push(symbolCode, symbolLength);
        if (!symbolInTable)
            bf.push(taggedSymbol, 8);
    }
    bf.appendPaddingBits();
    for (int i = 0; i < bf.buffer.size(); i++)
        outStrm.put(bf.buffer[i]);
    return bf.bitSize();
}

/**
 * Logger compress skip first two bytes
 * @param outStrm output stream
 * @param srcBuffer source
 * @param srcSize size in bytes
 * @return bits in the stream
 */
size_t compressLogger(
    std::ostream &outStrm,
    const void *srcBuffer,
    size_t srcSize
)
{
    if (srcSize > 0)
        outStrm.put(((const char *) srcBuffer)[0]);
    if (srcSize > 1)
        outStrm.put(((const char *) srcBuffer)[1]);
    return encodeHuffman(outStrm, ((const char *) srcBuffer) + 2, srcSize - 2);
}

/**
 * Decompress stream using Huffman tree
 * @param outStrm output stream
 * @param inStrm source stream
 * @return bits in the stream
 */
size_t decodeHuffmanStream(
    std::ostream &outStrm,
    std::istream &inStrm
)
{
    BitStreamReader bf(inStrm);
    HuffmanTreeNode *n = &huffmanTreeRoot;
    size_t r = outStrm.tellp();
    while (true) {
        uint32_t lr = bf.pop1();
        if (bf.reachEof)
            break;
        if (lr)
            n = n->right;
        else
            n = n->left;
        if (n->symbol) {
            // terminal
            if (*n->symbol == 8) {
                // tag, read next byte
                uint8_t v = bf.pop(8);
                if (bf.reachEof)
                    break;
                // store byte
                outStrm.put(v);
            } else
                outStrm.put(*n->symbol);
            // start from beginning
            n = &huffmanTreeRoot;
        }
    }
    size_t r2 = outStrm.tellp();
    return r2 - r;
}

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
)
{
    BitBufferReader bf(srcBuffer, srcSize * 8);
    HuffmanTreeNode *n = &huffmanTreeRoot;
    bool hasBits;
    size_t r = outStrm.tellp();
    while (true) {
        uint32_t lr = bf.pop1(hasBits);
        if (!hasBits)
            break;
        if (lr)
            n = n->right;
        else
            n = n->left;
        if (n->symbol) {
            // terminal
            if (*n->symbol == 8) {
                // tag, read next byte
                uint8_t v = bf.pop(8, hasBits);
                if (!hasBits)
                    break;
                // store byte
                outStrm.put(v);
            } else
                outStrm.put(*n->symbol);
            // start from beginning
            n = &huffmanTreeRoot;
        }
    }
    size_t r2 = outStrm.tellp();
    return r2 - r;
}

/**
 * Logger decompress skip first two bytes
 * @param outStrm output stream
 * @param srcBuffer source
 * @param srcSize size in bytes
 * @return bits in the stream
 */
size_t decompressLogger(
    std::ostream &outStrm,
    const void *srcBuffer,
    size_t srcSize
)
{
    if (srcSize > 0)
        outStrm.put(((const char *) srcBuffer)[0]);
    if (srcSize > 1)
        outStrm.put(((const char *) srcBuffer)[1]);
    return decodeHuffman(outStrm, ((const char *) srcBuffer) + 2, srcSize  - 2);
}

/**
 * Compress buffer skipping first two bytes
 * @param outBuffer output buffer
 * @param outSize buffer size
 * @param srcBuffer data to compress
 * @param srcSize data size in bytes
 * @return compressed size in bits not bytes
 */
size_t compressLoggerBuffer(
    char *outBuffer,
    size_t outSize,
    const char *srcBuffer,
    uint16_t srcSize
)
{
    std::ostringstream ss(std::string(outBuffer, outSize));
    return compressLogger(ss, srcBuffer, srcSize);
}

/**
 * Decompress buffer skipping first two bytes
 * @param outBuffer output buffer
 * @param outSize buffer size
 * @param srcBuffer data to compress
 * @param srcSize data size in bytes
 * @return size in bytes
 */
size_t decompressLoggerBuffer(
    char *outBuffer,
    size_t outSize,
    const char *srcBuffer,
    size_t srcSize
)
{
    std::ostringstream ss(std::string(outBuffer, outSize));
    return decompressLogger(ss, srcBuffer, srcSize);
}

/**
 * Compress data stored in the string skipping first two bytes
 * @param value data to compress
 * @return compressed data
 */
std::string compressLoggerString(const std::string &value)
{
    std::ostringstream ss;
    compressLogger(ss, value.c_str(), value.size());
    return ss.str();
}

/**
 * Decompress data stored in the string skipping first two bytes
 * @param value data to decompress
 * @return decompressed data
 */
std::string decompressLoggerString(const std::string &value)
{
    std::ostringstream ss;
    decompressLogger(ss, value.c_str(), value.size());
    return ss.str();
}
