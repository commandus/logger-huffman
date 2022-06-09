/**
 * @brief Logger Huffman v.4 print packet utility
 * @file logger-huffman-print.cpp
 * Copyright (c) 2022 andrey.ivanov@ikfia.ysn.ru
 * Yu.G. Shafer Institute of Cosmophysical Research and Aeronomy of Siberian Branch of the Russian Academy of Sciences
 * MIT license
 *
 * ./logger-huffman-print -f text 486226000203261301001900000000010000000000000000
 *  49260202000000ff00000000000000000000000000000000 492602030000
 * -b 4a0080000207261300011512010115261300003e3d710002 -b 4b26020200cf06aa01e6ff0002deff0003eaff0004dcff00
 * -b 4b26020305e3ff0006e0ff0007e2ff0008ddff0009e2ff00 -b 4b2602040adeff000bdaff000cdfff000debff000ee8ff00
 * -b 4b2602050fcdff0010e6ff0011dfff0012dcff0013e1ff00 -b 4b26020614dcff0015dcff0016eaff0017e5ff0018dfff00
 * -b 4b26020719dfff001adaff001be6ff00
 */
#include <string>
#include <vector>
#include <cstring>
#include <iostream>
#include <sstream>

#include "argtable3/argtable3.h"

#include "logger-huffman.h"
#include "logger-collection.h"
#include "logger-parse.h"
#include "logger-collection.h"
#include "util-compress.h"
#include "errlist.h"
#include "dumb-logger-loader.h"

const std::string programName = "logger-huffman-print";

typedef enum {
    LOGGER_OUTPUT_FORMAT_PG = 0,
    LOGGER_OUTPUT_FORMAT_MYSQL = 1,
    LOGGER_OUTPUT_FORMAT_FB = 2,
    LOGGER_OUTPUT_FORMAT_SQLITE = 3,

  	LOGGER_OUTPUT_FORMAT_JSON = 4,			// default
	LOGGER_OUTPUT_FORMAT_TEXT = 5,		    // text
    LOGGER_OUTPUT_FORMAT_TABLE = 6		    // table

} LOGGER_OUTPUT_FORMAT;

#define LOGGER_OUTPUT_FORMAT_EN_STRING_COUNT    7
const char *LOGGER_OUTPUT_FORMAT_EN_STRING[LOGGER_OUTPUT_FORMAT_EN_STRING_COUNT] = {
    "postgresql",
    "mysql",
    "firebird",
    "sqlite",
    "json",
    "text",
    "table"
};

typedef enum {
    MODE_PACKET = 0,
    MODE_COMPRESS = 1,
    MODE_DECOMPRESS = 2
} MODE;

const char *LOGGER_MODE_EN_STRINGS[3] = {
        "packet",
        "compress",
        "decompress"
};

/**
 * Used in command line help
 * @return list of modes and formats
 */
std::string formatModeList() {
    std::stringstream ss;
    for (int i = 0; i < LOGGER_OUTPUT_FORMAT_EN_STRING_COUNT; i++) {
        ss << LOGGER_OUTPUT_FORMAT_EN_STRING[i] << "|";
    }
    ss << LOGGER_MODE_EN_STRINGS[1] << "|" << LOGGER_MODE_EN_STRINGS[2];
    return ss.str();
}

class LoggerHuffmanPrintConfiguration {
public:
    std::vector<std::string> values;            // packet data
    std::vector<std::string> baseValues;        // "base" packet data
    std::string passportDir;                    // passport directory
    LOGGER_OUTPUT_FORMAT outputFormat;          // default 0- JSON
    bool readStdin;                             // read stdin
    MODE processingMode;                        // default packet processing
    int verbosity;                              // verbosity level
    bool printCreateClauses;                    // print out create clauses
};

void onLoggerParserLog(
    void *env,
    int level,
    int modulecode,
    int errorcode,
    const std::string &message
)
{
    // packet parser error code and error description
    std::cerr << "Error " << errorcode << ": " << message << std::endl;
}

