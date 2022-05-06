#include <string>
#include <cassert>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#include "logger-huffman.h"

#include "logger-collection.h"

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

void testCompressedData(
        const std::string &value
)
{
    std::string s = decompressLoggerString(value);
    std::cout << "COMPRESSED   " << bin2hexString(value) << " size: " << value.size() << std::endl;
    std::cout << "DECOMPRESSED " << bin2hexString(s) << " size: " << s.size() << std::endl;

    std::string c = compressLoggerString(s);
    std::cout << "COMPRESSED 2 " << bin2hexString(c) << " size: " << c.size() << std::endl << std::endl;
}

void testCompressDecompress(
        const std::string &value
)
{
    std::string c = compressLoggerString(value);
    std::cout << "SOURCE       " << bin2hexString(value) << " size: " << value.size() << std::endl;
    std::cout << "COMPRESSED   " << bin2hexString(c) << " size: " << c.size() << std::endl;

    std::string t = decompressLoggerString(c);
    std::cout << "DECOMPRESSED " << bin2hexString(t) << " size: " << t.size() << std::endl << std::endl;
}

int main(int argc, char **argv) {

    testCompressDecompress(hex2binString("0a0b0c0d0a0b0c0d0a0b"));
    testCompressDecompress(hex2binString("01020304010203040102"));
    testCompressDecompress(hex2binString("01020304"));
    testCompressDecompress("The quick brown fox jumps over the lazy dog");
}
