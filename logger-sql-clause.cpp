#include <sstream>

#include "logger-sql-clause.h"

static const std::string tableName = "logger_lora";

#define FLD_COUNT  10

enum SQL_FLD : int {
    SQL_FIELD_ID = 0,
    SQL_FIELD_KOSA = 1,
    SQL_FIELD_YEAR = 2,
    SQL_FIELD_MEASURE_NUMBER = 3,
    SQL_FIELD_MEASURED = 4,
    SQL_FIELD_RECEIVED = 5,
    SQL_FIELD_VCC = 6,
    SQL_FIELD_VBAT = 7,
    SQL_FIELD_TEMPERATURE = 8,
    SQL_FIELD_RAW = 9
};

const std::string fldName[FLD_COUNT] = {
        "id",
        "kosa",
        "year",
        "no",
        "measured",
        "received",
        "vcc",
        "vbat",
        "t",
        "raw"
};

/*
 *  POSTGRESQL 10.x, MYSQL, FIREBIRD 3,x), SQLITE
 */
const std::vector <std::vector <std::pair<const std::string&, std::string> >> fldNTypes =
        {
                {   // Postgres
                        { fldName[SQL_FIELD_ID], "bigint generated always as identity"},
                        { fldName[SQL_FIELD_KOSA], "smallint"},
                        { fldName[SQL_FIELD_YEAR], "smallint"},
                        { fldName[SQL_FIELD_MEASURE_NUMBER], "smallint"},
                        { fldName[SQL_FIELD_MEASURED], "integer"},
                        { fldName[SQL_FIELD_RECEIVED], "integer"},
                        { fldName[SQL_FIELD_VCC], "real"},
                        { fldName[SQL_FIELD_VBAT], "real"},
                        { fldName[SQL_FIELD_TEMPERATURE], "text"},
                        { fldName[SQL_FIELD_RAW], "text"}
                },
                {   // Firebird
                        { fldName[SQL_FIELD_ID], "bigint generated by default as identity primary key"},
                        { fldName[SQL_FIELD_KOSA], "integer"},
                        { fldName[SQL_FIELD_YEAR], "integer"},
                        { fldName[SQL_FIELD_MEASURE_NUMBER], "integer"},
                        { fldName[SQL_FIELD_MEASURED], "integer"},
                        { fldName[SQL_FIELD_RECEIVED], "integer"},
                        { fldName[SQL_FIELD_VCC], "float"},
                        { fldName[SQL_FIELD_VBAT], "float"},
                        { fldName[SQL_FIELD_TEMPERATURE], "text"},
                        { fldName[SQL_FIELD_RAW], "text"}
                },
                {   // MySQL
                        { fldName[SQL_FIELD_ID], "bigint NOT NULL AUTO_INCREMENT"},
                        { fldName[SQL_FIELD_KOSA], "integer"},
                        { fldName[SQL_FIELD_YEAR], "integer"},
                        { fldName[SQL_FIELD_MEASURE_NUMBER], "integer"},
                        { fldName[SQL_FIELD_MEASURED], "integer"},
                        { fldName[SQL_FIELD_RECEIVED], "integer"},
                        { fldName[SQL_FIELD_VCC], "float"},
                        { fldName[SQL_FIELD_VBAT], "float"},
                        { fldName[SQL_FIELD_TEMPERATURE], "text"},
                        { fldName[SQL_FIELD_RAW], "text"}
                },
                {   // SQLite
                        { fldName[SQL_FIELD_ID], "integer PRIMARY KEY AUTOINCREMENT"},
                        { fldName[SQL_FIELD_KOSA], "integer"},
                        { fldName[SQL_FIELD_YEAR], "integer"},
                        { fldName[SQL_FIELD_MEASURE_NUMBER], "integer"},
                        { fldName[SQL_FIELD_MEASURED], "integer"},
                        { fldName[SQL_FIELD_RECEIVED], "integer"},
                        { fldName[SQL_FIELD_VCC], "real"},
                        { fldName[SQL_FIELD_VBAT], "real"},
                        { fldName[SQL_FIELD_TEMPERATURE], "text"},
                        { fldName[SQL_FIELD_RAW], "text"}
                }
        };

static void createTableSQLClause1(
        std::ostream *output,
        const std::vector <std::vector <std::pair<const std::string&, std::string> >> flds,
        const std::string &tableName,
        int sqlDialect,
        const std::map<std::string, std::string> *extraFields
)
{
    std::string quote;
    if (sqlDialect == SQL_MYSQL)
        quote = "`";	// MySQL exceptions for spaces and reserved words
    else
        quote = "\"";

    *output << "CREATE TABLE " << quote << tableName << quote << "(";

    bool isFirst = true;
    if (sqlDialect < flds.size()) {
        for (std::vector<std::pair<const std::string&, std::string> >::const_iterator it(flds[sqlDialect].begin());
             it != flds[sqlDialect].end(); it++) {
            if (isFirst)
                isFirst = false;
            else
                *output << ", ";
            *output << quote << it->first << quote << " " << it->second;
        }
    }
    if (extraFields) {
        for (std::map<std::string, std::string>::const_iterator it(extraFields->begin()); it != extraFields->end(); it++)
        {
            if (isFirst)
                isFirst = false;
            else
                *output << ", ";
            *output << quote << it->first << quote << " " << it->second;
        }
    }

    *output << ");";
}

