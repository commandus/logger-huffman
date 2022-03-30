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
    LoggerKosaCollection c;
    LOGGER_PACKET_TYPE t = c.put(hex2binString(packet0));

    if (t == LOGGER_PACKET_UNKNOWN)
        printErrorAndExit(ERR_LOGGER_HUFFMAN_INVALID_PACKET);

    assert(c.koses.size() == 1);

    std::string r;

    for (int dialect = SQL_POSTGRESQL; dialect <= SQL_SQLITE; dialect++)
    {
        std::cout << "Database: " << SQL_DIALECT_NAME[dialect] << std::endl;
        r = createTableSQLClause(OUTPUT_FORMAT_SQL, dialect);
        std::cout << r << std::endl;

        r = parsePacketsToSQLClause(OUTPUT_FORMAT_SQL, dialect, *c.koses.begin());
        std::cout << r << std::endl << std::endl;
    }
    std::cout << r << std::endl;
}

void testLoggerParse() {
    std::string r;
    void *env = initLoggerParser();

    for (int dialect = SQL_POSTGRESQL; dialect <= SQL_SQLITE; dialect++) {
        std::cout << "Database: " << SQL_DIALECT_NAME[dialect] << std::endl;

        r = sqlCreateTable(dialect);
        std::cout << r << std::endl;

        r = parsePacket(env, hex2binString(packet0));
        std::vector <std::string> clauses;
        sqlInsertPackets(env, clauses, dialect);
        for (auto it(clauses.begin()); it != clauses.end(); it++) {
            std::cout << *it << std::endl;
        }

        std::cout << std::endl;
    }

    flushLoggerParser(env);
    doneLoggerParser(env);
}

int main(int argc, char **argv)
{
    testLoggerParse();
}
