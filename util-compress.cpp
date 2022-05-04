#include <vector>
#include <string>

#include <iostream>
#include <sstream>
#include <iomanip>

#include <bitset>
#include <cstring>

#include "util-compress.h"

class BitBuffer {
private:
    size_t offsetBits;  // in bits
public:
    std::vector<uint8_t> buffer;
    BitBuffer();
    /**
     * Add no more than 8 bits
     * @param value value
     * @param bits bit size
     */
    void push(uint8_t value, uint8_t bits);
    /**
     * Return remainingBits bits
     * @return remainingBits bits 0..7
     */
    uint8_t remainingBits();
    /**
     * Return bytes ready to pop
     * @return byte count ready to pop
     */
    uint8_t size();
    /**
     * pop out byte
     * @return 0 if empty
     */
    uint8_t pop();
    /**
     * Clear
     */
    void clear();
    /**
     * Return bytes as string
     * @return bytes as string
     */
    std::string toHexString();
    /**
     * Return bits as string
     * @return bits as string
     */
    std::string toBinString();
};

BitBuffer::BitBuffer()
    : offsetBits(0)
{
    buffer.reserve(8);
}

/**
 * Add no more than 8 bits
 * @param value value
 * @param bits bit size
 * @return bytes to pop
 */
void BitBuffer::push(uint8_t value, uint8_t bits)
{
    if (bits == 0)
        return;
    // How many bits there left in the current target byte
    uint8_t sz = size();
    while (buffer.size() <= sz)
        buffer.push_back(0);
    uint8_t *cur_dst = &buffer[sz];  // ?!!
    uint8_t bits_left = 8 - offsetBits % 8;

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
    offsetBits += bits_to_use;
    push(value, bits - bits_to_use);
}

/**
 * Return remainingBits bits
 * @return remainingBits bits 0..8
 */
uint8_t BitBuffer::remainingBits()
{
    return 8 - offsetBits % 8;
}

/**
 * Return bytes ready to pop
 * @return byte count ready to pop
 */
uint8_t BitBuffer::size()
{
    return offsetBits / 8;
}

/**
 * pop out byte
 * @return
 */
uint8_t BitBuffer::pop()
{
    uint8_t sz = size();
    uint8_t r;
    if (sz) {
        r = buffer[0];
        offsetBits -= 8;
        // shift array to the left
        buffer.erase(buffer.begin());
    } else
        r = 0;
    return r;
}

/**
 * Return bytes ready to pop
 * @return byte count ready to pop
 */
void BitBuffer::clear()
{
    offsetBits = 0;
    buffer.clear();
}

/**
 * Return bytes as string
 * @return bytes as string
 */
std::string BitBuffer::toHexString()
{
    std::stringstream ss;
    int sz = size();
    if (remainingBits())
        sz++;
    for (int i = 0; i < sz; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (uint) buffer[i];
    }
    return ss.str();
}

/**
 * @see https://stackoverflow.com/questions/111928/is-there-a-printf-converter-to-print-in-binary-format/45041802
 */
static std::string int2bin(int num, int pad)
{
    std::string r;
    r.resize(pad);
    while (--pad >= 0) {
        r[pad] = num & 1 ? '1' : '0';
        num >>= 1;
    }
    return r;
}

/**
 * Return bits as string
 * @return bits as string
 */
std::string BitBuffer::toBinString()
{
    std::stringstream ss;
    int sz = size();
    if (remainingBits())
        sz++;
    for (int i = 0; i < sz; i++) {
        ss << int2bin(buffer[i], 8);
    }
    return ss.str();
}

// длина в битах символа
static uint8_t symblenHafCod[10] = { 1, 4, 5, 6, 7, 7, 5, 5, 4, 2 };
static uint8_t hafCanonCod[] = { 0x01, 0x02, 0x01, 0x01, 0x00, 0x01, 0x02, 0x03, 0x03, 0x01 };

static std::string bits2string(uint8_t value, uint8_t  len) {
    std::bitset<8> bs(value);
    std::string r = bs.to_string();
    r.erase(0, 8 - len);
    return r;
}

static void dbgPrint(
    uint8_t src,
    uint8_t symbol,
    uint8_t symbolCode,
    uint8_t  symbolLength,
    bool isTag,
    uint8_t tagValue
) {
    std::cerr
/*
        << std::hex << std::setw(2) << std::setfill('0')
        << (uint32_t) src
        << ">"
        << (uint32_t) symbol
        << " "
*/
        << bits2string(symbolCode, symbolLength);
    if (isTag)
        std::cerr << "tag(" << std::hex << std::setw(2) << std::setfill('0') << (uint32_t) tagValue << ") "
                  << bits2string(tagValue, 8);
}

