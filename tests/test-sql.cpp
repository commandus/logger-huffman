#include <string>
#include <cassert>
#include <iostream>

#include "logger-parse.h"
#include "logger-sql-clause.h"
#include "errlist.h"

const std::string packet0 = "002614121f0c14261300003d3d71000100cf06aa01e6ff00"
                            "02deff0003eaff0004dcff0005e3ff0006e0ff0007e2ff00"
                            "08ddff0009e3ff000adeff000bdaff000cdfff000debff00"
                            "0ee8ff000fcdff0010e7ff0011dfff0012ddff0013e1ff00"
                            "14dcff0015dcff0016ebff0017e6ff0018dfff0019dfff00"
                            "1adaff001be6ff00";

const std::string packetIncomplete = "4a00280002031c140038100f160216000000003981190002";

static void printErrorAndExit(
        int errCode
)
{
    std::cerr << ERR_MESSAGE << errCode << ": " << strerror_logger_huffman(errCode) << std::endl;
    exit(errCode);
}

static const std::string SQL_DIALECT_NAME[] = {
    "PostgreSQL", "MySQL", "Firebird", "SQLite"
};

void test1() {
    LoggerKosaCollector c;
    LOGGER_PACKET_TYPE t = c.put(hex2binString(packet0));

    if (t == LOGGER_PACKET_UNKNOWN)
        printErrorAndExit(ERR_LOGGER_HUFFMAN_INVALID_PACKET);

    assert(c.koses.size() == 1);

    std::string r;

    for (int dialect = SQL_POSTGRESQL; dialect <= SQL_SQLITE; dialect++)
    {
        std::cout << "Database: " << SQL_DIALECT_NAME[dialect] << std::endl;
        r = sqlCreateTable1(dialect);
        std::cout << r << std::endl;

        r = parsePacketsToSQLClause(OUTPUT_FORMAT_SQL, dialect, *c.koses.begin());
        std::cout << r << std::endl << std::endl;
    }
    std::cout << r << std::endl;
}

void testLoggerParse() {
    std::string r;
    void *env = initLoggerParser("", nullptr);

    for (int dialect = SQL_POSTGRESQL; dialect <= SQL_SQLITE; dialect++) {
        std::cout << "Database: " << SQL_DIALECT_NAME[dialect] << std::endl;

        r = sqlCreateTable1(dialect);
        std::cout << r << std::endl;

        r = parsePacket(env, hex2binString(packet0));
        std::cout << sqlInsertPackets1(env, dialect) << std::endl;

        std::cout << sqlInsertRaw(dialect, "111") << std::endl;
    }

    flushLoggerParser(env);
    doneLoggerParser(env);
}

void testIncompletePacket() {
    void *env = initLoggerParser("", nullptr);
    int dialect = SQL_POSTGRESQL;
    int r;
    for (int i = 0; i < 2; i++) {
        r = parsePacket(env, hex2binString(packetIncomplete));
        std::cout << "r1: " << r << std::endl;
        r = parsePacket(env, hex2binString(packetIncomplete));
        std::cout << "r2: " << r << std::endl;
        r = parsePacket(env, hex2binString(packetIncomplete));
        std::cout << "r3: " << r << std::endl;
        r = parsePacket(env, hex2binString(packetIncomplete));
        std::cout << "r4: " << r << std::endl;


        std::cout << i << "  " << sqlInsertPackets1(env, dialect) << std::endl;
        std::cout << "pg " << loggerParserState(env, 0) << std::endl;
        std::cout << "js " << loggerParserState(env, 4) << std::endl;
        std::cout << std::endl;
    }
    doneLoggerParser(env);
}

int main(int argc, char **argv)
{
    testLoggerParse();
    testIncompletePacket();
}
