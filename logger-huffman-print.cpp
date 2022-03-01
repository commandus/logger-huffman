/**
 * @brief Logger Huffman v.4 print packet utility
 * @file logger-huffman-print.cpp
 * Copyright (c) 2022 andrey.ivanov@ikfia.ysn.ru
 * Yu.G. Shafer Institute of Cosmophysical Research and Aeronomy of Siberian Branch of the Russian Academy of Sciences
 * MIT license
 */
#include <string>
#include <cstring>
#include <iostream>
#include <sstream>
#include "argtable3/argtable3.h"

#include "logger-huffman.h"
#include "logger-huffman-impl.h"
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

class LoggerHuffmanPrintConfiguration {
public:
    std::string value;          // packet data
    int mode;                   // 0- binary from stdin, 1- hex in command line parameter
    int verbosity;              // verbosity level
    bool hasValue;
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
    struct arg_str *a_value_hex = arg_str0(NULL, NULL, "<packet>", "Packet data in hex. By default read binary from stdin");

    struct arg_lit *a_value_stdin = arg_lit0("r", "read", "Read binary data from stdin");

    struct arg_lit *a_verbosity = arg_litn("v", "verbose", 0, 3, "Set verbosity level");
    struct arg_lit *a_help = arg_lit0("?", "help", "Show this help");
    struct arg_end *a_end = arg_end(20);

    void *argtable[] = {
        a_value_hex, a_value_stdin,
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
        config->verbosity = a_verbosity->count;
        if (a_value_hex->count) {
            config->value = hex2binString(*a_value_hex->sval, strlen(*a_value_hex->sval));
        } else {
            if (a_value_stdin->count) {
                // read from stdin
                std::istreambuf_iterator<char> begin(std::cin), end;
                config->value = std::string(begin, end);
            }
        }
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

    LoggerPacket loggerPacket(config.value.c_str(), config.value.size());

    if (loggerPacket.errCode)
        printErrorAndExit(loggerPacket.errCode);
    std::string s = loggerPacket.toString();

    std::cout << s << std::endl;
}
