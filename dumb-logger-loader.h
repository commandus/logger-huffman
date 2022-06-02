#ifndef DUMB_LOGGER_LOADER_H
#define DUMB_LOGGER_LOADER_H     1

#include "logger-collection.h"

/**
 * Dumb Logger plume loader class implementation
 */
class DumbLoggerKosaPacketsLoader: public LoggerKosaPacketsLoader {
public:
    LoggerKosaCollector *collection;
    DumbLoggerKosaPacketsLoader();

    bool load(LoggerKosaPackets &retVal, uint32_t addr) override;

    void setCollection(LoggerKosaCollector *aCollection);
};

#endif
