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

int parsePacket(void *env, const std::string &packet);

/**
 * Return INSERT clause
 * @param retCompleted true- all packets received
 * @param retExpired
 * @param packetы binary packets
 * @param extraValues  <optional field name>=value
 * @return empty string if fails
 */
int sqlInsertPackets(
    void *env,
    std::vector <std::string> &retClauses,
    int sqlDialect,
    const std::map<std::string, std::string> *extraValues = NULL
);

#endif
