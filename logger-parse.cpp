#include <string>
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

int parsePacket(
    void *env,
    const std::string &packet
)
{
    if (!env)
        return 0;
    LoggerKosaCollection *kosaCollection = (LoggerKosaCollection*) env;
    LOGGER_PACKET_TYPE t = kosaCollection->put(packet);
    kosaCollection->rmExpired();
    return (int) t;
}

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
    const std::map<std::string, std::string> *extraValues
)
{
    if (!env)
        return 0;
    retClauses.clear();
    LoggerKosaCollection *kosaCollection = (LoggerKosaCollection*) env;
    for (std::vector<LoggerKosaPackets>::const_iterator it(kosaCollection->koses.begin()); it != kosaCollection->koses.end(); ) {
        bool ready = it->packets.completed() | it->expired();
        if (ready) {
            std::string s = parsePacketsToSQLClause(OUTPUT_FORMAT_SQL, sqlDialect, *it, extraValues);
            if (!s.empty())
                retClauses.push_back(s);
            it = kosaCollection->koses.erase(it);
        } else {
            it++;
        }
    }
    return retClauses.size();
}

/**
 * Return CREATE table SQL clause
 * @param sqlDialect 0- PostgeSQL, 1- MySQL, 2- Firebird, 3- SQLite
 * @param extraValues  <optional field name>=<SQL type name>
 * @return empty string if fails
 */
std::string sqlCreateTable(
    int sqlDialect,
    const std::map<std::string, std::string> *extraValues
)
{
    return createTableSQLClause(OUTPUT_FORMAT_SQL, sqlDialect, extraValues);
}
