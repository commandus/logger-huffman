#include <iostream>
#include "logger-builder.h"

#include "logger-huffman.h"

static LOGGER_PACKET_FIRST_HDR* setFirstHeader(
    const void *buffer,
    const LoggerMeasurements &measurements,
    uint8_t typ,
    uint8_t bytesPerSample
)
{
    LOGGER_PACKET_FIRST_HDR* h1 = (LOGGER_PACKET_FIRST_HDR*) buffer;
    h1->typ = typ;
    h1->status.rfu = 0;
    h1->status.command_change = 0;
    h1->status.data_bytes = bytesPerSample;
    // calc size in packets
    switch (typ) {
        case 0x4a:
            // one extra packet for first header and measurement
            h1->packets = measurements.temperature.size() / ((24 - sizeof(LOGGER_PACKET_SECOND_HDR)) / sizeof(LOGGER_DATA_TEMPERATURE_RAW)) + 1;
            if (measurements.temperature.size() % ((24 - sizeof(LOGGER_PACKET_SECOND_HDR)) / sizeof(LOGGER_DATA_TEMPERATURE_RAW)))
                h1->packets++;
            break;
        case 0x48:
            {
                int cnt = measurements.temperature.size();
                // 6 or 3 measurements in first packet
                int measurementsInFirstPacket = 6 / bytesPerSample;
                if (measurementsInFirstPacket > cnt)
                    measurementsInFirstPacket = cnt;
                int measurementsInSecondPackets = cnt - measurementsInFirstPacket;
                int secondPackets = measurementsInSecondPackets * bytesPerSample / 20;
                if (measurementsInSecondPackets > secondPackets * 20 / bytesPerSample)
                    secondPackets++;
                h1->packets = secondPackets + 1;    // + first header packet
            }
            break;
        case 0x4c:
            h1->packets = measurements.temperature.size() / ((24 - sizeof(LOGGER_PACKET_SECOND_HDR)) / sizeof(LOGGER_DATA_TEMPERATURE_RAW)) + 1;
            if (measurements.temperature.size() % ((24 - sizeof(LOGGER_PACKET_SECOND_HDR)) / sizeof(LOGGER_DATA_TEMPERATURE_RAW)))
                h1->packets++;
            break;
        default:
            h1->packets = 1;
    }
    h1->size = measurements.temperature.size() * 8;       // ?!! TODO
    h1->kosa = measurements.kosa;
    h1->kosa_year = measurements.kosa_year;
    h1->measure = measurements.measure;
    return h1;
}

static LOGGER_PACKET_SECOND_HDR *setSecondHeader(
    const void *buffer,
    const LoggerMeasurements &measurements,
    uint8_t typ,
    uint8_t packetNumber
)
{
    LOGGER_PACKET_SECOND_HDR* h2 = (LOGGER_PACKET_SECOND_HDR*) buffer;
    h2->typ = typ;
    h2->kosa = measurements.kosa;
    h2->measure = measurements.measure;
    h2->packet = packetNumber;
    return h2;
}

/**
 * Set temperature values to the base packet
 * @param buffer pointer to target packet buffer
 * @param measurements copy from
 * @param packetIndex 1- headers, 2..- data
 * @return count of copied bytes
 */
static size_t setMeasurementsData(
    const void *buffer,
    const LoggerMeasurements &measurements,
    uint8_t packetIndex
)
{
    int ofs = packetIndex * 5;
    size_t sz = measurements.temperature.size();
    int c = 0;
    for (; c < 5; c++) {
        if (ofs >= sz)
            break;  // range out
        LOGGER_DATA_TEMPERATURE_RAW *t = (LOGGER_DATA_TEMPERATURE_RAW *) buffer + c;
        t->sensor = ofs;
        double_2_TEMPERATURE_2_BYTES(&t->value, measurements.temperature[ofs]);
        ofs++;
    }
    return c * sizeof(LOGGER_DATA_TEMPERATURE_RAW);
}

/**
 * Set temperature values to the delta packet
 * @param buffer pointer to target packet buffer
 * @param diffInt deltas
 * @param packetIndex 1- headers, 2..- data
 * @param bytesPerSample 1 or 2 bytes dor deltas
 * @return count of copied bytes
 */