/**
 * Return CREATE table SQL clause
 * @param env packet declaratuions
 * @param outputFormat 3- sql, 4- Sql
 * @param sqlDialect 0- PostgeSQL, 1- MySQL, 1- Firebird
 * @param extraValues  <optional field name>=<SQL type name>
 * @return empty string if fails
 */
std::string createTableSQLClause(
	int outputFormat,
	int sqlDialect,
	const std::map<std::string, std::string> *extraValues
)
{
    std::stringstream ss;
    switch (outputFormat) {
        case OUTPUT_FORMAT_SQL2:
            createTableSQLClause1(&ss, fldNTypes, tableName, sqlDialect, extraValues);
            break;
        default:
            createTableSQLClause1(&ss, fldNTypes, tableName, sqlDialect, extraValues);
    }
    return ss.str();
}

/**
 * Parse packet by declaration to stream
 * @param env packet declaratuions
 * @param outputFormat 0- json(default), 1- csv, 2- tab, 3- sql, 4- Sql, 5- pbtext, 6- dbg, 7- hex, 8- bin
 * @param sqlDialect 0- PostgeSQL, 1- MySQL, 1- Firebird
 * @param packets data
 * @param tableAliases <table alias>=<SQL table name>
 * @param fieldAliases <field name>=<SQL column name>
 * @param properties  <optional field name>=<SQL type name>
 * @return empty string if fails
 */
void parsePacketStream(
        std::ostream *output,
        const std::vector <std::vector <std::pair<const std::string&, std::string> >> flds,
        int outputFormat,
        int sqlDialect,
        const std::vector<std::string> &valueStrings,
        const std::map<std::string, std::string> *extraFields
) {
    std::string quote;
    std::string quote2;
    if (sqlDialect == SQL_MYSQL) {
        quote = "`";    // MySQL exceptions for spaces and reserved words
        quote2 = "'";
    } else {
        quote = "\"";
        quote2 = "'";
    }

    *output << "INSERT INTO " << quote << tableName << quote << "(";

    // fields
    bool isFirst = true;
    int c = 0;
    if (sqlDialect < flds.size()) {
        for (std::vector<std::pair<const std::string&, std::string> >::const_iterator it(flds[sqlDialect].begin() + 1);
            it != flds[sqlDialect].end(); it++) {
            if (isFirst) {
                isFirst = false;
            } else
                *output << ", ";
            *output << quote << it->first << quote;
            c++;
        }
    }
    if (extraFields) {
        for (std::map<std::string, std::string>::const_iterator it(extraFields->begin()); it != extraFields->end(); it++)
        {
            if (isFirst)
                isFirst = false;
            else
                *output << ", ";
            *output << quote << it->first << quote;
        }
    }

    // values

    *output << ") VALUES (";

    isFirst = true;
    c = 0;
    if (sqlDialect < flds.size()) {
        for (std::vector<std::pair<const std::string&, std::string> >::const_iterator it(flds[sqlDialect].begin());
            it != flds[sqlDialect].end(); it++) {
            if (c >= valueStrings.size())
                break;
            if (isFirst) {
                isFirst = false;
            } else
                *output << ", ";
            if (c >= 7)
                *output << quote2;
            *output << valueStrings[c];
            if (c >= 7)
                *output << quote2;
            c++;
        }
    }
    if (extraFields) {
        for (std::map<std::string, std::string>::const_iterator it(extraFields->begin()); it != extraFields->end(); it++)
        {
            if (isFirst)
                isFirst = false;
            else
                *output << ", ";
            *output << quote << it->first << quote;
        }
    }
    *output << ");";
}

/**
 * Parse packet by declaration
 * @param env packet declaratuions
 * @param outputFormat 0- json(default), 1- csv, 2- tab, 3- sql, 4- Sql, 5- pbtext, 6- dbg, 7- hex, 8- bin
 * @param sqlDialect 0- PostgreSQL, 1- MySQL, 2- Firebird
 * @param packets data
 * @param extraValues <optional field name>=value
 * @return empty string if fails
 */
std::string parsePacketsToSQLClause(
        int outputFormat,
        int sqlDialect,
        const LoggerKosaPackets &packets,
        const std::map<std::string, std::string> *extraValues
)
{
    std::stringstream sout;
    std::vector<std::string> packetValueStrings;
    packets.toStrings(packetValueStrings, "NULL");
    parsePacketStream(&sout, fldNTypes, outputFormat, sqlDialect, packetValueStrings, extraValues);
    return sout.str();
}
