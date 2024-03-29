#include <string>
#include <sstream>

#include "logger-parse.h"
#include "logger-sql-clause.h"

#ifdef ENABLE_LOGGER_PASSPORT
#include "logger-passport/logger-passport.h"
#endif

class LoggerParserEnv {
    public:
        void *passportDescriptor;
        LoggerKosaCollector *lkc;
};

void *initLoggerParser(
    const std::string &passportDir,     ///< passport files root
    LOG_CALLBACK onLog,                 ///< log callback
    void *loggerKosaPacketsLoader
)
{
    LoggerParserEnv *r = new LoggerParserEnv();
#ifdef ENABLE_LOGGER_PASSPORT
    r->passportDescriptor = startPassportDirectory(passportDir, onLog);
#else
    r->passportDescriptor = nullptr;
#endif    
    r->lkc = new LoggerKosaCollector();
    r->lkc->setPassports(r->passportDescriptor);

    if (loggerKosaPacketsLoader) {
        // set "base" loader
        r->lkc->setLoggerKosaPacketsLoader(static_cast<LoggerKosaPacketsLoader *>(loggerKosaPacketsLoader));
    }
    return r;
}

void *initLoggerParser(
    const std::vector<std::string> &passportDirs,       ///< passport files root
    LOG_CALLBACK onLog,                                 ///< log callback
    void *loggerKosaPacketsLoader
)
{
    LoggerParserEnv *r = new LoggerParserEnv();
#ifdef ENABLE_LOGGER_PASSPORT
    r->passportDescriptor = startPassportDirectory(passportDirs, onLog);
#else
    r->passportDescriptor = nullptr;
#endif    
    r->lkc = new LoggerKosaCollector();
    r->lkc->setPassports(r->passportDescriptor);

    if (loggerKosaPacketsLoader) {
        // set "base" loader
        r->lkc->setLoggerKosaPacketsLoader(static_cast<LoggerKosaPacketsLoader *>(loggerKosaPacketsLoader));
    }
    return r;
}

void flushLoggerParser(void *env)
{
    if (!env)
        return;
    LoggerKosaCollector *c = ((LoggerParserEnv*) env)->lkc;
    c->rmExpired();
}

void doneLoggerParser(void *env)
{
    if (!env)
        return;
#ifdef ENABLE_LOGGER_PASSPORT
    if (((LoggerParserEnv*) env)->passportDescriptor) {
        stopPassportDirectory(((LoggerParserEnv*) env)->passportDescriptor);
        ((LoggerParserEnv*) env)->passportDescriptor = nullptr;
    }
#endif
    LoggerKosaCollector *c = ((LoggerParserEnv*) env)->lkc;
    if (c)
        delete c;
}

/**
 * Return state of the descriptor
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
    LoggerKosaCollector *c = ((LoggerParserEnv*) env)->lkc;
    switch (format) {
        case 4:
            return c->packetsToJsonString();
        case 5:
            return c->packetsToString();
        case 6:
            return c->toTableString();
        default:
            return loggerSQLInsertPackets1(env, format);
    }
}

int loggerParsePacket(void *env, uint32_t addr, const std::string &packet)
{
    if (!env)
        return 0;
    LoggerKosaCollector *c = ((LoggerParserEnv*) env)->lkc;
    LOGGER_PACKET_TYPE t = c->put(addr, packet);
    return (int) t;
}

int loggerParsePackets(void *env, uint32_t addr, const std::vector<std::string> &packets)
{
    if (!env)
        return 0;
    LoggerKosaCollector *c = ((LoggerParserEnv*) env)->lkc;
    LOGGER_PACKET_TYPE t = c->put(addr, packets);
    return (int) t;
}

/**
 * Return INSERT clause
 * @param env descriptor* 
 * @param retCompleted true- all packets received
 * @param retExpired
 * @param packets binary packets
 * @param extraValues  <optional field name>=value
 * @param nullValueString default "NULL"
 * @return empty string if fails
 */
int loggerSQLInsertPackets(
    void *env,
    std::vector <std::string> &retClauses,
    int sqlDialect,
    const std::map<std::string, std::string> *extraValues,
    const std::string &nullValueString
)
{
    if (!env)
        return 0;
    LoggerKosaCollector *c = ((LoggerParserEnv*) env)->lkc;
    int cnt = 0;
    for (std::vector<LoggerKosaPackets>::const_iterator it(c->koses.begin()); it != c->koses.end(); it++) {
        if (it->packets.completed() | it->expired()) {
            std::string s = loggerParsePacketsToSQLClause(OUTPUT_FORMAT_SQL, sqlDialect, *it, extraValues, nullValueString);
            if (!s.empty())
                retClauses.push_back(s);
            cnt++;
        }
    }
    return cnt;
}

