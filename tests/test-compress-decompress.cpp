#include <string>
#include <cassert>
#include <iostream>
#include <sstream>
#include <sys/time.h>

#include "logger-huffman.h"
#include "util-time-fmt.h"
#include "logger-collection.h"
#include "logger-builder.h"

#define CNT 25

static std::string sCompressedData[CNT] = {
        "4C03090004011C14040000030BFF319B00",
        "4C02070005011C14050000030BFF31",
        "4C02090006011C14060000030BFE444400",
        "4C03090007011C14070000030BF2CCCF00",
        "4C030A0008011C14080000030BE5C633007F",

        "4C02090009011C14090000030BE5CCCF00",
        "4C0208000A011C140A0000030BE5E667",
        "4C030A000B011C140B0000030BF2C622007F",
        "4C040D000C011C140C0000030BF2DFA6E9FA7E5A81",
        "4C040D000D011C140D0000030BF2DFB6E9FA7E9B81",

        "4C0B0F000E011C140E0000030BFF7E8DFA36E8D9936700",
        "4C050D000F011C140F0000030BFFFC380FC2ECF893",
        "4C050D0010011C14100000030BE5DF66D9F67D5793",
        "4C050D0011011C14110000030BE5DF66D9F67D5693",
        "4C050D0012011C14120000030BE45BEDFB36CFAB93",

        "4C050D0013011C14130000030BE5DF66D9F67D5693",
        "4C050D0014011C14140000030BE45BEFFBB7EFB393",
        "4C050D0015011C14150000030BF2DF77DDF77D9793",
        "4C050D0016011C14160000030BF2DF95E5F97E1993",
        "4C050D0017011C14170000030BF2DF84E1F77D9893",

        "4C050D0018011C14180000030BFFFBB7EFBBEAF793",
        "4C050D0019011C14190000030BFFFB36CFB3EAF693",
        "4C050D001A011C141A0000030BFFFB36CFB3EAF693",
        "4C050D001B011C141B0000030BE5DF66D9F67D5793",
        "4C050D001C011C141C0000030BFFFC380FC2ECF893"
};

void testDecompress(
    const std::string &value
) {
    std::cout << "Compressed   " << bin2hexString(value) << ", size: " << value.size() << std::endl;
    std::string s = decompressLoggerString(value);
    std::cout << "Decompressed " << bin2hexString(s) << ", size: " << s.size() << std::endl;
}

void testDecompress2() {
    for (int i = 0; i < CNT; i++) {
        testDecompress(hex2binString(sCompressedData[i]));
    }
}

void testCompressDecompress(
        const std::string &value
)
{
    std::string c = compressLoggerString(value);
    std::cout << "Source       " << bin2hexString(value) << ", size: " << value.size() << std::endl;
    std::cout << "Compressed   " << bin2hexString(c) << ", size: " << c.size() << std::endl;

    std::string t = decompressLoggerString(c);
    std::cout << "Decompressed " << bin2hexString(t) << ", size: " << t.size() << std::endl << std::endl;

    assert(value == t);
}

void testCompressDecompressBuffer(
        const std::string &value
)
{
    std::ostringstream ss;
    encodeHuffman(ss, value.c_str(), value.size());
    std::string c = ss.str();
    std::cout << "Source       " << bin2hexString(value) << ", size: " << value.size() << std::endl;
    std::cout << "Compressed   " << bin2hexString(c) << ", size: " << c.size() << std::endl;

    std::ostringstream ss1;
    decodeHuffman(ss1, c.c_str(), c.size());
    std::string t = ss1.str();
    std::cout << "Decompressed " << bin2hexString(t) << ", size: " << t.size() << std::endl << std::endl;

    assert(value == t);
}

void testPerformance(int count, int size)
{
    struct timeval t0, t1, df;
    gettimeofday(&t0, NULL);

    std::string source;
    source.resize(size);
    const char *c = source.c_str();
    for (int i = 0; i < size; i++) {
        std::stringstream ss;
        encodeHuffman(ss, c, size);
        std::string encoded = ss.str();
        std::ostringstream ss1;
        decodeHuffman(ss1, encoded.c_str(), encoded.size());
        std::string decoded = ss1.str();
        assert(decoded == source);
    }

    gettimeofday(&t1, NULL);
    timevalSubtract(&df, &t1, &t0);

    std::cout
            << "Count: " << std::dec << count << ", size: " << size
            << ", elapsed time: " << df.tv_sec << "." << df.tv_usec % 1000000 << std::endl;
}

