#include "dumb-logger-loader.h"

DumbLoggerKosaPacketsLoader::DumbLoggerKosaPacketsLoader()
    : collection(nullptr)
{

}

bool DumbLoggerKosaPacketsLoader::load(
    LoggerKosaPackets &retVal,
    uint32_t addr
)
{
    if (collection && (!collection->koses.empty())) {
        retVal = collection->koses[0];
        return true;
    } else
        return false;
}

void DumbLoggerKosaPacketsLoader::setCollection(
    LoggerKosaCollector *aCollection
) {
    collection = aCollection;
}

