#ifndef LOGGER_SQL_CLAUSE_H
#define LOGGER_SQL_CLAUSE_H     1

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
 * Parse packet by declaration
 * @param outputFormat 0- json(default), 1- csv, 2- tab, 3- sql, 4- Sql, 5- pbtext, 6- dbg, 7- hex, 8- bin
 * @param sqlDialect 0- PostgeSQL, 1- MySQL, 1- Firebird
 * @param packetы LoggerKosaPackets
 * @param extraValues  <optional field name>=value
 * @return empty string if fails
 */
std::string parsePacketsToSQLClause(
    int outputFormat,
    int sqlDialect,
    const LoggerKosaPackets &packets,
    const std::map<std::string, std::string> *extraValues = NULL
);

/**
 * Return CREATE table SQL clause logger_lora
 * @param outputFormat 3- sql, 4- Sql
 * @param sqlDialect 0- PostgreSQL, 1- MySQL, 2- Firebird
 * @param extraValues  <optional field name>=<SQL type name>
 * @return empty string if fails
 */
std::string createTableSQLClauseLoggerLora(
    int outputFormat,
    int sqlDialect,
    const std::map<std::string, std::string> *extraValues = NULL
);

/**
 * Return CREATE table SQL clause logger_raw
 * @param outputFormat 3- sql, 4- Sql
 * @param sqlDialect 0- PostgreSQL, 1- MySQL, 2 - Firebird
 * @param extraValues  <optional field name>=<SQL type name>
 * @return empty string if fails
 */
std::string createTableSQLClauseLoggerRaw(
    int outputFormat,
    int sqlDialect,
    const std::map<std::string, std::string> *extraValues = NULL
);

/**
 * Return INSERT raw data (as hex)
 * @param ostrm return stream
 * @param sqlDialect 0..3
 * @param extraValues  <optional field name>=value
 */
void sqlInsertRawStrm(
    std::ostream &ostrm,
    int sqlDialect,
    const std::string &value,
    const std::map<std::string, std::string> *extraValues = NULL
);

/**
 * Produce SQL SELECT statement get base measurement for device.
 * Used in delta packet processing.
 * @param outStrteam output stream
 * @param sqlDialect SQL dialect
 * @param addr LoRaWAN device address
 */
void buildSQLSelectBaseMeasurement(
    std::ostream &outStrteam,
    int sqlDialect,
    uint32_t addr
);

#endif