void testComposeBase(
    double t0,
    double inc,
    int cnt
)
{
    std::vector<std::string> packets;
    LoggerMeasurements m;
    m.kosa = 1;
    m.kosa_year = 22;
    m.measure = 42;
    m.time = time(nullptr);
    m.vcc = 4.5;
    m.vbat = 1.5;

    for (int i = 0; i < cnt; i++)
    {
        m.temperature.push_back(t0);
        t0 += inc;
    }
    LoggerBuilder::build(packets, m);
    for (int i = 0; i < packets.size(); i++) {
        std::cerr << bin2hexString(packets[i])  << " ";
    }
}

void testComposeDelta(
    double t0,
    double inc,
    double diff,
    int cnt
)
{
    LoggerMeasurements mBase;
    mBase.kosa = 1;
    mBase.kosa_year = 22;
    mBase.measure = 42;
    time(&mBase.time);
    mBase.vcc = 4.5;
    mBase.vbat = 1.5;

    LoggerMeasurements m;
    m.kosa = 1;
    m.kosa_year = 22;
    m.measure = 42;
    time(&m.time);
    m.vcc = 4.5;
    m.vbat = 1.5;

    for (int i = 0; i < cnt; i++)
    {
        m.temperature.push_back(t0);
        mBase.temperature.push_back(t0 + diff);
        t0 += inc;
    }

    std::vector<std::string> basePackets;
    LoggerBuilder::build(basePackets, mBase);

    std::cerr << "./logger-huffman-print ";
    for (int i = 0; i < basePackets.size(); i++) {
        std::cerr << " -b " << bin2hexString(basePackets[i]);
    }
    std::cerr << " ";
    std::vector<std::string> packets;
    LoggerBuilder::build(packets, m, mBase.temperature);
    for (int i = 0; i < packets.size(); i++) {
        std::cerr << bin2hexString(packets[i]) << " ";
    }
    std::cerr << " -vvv" << std::endl;
}

int main(int argc, char **argv)
{
    // expected 010204010410041f (last byte may be different)
    // testCompressDecompress(hex2binString("01020304010203040102"));
    /*
    testDecompress2();
    testCompressDecompressBuffer(hex2binString("123456"));
    testDecompress(hex2binString("4A00280002031C140038100F160216000000003981190002"));
    testDecompress(hex2binString("4B1C02020006CFAA0101A8000201A8000301A9000401A900"));
    testDecompress(hex2binString("4B1C02030501A900"));
    testDecompress(hex2binString("4A00280003031C140038150F160216000000003981190003"));
    testDecompress(hex2binString("4B1C03020006CFAA0101A8000201A9000301AA000401A800"));
    testDecompress(hex2binString("4B1C03030501A900"));
    */
    // testPerformance(1024, 1024);
    // testComposeBase(-3.0, 0.25, 40);
    std::cerr << std::endl;
    /*
    testComposeDelta(-3.0, 0.25, -0.0625, 40);
    testComposeDelta(-3.0, 0.25, -0.125, 40);
    testComposeDelta(-3.0, 0.25, -0.1875, 40);
    testComposeDelta(-3.0, 0.25, -1.0, 40);
    testComposeDelta(-3.0, 0.25, -2.0, 40);
    testComposeDelta(-3.0, 0.25, 0.0625, 40);
    testComposeDelta(-3.0, 0.25, 0.125, 40);
    testComposeDelta(-3.0, 0.25, 0.1875, 40);
    testComposeDelta(-3.0, 0.25, 1.0, 40);
     */
    //testComposeDelta(-50.0, 1.0, 0.1875, 80);
    testComposeDelta(-50.0, 1.0, 300.0, 80);
   /*
    testComposeDelta(-3.0, 0.25, 2.0, 80);
    testComposeDelta(-3.0, 0.25, 3.0, 40);
    testComposeDelta(-3.0, 0.25, 4.0, 40);
    testCompressDecompress(hex2binString("0a0b0c0d0a0b0c0d0a0b"));
    testCompressDecompress(hex2binString("01020304010203040102"));
    testCompressDecompress(hex2binString("01020304"));
    testCompressDecompress("The quick brown fox jumps over the lazy dog");
    */
}