static size_t setDeltaMeasurementsData(
    const char *buffer,
    const std::vector<int16_t> &diffInt,
    int packetIndex,
    int bytesPerSample
) {
    int ofs = (6 + packetIndex * 20) / bytesPerSample;
    size_t sz = diffInt.size();
    int c = 0;
    for (; c < 20 / bytesPerSample; c++) {

        if (ofs >= sz)
            break;  // range out
        if (bytesPerSample == 1) {
            int8_t *t = (int8_t *) buffer + c;
            *t = diffInt[ofs];

        } else {
            int16_t *t = (int16_t *) buffer + c;
            *t = diffInt[ofs];
        }
        ofs++;
    }
    return c * bytesPerSample;
}

/**
 * Set temperature values to the delta packet
 * @param buffer pointer to target packet buffer
 * @param diffInt deltas
 * @param packetIndex 1- headers, 2..- data
 * @param bytesPerSample 1 or 2 bytes dor deltas
 * @return count of copied bytes
 */
static size_t setDeltaMeasurementsDataFirstPacket(
    const char *buffer,
    const std::vector<int16_t> &diffInt,
    int bytesPerSample
) {
    size_t sz = diffInt.size();
    int c = 0;
    for (; c < 6 / bytesPerSample; c++) {
        if (c >= sz)
            break;  // range out
        if (bytesPerSample == 1) {
            int8_t *t = (int8_t *) buffer + c;
            *t = diffInt[c];
        } else {
            int16_t *t = (int16_t *) buffer + c;
            *t = diffInt[c];
        }
    }
    return c * bytesPerSample;
}

static LOGGER_MEASUREMENT_HDR *setMeasureHeader(
    const void *buffer,
    const LoggerMeasurements &measurements
)
{
    LOGGER_MEASUREMENT_HDR *m = (LOGGER_MEASUREMENT_HDR*) buffer;
    m->memblockoccupation = 0;
    struct tm *ti = localtime(&measurements.time);
    m->seconds = ti->tm_sec;			// 0-60 (1 leap second)
    m->minutes = ti->tm_min ;			// 0-59
    m->hours = ti->tm_hour; 			// 0-23
    m->day = ti->tm_mday;   			// 1-31
    m->month = ti->tm_mon;  			// 1-12
    m->year = ti->tm_year - 100;	    // Year	- 1900
    m->kosa = measurements.kosa;
    m->kosa_year = measurements.kosa_year;
    m->rfu1 = 0;							// 9 reserved
    m->rfu2 = 0;							// 10 reserved
    m->vcc = double2vcc(measurements.vcc);
    m->vbat = double2vbat(measurements.vbat);
    m->pcnt = 0;
    m->used = measurements.measure;
    return m;
}

static LOGGER_MEASUREMENT_HDR_DIFF *setMeasureDeltaHeader(
    const char *buffer,
    const LoggerMeasurements &measurements
)
{
    LOGGER_MEASUREMENT_HDR_DIFF  *m = (LOGGER_MEASUREMENT_HDR_DIFF*) buffer;
    m->used = measurements.measure;			// 0 record number diff
    m->delta_sec = 10;				        // 2 seconds
    m->kosa = measurements.kosa;			// 3 номер косы в году
    m->kosa_year = measurements.kosa_year;	// 4 год косы - 2000 (номер года последние 2 цифры)
    m->rfu1 = 0;							// 5 reserved
    m->rfu2 = 0;							// 6 reserved
    m->vcc = 0; 							// 7 V cc bus voltage, V
    m->vbat = 0;							// 8 V battery, V
    m->pcnt = 0;							// 9 pages count, Pcnt = ((ds1820_devices << 2) | pages_to_recods)
    return m;
}

LoggerMeasurements::LoggerMeasurements()
    : measure(0), kosa(0), kosa_year(0), vcc(0.0), vbat(0.0)
{
}

LoggerBuilder::LoggerBuilder()
{
}

void LoggerBuilder::build(
    std::vector<std::string> &retVal,
    const LoggerMeasurements &value
)
{
    // first packet firstHeader(8) MeasurementHeader(16)
    std::string firstPacket;
    firstPacket.resize(24);
    LOGGER_PACKET_FIRST_HDR* h1 = setFirstHeader(firstPacket.c_str(), value, 0x4a, 0);   // 8 bytes
    setMeasureHeader(firstPacket.c_str() + sizeof(LOGGER_PACKET_FIRST_HDR), value); // 16 bytes
    retVal.push_back(firstPacket);

    // second packet secondHeader(4) temperature(1..5 * 4)
    for (int i = 1; i < h1->packets; i++) {
        std::string secondPacket;
        secondPacket.resize(24);
        LOGGER_PACKET_SECOND_HDR* h2 = setSecondHeader(secondPacket.c_str(), value, 0x4b, i + 1);   // 4 bytes
        size_t sz = setMeasurementsData(secondPacket.c_str() + sizeof(LOGGER_PACKET_SECOND_HDR), value, i - 1);
        if (sz < 24 - sizeof(LOGGER_PACKET_SECOND_HDR)) // last packet length is variable 8..24 bytes long, resize it
            secondPacket.resize(sz + sizeof(LOGGER_PACKET_SECOND_HDR));
        retVal.push_back(secondPacket);
    }
}