/**
 * Parse command linelogger-huffman-impl
 * Return 0- success
 *        1- show help and exit, or command syntax error
 *        2- output file does not exists or can not open to write
 **/
int parseCmd(
        LoggerHuffmanPrintConfiguration *config,
        int argc,
        char *argv[])
{
    // device path
    std::string formatModeListString = formatModeList();
    struct arg_str *a_value_hex = arg_strn(NULL, NULL, "<packet>", 0, 100, "Packet data in hex. By default read binary from stdin");
    struct arg_str *a_value_base_hex = arg_strn("b", "base", "<packet>", 0, 100, "Base packet data in hex");
    struct arg_lit *a_value_stdin = arg_lit0("r", "read", "Read binary data from stdin");
    struct arg_str *a_output_format = arg_str0("f", "format", formatModeListString.c_str(),"Default json");

    struct arg_str *a_passport_dir = arg_str0("p", "passport", "<dir|file>", "Logger passports directory or file name");
    struct arg_lit *a_print_create_clauses = arg_lit0("c", "create", "print 'CREATE ..' clauses");

    struct arg_lit *a_verbosity = arg_litn("v", "verbose", 0, 3, "Set verbosity level");
    struct arg_lit *a_help = arg_lit0("?", "help", "Show this help");
    struct arg_end *a_end = arg_end(20);

    void *argtable[] = {
        a_value_hex, a_value_stdin, a_value_base_hex, a_output_format, a_passport_dir,
        a_print_create_clauses, a_verbosity, a_help, a_end
    };

    // verify the argtable[] entries were allocated successfully
    if (arg_nullcheck(argtable) != 0) {
        arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
        return 1;
    }
    // Parse the command line as defined by argtable[]
    int nerrors = arg_parse(argc, argv, argtable);

    if ((nerrors == 0) && (a_help->count == 0)) {
        config->outputFormat = LOGGER_OUTPUT_FORMAT_JSON;
        config->processingMode = MODE_PACKET;
        config->printCreateClauses = a_print_create_clauses->count > 0;

        if (a_passport_dir->count) {
            config->passportDir = *a_passport_dir->sval;
        }
        if (a_output_format->count) {
            std::string s(*a_output_format->sval);
            for (int i = 0; i < 7; i++) {
                if (s == LOGGER_OUTPUT_FORMAT_EN_STRING[i]) {
                    config->outputFormat = (LOGGER_OUTPUT_FORMAT) i;
                    break;
                }
            }
            for (int i = 0; i < 3; i++) {
                if (s == LOGGER_MODE_EN_STRINGS[i]) {
                    config->processingMode = (MODE) i;
                    break;
                }
            }
        }
        config->verbosity = a_verbosity->count;
        config->readStdin = a_value_stdin->count > 0;
        if (a_value_hex->count) {
            for (int i = 0; i < a_value_hex->count; i++) {
                config->values.push_back(hex2binString(a_value_hex->sval[i], strlen(a_value_hex->sval[i])));
            }
        } else {
            if (config->processingMode == MODE_PACKET && config->readStdin) {
                // read from stdin
                std::istreambuf_iterator<char> begin(std::cin), end;
                config->values.push_back(std::string(begin, end));
            }
        }
    }

    for (int i = 0; i < a_value_base_hex->count; i++) {
        config->baseValues.push_back(hex2binString(a_value_base_hex->sval[i], strlen(a_value_base_hex->sval[i])));
    }

    // special case: '--help' takes precedence over error reporting
    if ((a_help->count) || nerrors) {
        if (nerrors)
            arg_print_errors(stderr, a_end, programName.c_str());
        std::cerr << "Usage: " << programName << std::endl;
        arg_print_syntax(stderr, argtable, "\n");
        std::cerr
            << "Print logger packet" << std::endl
            << "  logger-huffman-print C0004A..." << std::endl;

        arg_print_glossary(stderr, argtable, "  %-25s %s\n");
        arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));

        return ERR_LOGGER_HUFFMAN_COMMAND_LINE_HELP;
    }

    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return 0;
}

