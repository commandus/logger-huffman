#include <string>
#include <cassert>
#include <iostream>
#include <sstream>

#include "logger-collection.h"
#include "logger-builder.h"

#include <cstdlib>
#include "argtable3/argtable3.h"

static const std::string programName = "test-huffman";

std::string composeHuffman_38_19_28(
    double t0,
    double inc,
    double delta
)
{
    LoggerMeasurements mBase;
    mBase.kosa = 38;
    mBase.kosa_year = 19;
    mBase.measure = 42;
    time(&mBase.time);
    mBase.vcc = 4.5;
    mBase.vbat = 1.5;

    LoggerMeasurements m;
    m.kosa = 38;
    m.kosa_year = 19;
    m.measure = 42;
    time(&m.time);
    m.vcc = 4.5;
    m.vbat = 1.5;

    int cnt = 28;

    for (int i = 0; i < cnt; i++) {
        m.temperature.push_back(t0);
        mBase.temperature.push_back(t0 + delta);
        t0 += inc;
    }

    std::vector<std::string> packets;
    // LoggerBuilder::build(packets, m, mBase.temperature);
    LoggerBuilder::buildHuffman(packets, m, mBase.temperature);
    std::stringstream ss;

    for (int i = 0; i < packets.size(); i++) {
        ss << bin2hexString(packets[i]) << std::endl;
    }

    return ss.str();
}

int parseCmd(
    double &value_initial,
    double &value_step,
    double &value_delta,
    int argc,
    char *argv[]
)
{
    // device path
    struct arg_str *a_value_initial = arg_str0("i", "init", "<float>", "Initial temperature e.g -i -10.0");
    struct arg_str *a_value_step = arg_str0("s", "step", "<float>", "Step temperature e.g. -s 0.0625");
    struct arg_str *a_value_delta = arg_str0("d", "delta", "<float>", "Delta temperature e.g. -d 0.5");

    struct arg_lit *a_verbosity = arg_litn("v", "verbose", 0, 3, "Set verbosity level");
    struct arg_lit *a_help = arg_lit0("?", "help", "Show this help");
    struct arg_end *a_end = arg_end(20);

    void *argtable[] = {
            a_value_initial, a_value_step, a_value_delta, a_verbosity, a_help, a_end
    };

    // verify the argtable[] entries were allocated successfully
    if (arg_nullcheck(argtable) != 0) {
        arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
        return 1;
    }
    // Parse the command line as defined by argtable[]
    int nerrors = arg_parse(argc, argv, argtable);

    value_initial = 0.0;
    value_step = 0.0;
    value_delta = 0.0;

    if ((nerrors == 0) && (a_help->count == 0)) {
        if (a_value_initial->count) {
            value_initial = strtod(*a_value_initial->sval, nullptr);
        }
        if (a_value_step->count) {
            value_step = strtod(*a_value_step->sval, nullptr);
        }
        if (a_value_delta->count) {
            value_delta = strtod(*a_value_delta->sval, nullptr);
        }
    }

    // special case: '--help' takes precedence over error reporting
    if ((a_help->count) || nerrors) {
        if (nerrors)
            arg_print_errors(stderr, a_end, programName.c_str());
        std::cerr << "Usage: " << programName << std::endl;
        arg_print_syntax(stderr, argtable, "\n");
        std::cerr
                << "Test huffman packet" << std::endl
                << "  test-huffman -i -10.0 -s 0.0625 -d 0.5" << std::endl;

        arg_print_glossary(stderr, argtable, "  %-25s %s\n");
        arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));

        return -1;
    }

    arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
    return 0;
}

int main(int argc, char **argv)
{
    double value_initial;
    double value_step;
    double value_delta;

    parseCmd(value_initial, value_step, value_delta, argc, argv);
    // -10.0, 0.0625, 0.5
    std::cout << composeHuffman_38_19_28(value_initial, value_step, value_delta) << std::endl;
}