/**
 * Calc temperature differences in 1/16 degrees int
 * @param retDiff if not NULL, return differences
 * @param temperature current values
 * @param baseTemperature values to compare
 * @return return 1 or 2- bytes required to keep max difference
 */
static int calcDeltas(
    std::vector<int16_t> *retDiff,
    const std::vector<double> &temperature,
    const std::vector<double> &baseTemperature
)
{
    int16_t maxDiff = 0;
    std::vector<double>::const_iterator itBase(baseTemperature.begin());
    for (std::vector<double>::const_iterator it(temperature.begin()); it != temperature.end(); it++) {
        if (itBase == baseTemperature.end())
            break;
        TEMPERATURE_2_BYTES t1;
        TEMPERATURE_2_BYTES t2;
        double_2_TEMPERATURE_2_BYTES(&t1, *it);
        double_2_TEMPERATURE_2_BYTES(&t2, *itBase);
        int16_t d = NTOH2(t1.t.t00625) - NTOH2(t2.t.t00625);
        if (abs(d) > maxDiff)
            maxDiff = abs(d);
        if (retDiff)
            retDiff->push_back(d);
        // next
        itBase++;
    }
    if (maxDiff < 0x80)
        return 1;
    return 2;
}

void LoggerBuilder::build(
    std::vector<std::string> &retVal,
    const LoggerMeasurements &value,
    const std::vector<double> &baseTemperature
)
{
    std::vector<int16_t> diffInt;
    int bytesPerSample = calcDeltas(&diffInt, value.temperature, baseTemperature);

    int cnt = diffInt.size();   // diffInt may be less than value.temperature
    // 6 or 3 measurements in first packet
    int measurementsInFirstPacket = 6 / bytesPerSample;
    bool firstPacketShorter24Bytes = false;
    if (measurementsInFirstPacket > cnt) {
        measurementsInFirstPacket = cnt;
        firstPacketShorter24Bytes = true;
    }

    int measurementsInSecondPackets = cnt - measurementsInFirstPacket;
    int secondPackets = measurementsInSecondPackets * bytesPerSample / 20;
    if (measurementsInSecondPackets > secondPackets * 20 / bytesPerSample)
        secondPackets++;

    // first packet firstHeader(8) MeasurementHeader(16)
    std::string firstPacket;
    firstPacket.resize(24);
    setFirstHeader(firstPacket.c_str(), value, 0x48, bytesPerSample);   // 8 bytes
    setMeasureDeltaHeader(firstPacket.c_str() + sizeof(LOGGER_PACKET_FIRST_HDR), value); // 10 bytes

    // add up to 3 or 6 measurements to the first packet
    setDeltaMeasurementsDataFirstPacket(firstPacket.c_str()
        + sizeof(LOGGER_PACKET_FIRST_HDR) + sizeof(LOGGER_MEASUREMENT_HDR_DIFF),
        diffInt, bytesPerSample);

    if (firstPacketShorter24Bytes)
        firstPacket.resize(24 - (6 - (measurementsInFirstPacket * bytesPerSample)));
    retVal.push_back(firstPacket);

    // second packet secondHeader(4) temperature(1..10 or 1..20)
    for (int i = 0; i < secondPackets; i++) {
        std::string secondPacket;
        secondPacket.resize(24);
        setSecondHeader(secondPacket.c_str(), value, 0x49, i + 2);   // 4 bytes
        size_t sz = setDeltaMeasurementsData(secondPacket.c_str() + sizeof(LOGGER_PACKET_SECOND_HDR),
            diffInt, i, bytesPerSample);
        if (sz < 24 - sizeof(LOGGER_PACKET_SECOND_HDR)) // last packet length is variable 8..24 bytes long, resize it
            secondPacket.resize(sz + sizeof(LOGGER_PACKET_SECOND_HDR));
        retVal.push_back(secondPacket);
    }
}

void LoggerBuilder::buildHuffman(
    std::vector<std::string> &retVal,
    const LoggerMeasurements &value,
    const std::vector<double> &baseTemperature
)
{
    std::string firstPacket;
    firstPacket.resize(24);
    setFirstHeader(firstPacket.c_str(), value, 0x4c, 0);

}
