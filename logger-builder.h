#ifndef LOGGER_BUILDER_H_
#define LOGGER_BUILDER_H_ 1

#include <vector>
#include <string>
#include <inttypes.h>

class LoggerMeasurements {
public:
    time_t time;
    uint16_t measure;
    uint8_t kosa;
    uint8_t kosa_year;
    float vcc;
    float vbat;
    std::vector<double> temperature;

    LoggerMeasurements();
};

class LoggerBuilder {
private:
    static size_t buildHuffmanChunk(std::vector<std::string> &retVal, const LoggerMeasurements &value, const std::vector<double> &baseTemperature,
                                    size_t firstPacketMeasurementCount, size_t otherPacketMeasurementCount);
public:
    LoggerBuilder();
    /**
     * Produce 4a/4b "base" packets
     * @param retVal produced packets, up to 24 bytes long
     * @param value measurement(time, temperatures, voltage)
     */
    static void build(std::vector<std::string> &retVal, const LoggerMeasurements &value);
    /**
     * Produce 48/49 "delta" packets
     * @param retVal produced packets, up to 24 bytes long
     * @param value measurement(time, temperatures, voltage)
     * @param baseTemperature measurements base for delta
     */
    static void build(std::vector<std::string> &retVal, const LoggerMeasurements &value, const std::vector<double> &baseTemperature);
    /**
     * Produce 4c/4d "delta huffman" packets
     * @param retVal produced packets, up to 24 bytes long
     * @param value measurement(time, temperatures, voltage)
     * @param baseTemperature measurements base for delta
     */
    static void buildHuffman(std::vector<std::string> &retVal, const LoggerMeasurements &value, const std::vector<double> &baseTemperature);
};

#endif