#define HUFF_DEBUG_PRINT 1

#ifdef HUFF_DEBUG_PRINT
#define DBGPRINTCODE dbgPrint(inBuffer[i], symbol, symbolCode, symbolLength, tagByte, taggedSymbol);
#define DBGPRINTTREE(i) std::cerr << code2string(hafCanonCod[i], symblenHafCod[i]) << std::endl;
#define DBGPRINTREE10 DBGPRINTTREE(0)    DBGPRINTTREE(1)    DBGPRINTTREE(2)    DBGPRINTTREE(3)    DBGPRINTTREE(4)    DBGPRINTTREE(5)    DBGPRINTTREE(6)    DBGPRINTTREE(7)    DBGPRINTTREE(8)    DBGPRINTTREE(9)
#else
#define DBGPRINTCODE
#define DBGPRINTCODE(i)
#define DBGPRINTREE10
#endif

uint16_t encodeHuffman(
    char *outBuffer,
    uint16_t outSize,
    const char *inBuffer,
    uint16_t inSize
)
{
    BitBuffer bf;

    uint16_t outBufferPos = 0;
    uint8_t taggedSymbol;
    bool tagByte = false;
    uint16_t bitBuffer = 0;
    uint8_t bitsAvailableInBitBuffer = 16;
    for (int i = 0; i < inSize; i++) {
        uint8_t symbol = inBuffer[i];
        if ((symbol <= 4) || (symbol >= 0xFC)) {
            if (symbol > 0xFB)
                symbol = (uint8_t) (8 - (uint8_t) (0xFF - symbol));
        } else {
            tagByte = true;
            symbol = 9;
            taggedSymbol = symbol;
        }

        uint8_t symbolLength = symblenHafCod[symbol];
        uint8_t symbolCode = hafCanonCod[symbol];
        bf.push(symbolCode, symbolLength);
        if (tagByte)
            bf.push(taggedSymbol, 8);
DBGPRINTCODE
        uint8_t tab_mask[] = { 0x00, 0x01, 0x03, 0x07, 0xF, 0x1F, 0x3F, 0x7F, 0xFF };
        if (symbolLength <= bitsAvailableInBitBuffer) {
            // enough space in a bit buffer
            bitBuffer <<= symbolLength;        //0х02 для префикса как инт ! 0111 1111 1111 1111 << 1
            bitBuffer |= (uint16_t)symbolCode;   //0х02 для префикса  //0x0000 | 0x01
            bitsAvailableInBitBuffer = (uint8_t)(bitsAvailableInBitBuffer - symbolLength);     //(bitsAvailableInBitBuffer(15) - symbolLength) - не  может быть минус! только 0 !!
            if ((bitsAvailableInBitBuffer == 0) && (i == (inSize - 1)))  //последний!!
            {
                outBuffer[outBufferPos] = (uint8_t)(bitBuffer >> 8); outBufferPos++;   //msb
                outBuffer[outBufferPos] = (uint8_t)bitBuffer; outBufferPos++;   //lsb
            }
        } else {
            // not enough space in a bit buffer
            bitBuffer <<= bitsAvailableInBitBuffer;      //ост.бит bitsAvailableInBitBuffer <7 сдвиг по оставшимся (может быть уже 0 на 16 символе, 6 или 4 ?!!)
            bitBuffer |= (uint16_t)((symbolCode >> (symbolLength - bitsAvailableInBitBuffer)) & tab_mask[bitsAvailableInBitBuffer]);  //старшие биты! ((1>>(1-0) & 0 ) даст 0!
            outBuffer[outBufferPos] = (uint8_t)(bitBuffer >> 8);    //msb
            outBufferPos++;
            outBuffer[outBufferPos] = (uint8_t)bitBuffer;    //lsb
            outBufferPos++;

            bitBuffer = 0;     // снова пустой битбуфер - 2 байта выведены bitsAvailableInBitBuffer=16; ??
            bitBuffer |= (uint16_t)(symbolCode & tab_mask[symbolLength - bitsAvailableInBitBuffer]);

            bitsAvailableInBitBuffer = (uint8_t) (16 - (uint8_t)(symbolLength - bitsAvailableInBitBuffer)); //  новая длина
        }

        if (tagByte) {
            // write tag
            tagByte = false;
            symbolLength = 8;
            symbolCode = taggedSymbol;  // реальный символ НАДО ТОЖЕ ИНВЕРСИЮ??? чтоб c мл бита брать!!!
            if(symbolLength <= bitsAvailableInBitBuffer) {      // длина меньше остатка битбуфера
                bitBuffer <<= symbolLength;    // 8 перед этим 2            05 0 0 0 0 0 02=0x0C дл.=5
                bitBuffer |= (uint16_t)symbolCode;    //реальный символ 05 всего 10 00000101 11111 01100 = 81 7e ост 1100 обратно
                bitsAvailableInBitBuffer = (uint8_t)(bitsAvailableInBitBuffer - symbolLength);   //еще не выводить
            } else {
                // длина больше остатка битбуфера -заполняется от lsb к msb
                bitBuffer <<= bitsAvailableInBitBuffer; //
                bitBuffer |= (uint16_t)((symbolCode >> (symbolLength - bitsAvailableInBitBuffer)) & tab_mask[bitsAvailableInBitBuffer]);  // старшие биты!
                outBuffer[outBufferPos] = (uint8_t)(bitBuffer >> 8);
                outBufferPos++;
                outBuffer[outBufferPos] = (uint8_t)bitBuffer;
                outBufferPos++;
                bitBuffer = 0;
                bitBuffer |= (uint16_t)(symbolCode & tab_mask[symbolLength - bitsAvailableInBitBuffer]);
                bitsAvailableInBitBuffer = (uint8_t)(16 - (uint8_t)(symbolLength - bitsAvailableInBitBuffer)); //  новая длина
            }
        }
    }

    if ((bitsAvailableInBitBuffer > 0) && (bitsAvailableInBitBuffer != 16)) {
        // незаполненный остаток
        for (int i = 0; i < bitsAvailableInBitBuffer; i++) {
            bitBuffer = (uint16_t)(bitBuffer << 1);
            bitBuffer |= (uint16_t)1;
        }

        outBuffer[outBufferPos] = (uint8_t)(bitBuffer >> 8);
        outBufferPos++;

        if ((16 - bitsAvailableInBitBuffer) > 8) {    // (16- bitsAvailableInBitBuffer) использовано
            outBuffer[outBufferPos] = (uint8_t)bitBuffer;
            outBufferPos++;
        }
    }
    std::cerr
        << std::endl << "===="
        << std::endl << bf.toBinString() << std::endl
        << std::endl << bf.toHexString() << std::endl
        << "====" << std::endl;
    return outBufferPos;
}