static void printErrorAndExit(
    int errCode
)
{
    std::cerr << ERR_MESSAGE << errCode << ": " << strerror_logger_huffman(errCode) << std::endl;
    exit(errCode);
}

const uint32_t DEVICE_ADDR_INT = 42;

int main(int argc, char **argv)
{
    LoggerHuffmanPrintConfiguration config;
    int r = parseCmd(&config, argc, argv);
    if (r == ERR_LOGGER_HUFFMAN_COMMAND_LINE_HELP)
        exit(r);
    if (r != 0)
        printErrorAndExit(r);

    switch (config.processingMode) {
        case MODE_COMPRESS:
            if (config.readStdin) {
                // read from stdin
                encodeHuffmanStream(std::cout, std::cin);
            } else {
                // read from command line
                for (int i = 0; i < config.values.size(); i++) {
                    encodeHuffman(std::cout, config.values[i].c_str(), config.values[i].size());
                }
            }
            return 0;
        case MODE_DECOMPRESS:
            if (config.readStdin) {
                // read from stdin
                decodeHuffmanStream(std::cout, std::cin);
            } else {
                // read from command line
                for (int i = 0; i < config.values.size(); i++) {
                    std::string s = config.values[i];
                    decodeHuffman(std::cout, s.c_str(), s.size());
                }
            }
            return 0;
        default:    // MODE_PACKET
            break;
    }

    DumbLoggerKosaPacketsLoader lkl;
    // set "base" loader
    LoggerKosaCollector lkcBase;
    if (!config.baseValues.empty()) {
        lkcBase.put(DEVICE_ADDR_INT, config.baseValues);
        lkl.setCollection(&lkcBase);
        if (config.verbosity > 2) {
            std::cerr << "Base packets: " << lkcBase.packetsToString() << std::endl;
        }
    }

    void *loggerParserEnv = initLoggerParser(config.passportDir, onLoggerParserLog, &lkl);

    if (config.printCreateClauses) {
        std::cout << loggerSQLCreateTable1(config.outputFormat, nullptr, "\n") << std::endl;
        doneLoggerParser(loggerParserEnv);
        exit(0);
    }


    LoggerKosaCollector *c = (LoggerKosaCollector*) getLoggerKosaCollector(loggerParserEnv);
    // LOGGER_PACKET_TYPE t = c->put(DEVICE_ADDR_INT, config.values);
    // LOGGER_PACKET_TYPE t = (LOGGER_PACKET_TYPE) loggerParsePackets(loggerParserEnv, DEVICE_ADDR_INT, config.values);
    LOGGER_PACKET_TYPE t;
    for (auto it(config.values.begin()); it != config.values.end(); it++) {
        t = (LOGGER_PACKET_TYPE) loggerParsePacket(loggerParserEnv, DEVICE_ADDR_INT, *it);
    }

    if (t == LOGGER_PACKET_UNKNOWN)
        printErrorAndExit(ERR_LOGGER_HUFFMAN_INVALID_PACKET);

    std::string outputString;

    switch (config.outputFormat) {
        case LOGGER_OUTPUT_FORMAT_JSON:
            if (config.verbosity)
                outputString = c->packetsToJsonString();
            else
                outputString = c->toJsonString();
            break;
        case LOGGER_OUTPUT_FORMAT_TEXT:
            if (config.verbosity)
                outputString = c->packetsToString();
            else
                outputString = c->toString();
            break;
        case LOGGER_OUTPUT_FORMAT_TABLE:
            outputString = c->toTableString();
            break;
        default: // LOGGER_OUTPUT_FORMAT_PG LOGGER_OUTPUT_FORMAT_MYSQL LOGGER_OUTPUT_FORMAT_FB LOGGER_OUTPUT_FORMAT_SQLITE
            outputString = loggerSQLInsertPackets1(loggerParserEnv, config.outputFormat);
    }
    std::cout << outputString << std::endl;
    doneLoggerParser(loggerParserEnv);
}
