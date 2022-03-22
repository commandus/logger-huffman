#include <sstream>

#include "logger-passport.h"

/**
 * Initialize logger password directory
 * @param passwords_path path to the catalog with password files
 * @param verbosity if 1, 2 or 3 print out to the stderr errors parsing declarations
 * @return descriptor of the passwords to be passed to the parsePacket()
 */
void* initLoggerPasswords(
	const std::string &passwords_path,
	const int verbosity // default 0
)
{
    return NULL;
}

/**
 * Destroy and free logger properties
 * @param env descriptor
 */
void doneLoggerPasswords(void *env)
{
}

/**
 * Parse packet by declaration
 * @param env packet declaratuions
 * @param outputFormat 0- json(default), 1- csv, 2- tab, 3- sql, 4- Sql, 5- pbtext, 6- dbg, 7- hex, 8- bin
 * @param sqlDialect 0- PostgeSQL, 1- MySQL, 1- Firebird
 * @param packets data
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
	const std::map<std::string, std::string> *tableAliases,
	const std::map<std::string, std::string> *fieldAliases,
	const std::map<std::string, std::string> *properties
)
{
    return "";
}

// replace string
static std::string replaceString(const std::string &str, const std::string &from, const std::string &to)
{
    size_t start_pos = str.find(from);
    if (start_pos == std::string::npos)
        return str;
    std::string ret(str);
    ret.replace(start_pos, from.length(), to);
    return ret;
}

static std::string getSqlDialectTypeName(
        const std::string &typeName,
        int sqlDialect
)
{
    return "INTEGER";
}

static void createTableSQLClause1(
        std::ostream *output,
        void *env,
        const std::string &tableName,
        int outputFormat,
        int sqldialect,
        const std::map<std::string, std::string> *tableAliases,
        const std::map<std::string, std::string> *fieldAliases,
        const std::map<std::string, std::string> *properties
)
{
    std::string quote;
    if (sqldialect == SQL_MYSQL)
        quote = "`";	// MySQL exceptions for spaces and reserved words
    else
        quote = "\"";

    *output << "CREATE TABLE " << quote << replaceString(tableName, ".", "_") << quote << "(";

    int sz = values.size();

    int fieldCount = 0;
    for (int i = 0; i < sz; i++)
    {
        std::string fieldName = findAlias(fieldAliases, values[i].field);
        // if alias set to empty string, skip table
        if (fieldName.empty())
            continue;
        if (fieldCount)
            *output << ", ";
        *output << quote << fieldName << quote << " ";
        *output << getSqlDialectTypeName(values[i].field_type, sqldialect);
        fieldCount++;
    }
    if (fieldCount == 0)
        return;

    if (properties) {
        for (std::map<std::string, std::string>::const_iterator it(properties->begin()); it != properties->end(); it++)
        {
            if (it->second.empty())
                continue;
            if (fieldCount)
                *output << ", ";
            *output << quote << it->second << quote << " ";
            // force string type
            *output << getSqlDialectTypeName(it->second, sqldialect);
            fieldCount++;
        }
    }

    *output << ");";
}

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
	const std::map<std::string, std::string> *tableAliases,
	const std::map<std::string, std::string> *fieldAliases,
	const std::map<std::string, std::string> *properties
)
{
    std::stringstream ss;
    std::string tableName = "table";

    switch (outputFormat) {
        case OUTPUT_FORMAT_SQL2:
            createTableSQLClause1(&ss, env, tableName, outputFormat, sqlDialect, tableAliases, fieldAliases, properties);
        default:
            createTableSQLClause1(&ss, env, tableName, outputFormat, sqlDialect, tableAliases, fieldAliases, properties);
    }
    return ss.str();
}