/**
 * Remove completed or expired items
 * @param env descriptor
 */
void loggerRemoveCompletedOrExpired(
    void *env
)
{
    if (!env)
        return;
    LoggerKosaCollector *c = ((LoggerParserEnv*) env)->lkc;
    for (std::vector<LoggerKosaPackets>::iterator it(c->koses.begin()); it != c->koses.end(); ) {
        bool ready2delete = it->packets.completed() | it->expired();
        if (ready2delete) {
            it = c->koses.erase(it);
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
int loggerSQLCreateTable(
    std::vector <std::string> &retClauses,
    int sqlDialect,
    const std::map<std::string, std::string> *extraValues
)
{
    retClauses.push_back(createTableSQLClauseLoggerRaw(OUTPUT_FORMAT_SQL, sqlDialect, extraValues));
    retClauses.push_back(createTableSQLClauseLoggerLora(OUTPUT_FORMAT_SQL, sqlDialect, extraValues));
    return (int) retClauses.size(); 
}

/**
 * Return CREATE table SQL clause
 * @param sqlDialect 0- PostgeSQL, 1- MySQL, 2- Firebird, 3- SQLite
 * @param extraValues  <optional field name>=<SQL type name>
 * @param separator  separator string default space 
 * @return empty string if fails
 */
std::string loggerSQLCreateTable1(
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
std::string loggerSQLInsertPackets1(
    void *env,
    int sqlDialect,
    const std::map<std::string, std::string> *extraValues,
    const std::string &nullValueString,
    const std::string &separator
)
{
    std::vector <std::string> clauses;
    std::stringstream ss;
    loggerSQLInsertPackets(env, clauses, sqlDialect, extraValues, nullValueString);
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
 * @param value data
 * @param extraValues  <optional field name>=value
 * @return empty string if fails
 */
std::string loggerSQLInsertRaw(
    int sqlDialect,
    const std::string &value,
    const std::map<std::string, std::string> *extraValues
)
{
    std::stringstream ss;
    loggerSQLInsertRawStrm(ss, sqlDialect, value, extraValues);
    return ss.str();
}

void *getPassportDescriptor(void *env)
{
    return ((LoggerParserEnv *) env)->passportDescriptor;
}

void *getLoggerKosaCollector(void *env)
{
    return (void *) ((LoggerParserEnv *) env)->lkc;
}

size_t lsPassports(
    void *env,
    int format,
    std::vector<std::string> *retVal,
    int year,
    int plume,
    size_t offset,
    size_t count
)
{
#ifdef ENABLE_LOGGER_PASSPORT
    return countPassports(getPassportDescriptor(env), (FORMAT_PASSPORT_TYPE) format, retVal, year, plume, offset, count);
#else
    return 0;
#endif
}


/**
 * Return SQL SELECT statement returning packets as hex strings separated by space
 * @param sqlDialect SQL dialect number
 * @param addr LoRaWAN device address 4 bytes long integer
 * @return SQL SELECT statement returning packets as hex strings separated by space9
 */
std::string loggerBuildSQLBaseMeasurementSelect(
    int sqlDialect,
    uint32_t addr
)
{
    std::stringstream ss;
    loggerBuildSQLSelectBaseMeasurement(ss, sqlDialect, addr);
    return ss.str();
}

static std::string readHex(
    size_t &pos,
    const std::string &s
)
{
    std::stringstream r;
    char c[3] = { 0, 0, 0 };
    size_t sz = s.size();
    bool hasData = false;
    while (true) {
        if (pos + 1 >= sz)  // 2 digits
            break;
        c[0] = s.at(pos);
        c[1] = s.at(pos + 1);
        pos++;
        if (c[0] <= 32) {
            if (hasData)
                break;
            else
                continue;
        }
        pos++;
        if (c[1] <= 32) {
            if (hasData)
                break;
            else
                continue;
        }
        unsigned char x = (unsigned char) strtol(c, NULL, 16);
        r << x;
        hasData = true;
    }
    return r.str();
}

bool loggerParseSQLBaseMeasurement(
    std::vector <std::string> &retClauses,
    const std::string &value
)
{
    size_t p = 0;
    size_t sz = value.size();
    while (true) {
        std::string s = readHex(p, value);
        if (!s.empty())
            retClauses.push_back(s);
        if (p + 1 >= sz)
            break;
    }
    return true;
}
