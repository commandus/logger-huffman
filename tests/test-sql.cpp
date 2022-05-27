#include <string>
#include <cassert>
#include <iostream>
#include <sys/time.h>
#include <iomanip>

#include "logger-parse.h"
#include "logger-sql-clause.h"
#include "util-time-fmt.h"
#include "dumb-logger-loader.h"
#include "errlist.h"

const uint32_t DEV_ADDR_INT = 42;

const std::string packet0 = "002614121f0c14261300003d3d71000100cf06aa01e6ff00"
                            "02deff0003eaff0004dcff0005e3ff0006e0ff0007e2ff00"
                            "08ddff0009e3ff000adeff000bdaff000cdfff000debff00"
                            "0ee8ff000fcdff0010e7ff0011dfff0012ddff0013e1ff00"
                            "14dcff0015dcff0016ebff0017e6ff0018dfff0019dfff00"
                            "1adaff001be6ff00";

const std::string packetHuff = "4c620a00020126130100467cbff9fe73e67f";
const std::string packetsBase = "4a0080000207261300011512010115261300003e3d710002"
                                "4b26020200cf06aa01e6ff0002deff0003eaff0004dcff00"
                                "4b26020305e3ff0006e0ff0007e2ff0008ddff0009e2ff00"
                                "4b2602040adeff000bdaff000cdfff000debff000ee8ff00"
                                "4b2602050fcdff0010e6ff0011dfff0012dcff0013e1ff00"
                                "4b26020614dcff0015dcff0016eaff0017e5ff0018dfff00"
                                "4b26020719dfff001adaff001be6ff00";

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
    LOGGER_PACKET_TYPE t = c.put(0, hex2binString(packet0));

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

        r = parsePacket(env, DEV_ADDR_INT, hex2binString(packet0));
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
        r = parsePacket(env, DEV_ADDR_INT, hex2binString(packetIncomplete));
        std::cout << "r1: " << r << std::endl;
        r = parsePacket(env, DEV_ADDR_INT, hex2binString(packetIncomplete));
        std::cout << "r2: " << r << std::endl;
        r = parsePacket(env, DEV_ADDR_INT, hex2binString(packetIncomplete));
        std::cout << "r3: " << r << std::endl;
        r = parsePacket(env, DEV_ADDR_INT, hex2binString(packetIncomplete));
        std::cout << "r4: " << r << std::endl;


        std::cout << i << "  " << sqlInsertPackets1(env, dialect) << std::endl;
        std::cout << "pg " << loggerParserState(env, 0) << std::endl;
        std::cout << "js " << loggerParserState(env, 4) << std::endl;
        std::cout << std::endl;
    }
    doneLoggerParser(env);
}

void testDeltaPacket() {
    struct timeval t0, t1, df;
    size_t count = 0;
    gettimeofday(&t0, NULL);

    void *env = initLoggerParser("../logger-passport/tests/passport", nullptr);
    LoggerKosaCollector *c = (LoggerKosaCollector*) getLoggerKosaCollection(env);

    // Delta packet require "base" packets to be loaded
    LoggerKosaCollector lkcBase;
    lkcBase.put(DEV_ADDR_INT, hex2binString(packetsBase));

    // set "base" loader
    DumbLoggerKosaPacketsLoader lkl;
    lkl.setCollection(&lkcBase);
    c->setLoggerKosaPacketsLoader(&lkl);


    // set SQL dialect output
    int dialect = SQL_POSTGRESQL;
    std::string pH = hex2binString(packetHuff);
    for (int i = 0; i < 1000; i++) {
        parsePacket(env, DEV_ADDR_INT, pH);
        std::string outputString = sqlInsertPackets1(env, dialect); // toJsonString();
        // std::cerr << outputString << std::endl;
        rmCompletedOrExpired(env);
        count++;
    }
    doneLoggerParser(env);

    gettimeofday(&t1, NULL);
    timevalSubtract(&df, &t1, &t0);

    std::cout
        << "Count: " << std::dec << count
        << ", elapsed time: " << df.tv_sec << "." << df.tv_usec % 1000000 << std::endl;
}

void testBaseSQLStatement() {
    std::cout << buildSQLBaseMeasurementSelect(SQL_POSTGRESQL, DEV_ADDR_INT) << std::endl;
    std::cout << buildSQLBaseMeasurementSelect(SQL_MYSQL, DEV_ADDR_INT) << std::endl;
    std::cout << buildSQLBaseMeasurementSelect(SQL_FIREBIRD, DEV_ADDR_INT) << std::endl;
    std::cout << buildSQLBaseMeasurementSelect(SQL_SQLITE, DEV_ADDR_INT) << std::endl;
}

void testParseSQLBaseMeasurement() {
    std::vector<std::string> s;
    for (int i = 0; i < 1; i++) {
        s.clear();
        parseSQLBaseMeasurement(s, " 12  3456 78 ab  de ");
    }
    for (int i = 0; i < s.size(); i++) {
        std::cout << std::hex << std::setfill('0') << std::setw(2) << bin2hexString(s[i]) << " ";
    }
}

int main(int argc, char **argv)
{
    testLoggerParse();
    testIncompletePacket();
    testDeltaPacket();
    testBaseSQLStatement();
    // testParseSQLBaseMeasurement();
    exit(0);
}
