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
