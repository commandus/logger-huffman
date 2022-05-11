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
/*
#include "utilcompress.h"
*/
#include "errlist.h"

const std::string programName = "logger-huffman-print";

#ifdef _MSC_VER
#undef ENABLE_TERM_COLOR
#else
#define ENABLE_TERM_COLOR	1
#endif

typedef enum {
    LOGGER_OUTPUT_FORMAT_PG = 0,
    LOGGER_OUTPUT_FORMAT_MYSQL = 1,
    LOGGER_OUTPUT_FORMAT_FB = 2,
    LOGGER_OUTPUT_FORMAT_SQLITE = 3,

  	LOGGER_OUTPUT_FORMAT_JSON = 4,			// default
	LOGGER_OUTPUT_FORMAT_TEXT = 5,		    // text
    LOGGER_OUTPUT_FORMAT_TABLE = 6		    // table

} LOGGER_OUTPUT_FORMAT;

typedef enum {
    MODE_PACKET = 0,
    MODE_COMPRESS = 1,
    MODE_DECOMPRESS = 2
} MODE;

static const std::string SQL_DIALECT_NAME[] = {
    "postgresql", "mysql", "firebird", "sqlite"
};

class LoggerHuffmanPrintConfiguration {
public:
    std::vector<std::string> values;            // packet data
    std::vector<std::string> baseValues;        // "base" packet data
    std::string passportDir;                    // passport directory
    LOGGER_OUTPUT_FORMAT outputFormat;          // default 0- JSON
    bool readStdin;                             // read stdin
    MODE processingMode;                        // default packet processing
    int verbosity;                              // verbosity level
    bool hasValue;
};

class DumbLoggerKosaPacketsLoader: public LoggerKosaPacketsLoader {
public:
    LoggerKosaCollection *collection;
    DumbLoggerKosaPacketsLoader()
        : collection(nullptr)
    {

    }

    LoggerKosaPackets *load(uint8_t kosa, uint8_t year2000) override {
        if (collection && (!collection->koses.empty()))
            return &collection->koses[0];
        else
            return nullptr;
    }

    void setCollection(LoggerKosaCollection *aCollection) {
        collection = aCollection;
    }
};


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
    struct arg_str *a_value_hex = arg_strn(NULL, NULL, "<packet>", 0, 100, "Packet data in hex. By default read binary from stdin");
    struct arg_str *a_value_base_hex = arg_strn("b", "base", "<packet>", 0, 100, "Base packet data in hex");
    struct arg_lit *a_value_stdin = arg_lit0("r", "read", "Read binary data from stdin");
    struct arg_str *a_output_format = arg_str0("f", "format", "json|text|table|postgresql|mysql|firebird|sqlite|compress|decompress", "Default json");

    struct arg_str *a_passport_dir = arg_str0("p", "passport", "<dir|file>", "Logger passports directory or file name");

    struct arg_lit *a_verbosity = arg_litn("v", "verbose", 0, 3, "Set verbosity level");
    struct arg_lit *a_help = arg_lit0("?", "help", "Show this help");
    struct arg_end *a_end = arg_end(20);

    void *argtable[] = {
        a_value_hex, a_value_stdin, a_value_base_hex, a_output_format, a_passport_dir,
        a_verbosity, a_help, a_end
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
        if (a_passport_dir->count) {
            config->passportDir = *a_passport_dir->sval;
        }
        if (a_output_format->count) {
            std::string s(*a_output_format->sval);
            if (s == "text")
                config->outputFormat = LOGGER_OUTPUT_FORMAT_TEXT;
            if (s == "table")
                config->outputFormat = LOGGER_OUTPUT_FORMAT_TABLE;
            if (s == "postgresql")
                config->outputFormat = LOGGER_OUTPUT_FORMAT_PG;
            if (s == "mysql")
                config->outputFormat = LOGGER_OUTPUT_FORMAT_MYSQL;
            if (s == "firebird")
                config->outputFormat = LOGGER_OUTPUT_FORMAT_FB;
            if (s == "sqlite")
                config->outputFormat = LOGGER_OUTPUT_FORMAT_SQLITE;
            config->processingMode = MODE_PACKET;
            if (s == "compress")
                config->processingMode = MODE_COMPRESS;
            if (s == "decompress")
                config->processingMode = MODE_DECOMPRESS;
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
        config->baseValues.push_back(hex2binString(a_value_base_hex->sval[i], strlen(*a_value_base_hex->sval)));
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

const char* TAB_DELIMITER = "\t";

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
                    std::cerr << "==" << bin2hexString(s) << std::endl;
                    decodeHuffman(std::cout, s.c_str(), s.size());
                }
            }
            return 0;
    }

    void *loggerParserEnv = initLoggerParser(config.passportDir, nullptr);
    LoggerKosaCollection *c = (LoggerKosaCollection*) getLoggerKosaCollection(loggerParserEnv);

    LOGGER_PACKET_TYPE t = c->put(config.values);

    if (t == LOGGER_PACKET_UNKNOWN)
        printErrorAndExit(ERR_LOGGER_HUFFMAN_INVALID_PACKET);

    // set "base" loader
    LoggerKosaCollection lkcBase;
    DumbLoggerKosaPacketsLoader lkl;
    if (!config.baseValues.empty()) {
        lkcBase.put(config.baseValues);
        lkl.setCollection(&lkcBase);
        c->setLoggerKosaPacketsLoader(&lkl);
        if (config.verbosity > 2) {
            std::cerr << "Base packets: " << lkcBase.toString() << std::endl;
        }
    }

    std::string outputString;

    switch (config.outputFormat) {
        case LOGGER_OUTPUT_FORMAT_JSON:
            outputString = c->toJsonString();
            break;
        case LOGGER_OUTPUT_FORMAT_TEXT:
            outputString = c->toString();
            break;
        case LOGGER_OUTPUT_FORMAT_TABLE:
            outputString = c->toTableString();
            break;
        default: // LOGGER_OUTPUT_FORMAT_PG LOGGER_OUTPUT_FORMAT_MYSQL LOGGER_OUTPUT_FORMAT_FB LOGGER_OUTPUT_FORMAT_SQLITE
            outputString = sqlInsertPackets1(loggerParserEnv, config.outputFormat);
    }
    std::cout << outputString << std::endl;
    doneLoggerParser(loggerParserEnv);
}
