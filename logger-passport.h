#ifndef LOGGER_PASSPORT_H
#define LOGGER_PASSPORT_H     1

#include <string>
#include <map>

#include "logger-collection.h"

#define OUTPUT_FORMAT_JSON		0
#define OUTPUT_FORMAT_CSV		1
#define OUTPUT_FORMAT_TAB		2
#define OUTPUT_FORMAT_SQL		3
#define OUTPUT_FORMAT_SQL2		4
#define OUTPUT_FORMAT_PBTEXT	5
#define OUTPUT_FORMAT_DEBUG		6
#define OUTPUT_FORMAT_HEX		7
#define OUTPUT_FORMAT_BIN		8

enum SQL_DIALECT : int {
    SQL_POSTGRESQL = 0,
    SQL_MYSQL = 1,
    SQL_FIREBIRD = 2,
    SQL_SQLITE = 3
};

/**
 * Initialize logger password directory
 * @param passwords_path path to the catalog with password files
 * @param verbosity if 1, 2 or 3 print out to the stderr errors parsing declarations
 * @return descriptor of the passwords to be passed to the parsePacket()
 */
void* initLoggerPasswords(
    const std::string &passwords_path,
    const int verbosity = 0
);

/**
 * Destroy and free logger properties
 * @param env descriptor
 */
void doneLoggerPasswords(void *env);

/**
 * Parse packet by declaration
 * @param env packet declaratuions
 * @param outputFormat 0- json(default), 1- csv, 2- tab, 3- sql, 4- Sql, 5- pbtext, 6- dbg, 7- hex, 8- bin
 * @param sqlDialect 0- PostgeSQL, 1- MySQL, 1- Firebird
 * @param packetы LoggerKosaPackets
 * @param tableAliases protobuf message to datanase table map
 * @param fieldAliases protobuf message attribute to datanase column map
 * @param properties "session environment variables", e.g addr, eui, time, timestamp
 * @return empty string if fails
 */
std::string parsePacket(
    void *env,
    int outputFormat,
    int sqlDialect,
    const LoggerKosaPackets &packets,
    const std::map<std::string, std::string> *tableAliases = NULL,
    const std::map<std::string, std::string> *fieldAliases = NULL,
    const std::map<std::string, std::string> *properties = NULL
);

/**
 * Return CREATE table SQL clause
 * @param env packet declaratuions
 * @param outputFormat 3- sql, 4- Sql
 * @param sqlDialect 0- PostgeSQL, 1- MySQL, 1- Firebird
 * @param tableAliases <Protobuf full type name>=<alias (SQL table name)>
 * @param fieldAliases <Protobuf message fiekd name>=<alias (SQL column name)>
 * @return empty string if fails
 */
std::string createTableSQLClause(
    void *env,
    int outputFormat,
    int sqlDialect,
    const std::map<std::string, std::string> *tableAliases = NULL,
    const std::map<std::string, std::string> *fieldAliases = NULL,
    const std::map<std::string, std::string> *properties = NULL
);

#endif