// сжатие по Хаффману!
// буфер 2 памяти, в нем дельты с шапкой замера!!!
// в начале шапки замера used два байта -не жмутся! Общая длина дельт 10+2*cnt_mac или 10+cnt_mac
uint16_t compressLogger(
    char *outBuffer,
    size_t outSize,
    const char *inBuffer,
    uint16_t inSize
)
{
    outBuffer[0] = inBuffer[0];
    outBuffer[1] = inBuffer[1];
    return encodeHuffman(outBuffer + 2, outSize - 2, inBuffer + 2, inSize - 2);
}

static uint8_t symbHaf[] = { 0x00, 0x08, 0x01, 0xff, 0x02, 0xfd, 0xfe, 0x03, 0x04, 0xfc };
static uint8_t baseHaf[] = { 0x3f, 1, 1, 2, 2, 1, 1, 0 }; //[0..7] ?=0x3f для декода
static uint8_t offsHaf[] = { 0x3f, 0, 1, 0x3f, 2, 4, 7, 8 };

static uint16_t decodeHuffmanExceptFirst2bytes(
        char *buff_out,
        size_t outSize,
        const char *buff_in,
        uint16_t lenComp
)
{
    //uint16_t i;		//индекс во входном буфере
    uint8_t j;
    uint16_t bufpos = 0;		//индекс во входном буфере!
    uint16_t bufpos_out = 0;		//индекс в выходном буфере!

    uint16_t bitbuf = 0;		// битовый буфер 
    uint8_t symbol;		    //текущий проверяемый символ из входа 
    uint16_t symb_next; 		// след. байт подгрузки - грузит в инт для сджвигов!
    uint8_t priznak = 0;         // префикс
    uint8_t symblen = 0; 		// текущие из массивов
    uint8_t symbcode = 0;  		//код символа 
    uint8_t bits = 16;		// длина - битовый буфер 
    symbol = buff_in[bufpos];      //  2 байта lsb msb used
    bufpos++;
    buff_out[bufpos_out] = symbol;
    bufpos_out++;
    symbol = buff_in[bufpos];         //если 2 байта 
    bufpos++;
    buff_out[bufpos_out] = symbol;
    bufpos_out++;

    bitbuf = (uint16_t)(buff_in[bufpos] * 0x100);
    bufpos++;
    bitbuf |= buff_in[bufpos];      //lsb второй (остальные подргужаются в msb!)
    bufpos++;

    while (bufpos < (lenComp + 2))         // <11  bufpos можен возрасти +2 за цикл!? и не весь разобран! лишняя загр. тк слово!
    {
        uint16_t tabMaskWord[] = { 0x0000, 0xFF00, 0xFE00, 0xFC00, 0xF800, 0xF000, 0xE000, 0xC000, 0x8000 };
        int symblenInd = 1;
        for (j = 8; j > 0; j--)   // 8 7 6 5  4 3 2 1 поиск символа
        {
            uint16_t tempSymb = (uint16_t)(bitbuf & tabMaskWord[j]);  //начало с конца таб.!! 0х8000
            symbol = (uint8_t)(tempSymb >> (8 + j - 1));  // сдвиг 15 14 13 12  11 10 9 8   (7 6 5  4 3 2 1 0)
            if (symblenInd >= 8)   //error!! слишком длинный!
            {
                return 0;    // из while !!! не найден вообще!!
            }
            if (symbol < baseHaf[symblenInd]) //   0 01 00000101 1 1 1 ... < 1
            {
                symblenInd++;  //symblen  2 3 4 5 6 7  8 9
                continue;   //еще не найден 
            }
            else break; //symblen  1 2 3 4 5 6 7   8  найден в табл.
        } // for j = 8
        symbcode = symbHaf[offsHaf[symblenInd] + symbol - baseHaf[symblenInd]]; // [2 + 0010 - 2] symb[2]
        symblen = (uint8_t)symblenInd;

        if(symbcode == 0x08)  priznak = 1;       // префикс - читать след. байт полностью!!

        bitbuf = (uint16_t)(bitbuf << symblen);      // сдвинуть битбуф.
        bits = (uint8_t)(bits - symblen);
        if(bits < 8)   //заранее подгрузить и выровнять байт!!
        {
            symb_next = (uint16_t)(buff_in[bufpos]);    //
            bufpos++;                       //сдесь увелич. bufpos для while!
            symb_next = (uint16_t)(symb_next << (8 - bits));   //
            bitbuf |= (uint16_t)symb_next;
            bits = (uint8_t)(bits + 8);
        }
        if(priznak == 1)            // префикс длина 2 и биты 10 -взять след.8 бит на выход
        {
            priznak = 0;
            symbcode = (uint8_t)((bitbuf & 0xFF00) >> 8);  //

            bits = (uint8_t)(bits - 8);
            bitbuf = (uint16_t)(bitbuf << 8);
            buff_out[bufpos_out] = symbcode;  // запись байта в буфер
            bufpos_out++;
            if (bits < 8)   // подгрузить и выровнять байт!! в lsb !!! bits 16-2-8=6
            {
                symb_next = (uint16_t)(buff_in[bufpos]);    //
                bufpos++;                        //сдесь увелич. bufpos для while! может второй раз за цикл!!!!! 
                symb_next = (uint16_t)(symb_next << (8 - bits));   //
                bitbuf |= (uint16_t)symb_next;     //
                bits = (uint8_t)(bits + 8);         //
            }
        }
        else       // нет признака - сам символ сохранить 
        {
            buff_out[bufpos_out] = symbcode;
            bufpos_out++;
        }

    }  //while
    return bufpos_out;      //  bufpos_out+1 кол. зап.символов (включая used)
}

