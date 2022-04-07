#include <string>
#include <sstream>

#include "logger-parse.h"
#include "logger-sql-clause.h"

void *initLoggerParser()
{
    LoggerKosaCollection *r = new LoggerKosaCollection();
    return r;
}

void flushLoggerParser(void *env)
{
    if (!env)
        return;
    LoggerKosaCollection *c = (LoggerKosaCollection*) env;
    c->rmExpired();
}

void doneLoggerParser(void *env)
{
    if (!env)
        return;
    LoggerKosaCollection *c = (LoggerKosaCollection*) env;
    delete c;
}

/**
 * Return state of the desctiptor
 * @param env descriptor
 * @param format 0- Postgres, 1- MySQL, 2- Firebird, 3- SQLite 4- JSON, 5- text, 6- table
 */
std::string loggerParserState(
    void *env,
    int format
)
{
    if (!env)
        return "";
    LoggerKosaCollection *c = (LoggerKosaCollection*) env;
    switch (format) {
        case 4:
            return c->toJsonString();
        case 5:
            return c->toString();
        case 6:
            return c->toTableString();
        default:
            return sqlInsertPackets1(env, format);
    }
}

int parsePacket(
    void *env,
    const std::string &packet
)
{
    if (!env)
        return 0;
    LoggerKosaCollection *kosaCollection = (LoggerKosaCollection*) env;
    LOGGER_PACKET_TYPE t = kosaCollection->put(packet);
    // kosaCollection->rmExpired();
    return (int) t;
}

/**
 * Return INSERT clause
 * @param env descriptor* 
 * @param retCompleted true- all packets received
 * @param retExpired
 * @param packets binary packets
 * @param extraValues  <optional field name>=value
 * @return empty string if fails
 */
int sqlInsertPackets(
    void *env,
    std::vector <std::string> &retClauses,
    int sqlDialect,
    const std::map<std::string, std::string> *extraValues
)
{
    if (!env)
        return 0;
    LoggerKosaCollection *kosaCollection = (LoggerKosaCollection*) env;
    int c = 0;
    for (std::vector<LoggerKosaPackets>::const_iterator it(kosaCollection->koses.begin()); it != kosaCollection->koses.end(); it++) {
        if (it->packets.completed() | it->expired()) {
            std::string s = parsePacketsToSQLClause(OUTPUT_FORMAT_SQL, sqlDialect, *it, extraValues);
            if (!s.empty())
                retClauses.push_back(s);
            c++;
        }
    }
    return c;
}

/**
 * Remove completed or expired items
 * @param env descriptor
 */
void rmCompletedOrExpired(
    void *env
)
{
    if (!env)
        return;
    LoggerKosaCollection *kosaCollection = (LoggerKosaCollection*) env;
    for (std::vector<LoggerKosaPackets>::const_iterator it(kosaCollection->koses.begin()); it != kosaCollection->koses.end(); ) {
        bool ready2delete = it->packets.completed() | it->expired();
        if (ready2delete) {
            it = kosaCollection->koses.erase(it);
        } else {
            it++;
        }
    }
}

/**
 * Return CREATE table SQL clause in 
 * @param retClauses vector of CREATE statements
 * @param sqlDialect 0- PostgeSQL, 1- MySQL, 2- Firebird, 3- SQLite
 * @param extraValues  <optional field name>=<SQL type name>
 * @return count of statements, <0- error
 */
int sqlCreateTable(
    std::vector <std::string> &retClauses,
    int sqlDialect,
    const std::map<std::string, std::string> *extraValues
)
{
    retClauses.push_back(createTableSQLClauseLoggerRaw(OUTPUT_FORMAT_SQL, sqlDialect, extraValues));
    retClauses.push_back(createTableSQLClauseLoggerLora(OUTPUT_FORMAT_SQL, sqlDialect, extraValues));
    return retClauses.size(); 
}

/**
 * Return CREATE table SQL clause
 * @param sqlDialect 0- PostgeSQL, 1- MySQL, 2- Firebird, 3- SQLite
 * @param extraValues  <optional field name>=<SQL type name>
 * @param separator  separator string default space 
 * @return empty string if fails
 */
std::string sqlCreateTable1(
    int sqlDialect,
    const std::map<std::string, std::string> *extraValues,
    const std::string &separator
)
{
    return 
        createTableSQLClauseLoggerRaw(OUTPUT_FORMAT_SQL, sqlDialect, extraValues)
        + separator
        + createTableSQLClauseLoggerLora(OUTPUT_FORMAT_SQL, sqlDialect, extraValues);
}

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
    const std::map<std::string, std::string> *extraValues,
    const std::string &separator
)
{
    std::vector <std::string> clauses;
    std::stringstream ss;
    sqlInsertPackets(env, clauses, sqlDialect);
    bool first = true;
    for (auto it(clauses.begin()); it != clauses.end(); it++) {
        if (first)
            first = false;
        else
            ss << separator;
        ss << *it;
    }
    return ss.str();
}

/**
 * Return INSERT raw data (as hex)
 * @param sqlDialect 0..3
 * @param extraValues  <optional field name>=value
 * @return empty string if fails
 */
std::string sqlInsertRaw(
    int sqlDialect,
    const std::string &value,
    const std::map<std::string, std::string> *extraValues
)
{
    std::stringstream ss;
    sqlInsertRawStrm(ss, sqlDialect, value, extraValues);
    return ss.str();
}
