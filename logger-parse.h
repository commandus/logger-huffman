#ifndef LOGGER_PARSE_H_
#define LOGGER_PARSE_H_ 1

#include <vector>
#include <map>

/**
 * Return CREATE table SQL clause
 * @param sqlDialect 0- PostgeSQL, 1- MySQL, 2- Firebird, 3- SQLite
 * @param extraValues  <optional field name>=<SQL type name>
 * @return empty string if fails
 */
std::string sqlCreateTable(
    int sqlDialect,
    const std::map<std::string, std::string> *extraValues = NULL
);

void *initLoggerParser();
void flushLoggerParser(void *env);
void doneLoggerParser(void *env);

/**
 * Return state of the desctiptor
 * @param env descriptor
 * @param format 0- Postgres, 1- MySQL, 2- Firbord, 3- SQLite 4- JSON, 5- text, 6- table
 */
std::string loggerParserState(void *env, int format);

int parsePacket(void *env, const std::string &packet);

/**
 * Return INSERT clause(s) in retClauses
 * @param env desciptor
 * @param retClauses vector of INSERT statements
 * @param sqlDialect 0..3
 * @param extraValues  <optional field name>=value
 * @return 0- success
 */
int sqlInsertPackets(
    void *env,
    std::vector <std::string> &retClauses,
    int sqlDialect,
    const std::map<std::string, std::string> *extraValues = NULL
);

/**
 * Return INSERT clause(s) as one string
 * @param env desciptor
 * @param sqlDialect 0..3
 * @param extraValues  <optional field name>=value
 * @param separator  separator string default space
 * @return empty string if fails
 */
std::string sqlInsertPackets1(
    void *env,
    int sqlDialect,
    const std::map<std::string, std::string> *extraValues = NULL,
    const std::string &separator = " "
);

#endif