// декодирование Хафмана
//для Хафмана входной буфер это буфер 1 памяти!!! LOGGER_REC_DELTA , выходной для кодера buff_all + 11 !!
// входной буфер это buff_all + 11 !! выходной  это буфер 1 памяти
//  len = общая длина = compress_len - 9
static unsigned char tabl_haf_symbol[10] = {0, 1, 2, 3, 4, 0xfc, 0xfd, 0xfe, 0xff, 0xAA};

/// декодирование Хафмана - выходной для декодера bufferPack2
//для Хафмана входной буфер это буфер -массив принятого тела замера!!!
// выходной для декодера bufferPack2 (+ 10? !!) на выходе дельта
// входной буфер это массив тела пакета
// len = общая длина = compress_len - 8, на выходе дельта
uint16_t decompressLogger(
    char *outBuffer,
    size_t outSize,
    const char *inBuffer,
    uint16_t lenComp
)
{
    return decodeHuffmanExceptFirst2bytes(outBuffer, outSize, inBuffer, lenComp);
}

std::string compressLoggerString(const std::string &value)
{
    size_t rsz = value.size() * 2;
    std::string retval(rsz, 0);
    uint16_t sz = compressLogger((char *) retval.c_str(), retval.size(), value.c_str(), value.size());
    if (sz < rsz)
        retval.resize(sz);
    return retval;
}

std::string decompressLoggerString(const std::string &value)
{
    size_t rsz = 512;
    std::string retval(rsz, 0);
    uint16_t sz = decompressLogger((char *) retval.c_str(), rsz, value.c_str(), value.size());
    if (sz < rsz)
        retval.resize(sz);
    return retval;
}
