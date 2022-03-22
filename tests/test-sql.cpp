#include <string>
#include <cassert>
#include <iostream>

#include "logger-passport.h"
#include "errlist.h"

const std::string packet0 = "002614121f0c14261300003d3d71000100cf06aa01e6ff00 "
                            "02deff0003eaff0004dcff0005e3ff0006e0ff0007e2ff00 "
                            "08ddff0009e3ff000adeff000bdaff000cdfff000debff00 "
                            "0ee8ff000fcdff0010e7ff0011dfff0012ddff0013e1ff00 "
                            "14dcff0015dcff0016ebff0017e6ff0018dfff0019dfff00 "
                            "1adaff001be6ff00";

static void printErrorAndExit(
        int errCode
)
{
    std::cerr << ERR_MESSAGE << errCode << ": " << strerror_logger_huffman(errCode) << std::endl;
    exit(errCode);
}

int main(int argc, char **argv)
{
    void *ed = initLoggerPasswords(".");
    /*
     * INPUT_FORMAT_BINARY		0
       INPUT_FORMAT_HEX		1

     *  OUTPUT_FORMAT_JSON		0
        ..
        OUTPUT_FORMAT_SQL		3
        OUTPUT_FORMAT_SQL2		4
        ..
        OUTPUT_FORMAT_BIN		8
     */
    int sqlDialect = 0;
    std::string r = createTableSQLClause(ed, OUTPUT_FORMAT_SQL, sqlDialect);
    std::cout << r << std::endl;
    r = createTableSQLClause(ed, OUTPUT_FORMAT_SQL2, sqlDialect);
    std::cout << r << std::endl;


    LoggerKosaCollection c;
    LOGGER_PACKET_TYPE t = c.put(hex2binString(packet0));

    std::cout << "t: " << (int) t << std::endl;

    if (t == LOGGER_PACKET_UNKNOWN)
        printErrorAndExit(ERR_LOGGER_HUFFMAN_INVALID_PACKET);
    assert(c.koses.size() == 1);

    std::cout << "Packet: " << c.koses.begin()->toString() << std::endl;

    for (int of = OUTPUT_FORMAT_JSON; of <= OUTPUT_FORMAT_BIN; of++)
    {
        r = parsePacket(ed, of, sqlDialect, *c.koses.begin());
        std::cout << r << std::endl;
    }

    doneLoggerPasswords(ed);
}
