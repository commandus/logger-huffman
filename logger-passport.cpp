#include "logger-passport.h"

/**
 * Initialize logger password directory
 * @param passwords_path path to the catalog with password files
 * @param verbosity if 1, 2 or 3 print out to the stderr errors parsing declarations
 * @return descriptor of the passwords to be passed to the parsePacket()
 */
void* initLoggerPasswords(
	const std::string &passwords_path,
	int verbosity
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
 * @param inputFormat 0- binary, 1- hex string
 * @param outputFormat 0- json(default), 1- csv, 2- tab, 3- sql, 4- Sql, 5- pbtext, 6- dbg, 7- hex, 8- bin 
 * @param sqlDialect 0- PostgeSQL, 1- MySQL, 1- Firebird
 * @param packet data
 * @param forceMessage "" If specifed, try only message type
 * @param tableAliases protobuf message to datanase table map
 * @param fieldAliases protobuf message attribute to datanase column map
 * @param properties "session environment variables", e.g addr, eui, time, timestamp
 * @return empty string if fails
 */
std::string parsePacket(
	void *env, 
	int inputFormat,
	int outputFormat,
	int sqlDialect,
	const std::string &packet,
	const std::string &forceMessage,
	const std::map<std::string, std::string> *tableAliases,
	const std::map<std::string, std::string> *fieldAliases,
	const std::map<std::string, std::string> *properties
)
{
    return "";
}

/**
 * Return CREATE table SQL clause
 * @param env packet declaratuions
 * @param messageName Protobuf full type name (including packet)
 * @param outputFormat 3- sql, 4- Sql
 * @param sqlDialect 0- PostgeSQL, 1- MySQL, 1- Firebird
 * @param tableAliases <Protobuf full type name>=<alias (SQL table name)>
 * @param fieldAliases <Protobuf message fiekd name>=<alias (SQL column name)>
 * @return empty string if fails
 */
std::string createTableSQLClause(
	void *env, 
	const std::string &messageName,
	int outputFormat,
	int sqlDialect,
	const std::map<std::string, std::string> *tableAliases,
	const std::map<std::string, std::string> *fieldAliases,
	const std::map<std::string, std::string> *properties
)
{

}
