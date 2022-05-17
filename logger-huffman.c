#include <time.h>
#include "platform.h"

#include "logger-huffman.h"
#include "errlist.h"

/**
 * Return LOGGER_PACKET_UNKNOWN if buffer is NULL or size = 0
 * @param retSize return packet size if not NULL
 * @param buffer read data from the buffer
 * @param bufferSize buffer size
 */ 
LOGGER_PACKET_TYPE extractLoggerPacketType(
	size_t *retSize,
	const void *buffer,
	size_t bufferSize
)
{
	if ((!buffer) || (bufferSize == 0))
		return LOGGER_PACKET_UNKNOWN;
	LOGGER_PACKET_TYPE t;
    if ((*(char *) buffer < 0x48) || (*(char *) buffer > 0x4d))
        t =  LOGGER_PACKET_RAW;
    else
        t = (LOGGER_PACKET_TYPE) *(char *) buffer;

	size_t sz = getLoggerPacketTypeSize(t, bufferSize);
	if (retSize)
		*retSize = sz;
	return t;
}

/**
 * Return expected packet size in bytes
 * @param typ packet type
 * @param bufferSize size
 */
size_t getLoggerPacketTypeSize(
	LOGGER_PACKET_TYPE typ,
	size_t bufferSize
)
{
	size_t r;
	switch (typ) {
		case LOGGER_PACKET_RAW:			// raw w/o packet headers. замер, разбитый по пакетам в 24 байта (в hex 48 байт). Используется для передачи 0 замера
			r = sizeof(LOGGER_MEASUREMENT_HDR);
			break;
 		case LOGGER_PACKET_PKT_1:		// with packet header (first). К данным замера добавляются шапки пакетов, для первого 8 байт, для следующих 4 байта/.Используется для передачи 0 замера
			r = sizeof(LOGGER_PACKET_FIRST_HDR) + sizeof(LOGGER_MEASUREMENT_HDR);	// 8 + 16 = 24
			break;
 		case LOGGER_PACKET_PKT_2:		// with packet header (next)
			r = sizeof(LOGGER_PACKET_SECOND_HDR) + 5 * sizeof(LOGGER_DATA_TEMPERATURE_RAW);	// 4 + 5 * 4 = 24
			break;
 		case LOGGER_PACKET_DELTA_1:		// дельты замеров от 0 замера.
            r = sizeof(LOGGER_PACKET_FIRST_HDR) + sizeof(LOGGER_MEASUREMENT_HDR_DIFF) + 6 * sizeof(uint8_t);	// 8 + 10 + (1..6) = 18..24
			break;
		case LOGGER_PACKET_DELTA_2:		// дельты замеров от 0 замера.
            r = sizeof(LOGGER_PACKET_SECOND_HDR) + 20 * sizeof(uint8_t);	// 4 + 20 * 1 = 24
			break;
 		case LOGGER_PACKET_HUFF_1:		// дельты замеров от 0 сжаты каноническим Хафманом по таблице +-4.
		 	r = 24; // max
			break;
		case LOGGER_PACKET_HUFF_2:		// дельты замеров от 0 сжаты каноническим Хафманом по таблице +-4.
			r = 24; // max
			break;
		default:
			// case LOGGER_PACKET_UNKNOWN:
			r = 0;
			break;
	}
	if (r <= bufferSize)
		return r;
	return bufferSize;
}

/**
 * @param retHdr return header
 * @param buffer data
 * @param size buffer size
 */
LOGGER_PACKET_TYPE extractMeasurementHeader(
	LOGGER_MEASUREMENT_HDR **retHdr,
	const void *buffer,
	size_t bufferSize
)
{
	size_t sz;
	LOGGER_PACKET_TYPE t = extractLoggerPacketType(&sz, buffer, bufferSize);
	switch (t) {
		case LOGGER_PACKET_RAW:			// raw w/o packet headers. замер, разбитый по пакетам в 24 байта (в hex 48 байт). Используется для передачи 0 замера
			if (bufferSize < sizeof(LOGGER_MEASUREMENT_HDR))
				return ERR_LOGGER_HUFFMAN_INVALID_PACKET;
			*retHdr = (LOGGER_MEASUREMENT_HDR*) buffer;
			break;
 		case LOGGER_PACKET_PKT_1:		// with packet header (first). К данным замера добавляются шапки пакетов, для первого 8 байт, для следующих 4 байта/.Используется для передачи 0 замера
			if (bufferSize < sizeof(LOGGER_PACKET_FIRST_HDR) + sizeof(LOGGER_MEASUREMENT_HDR))
				return ERR_LOGGER_HUFFMAN_INVALID_PACKET;
		 	*retHdr = (LOGGER_MEASUREMENT_HDR*) ((char *) buffer + sizeof(LOGGER_PACKET_FIRST_HDR));
			break;
		default:
			// case LOGGER_PACKET_UNKNOWN:
			*retHdr = NULL;
			break;
	}
	return t;
}

/**
 * Extract header only
 * @param retHdr return header pointer
 * @param buffer data
 * @param size buffer size
 */
int extractFirstHdr(
	LOGGER_PACKET_FIRST_HDR **retHdr,
	const void *buffer,
	size_t bufferSize
)
{
	size_t sz;
	LOGGER_PACKET_TYPE t = extractLoggerPacketType(&sz, buffer, bufferSize);
	switch (t) {
 		case LOGGER_PACKET_PKT_1:		// with packet header (first). К данным замера добавляются шапки пакетов, для первого 8 байт, для следующих 4 байта/.Используется для передачи 0 замера
        case LOGGER_PACKET_DELTA_1:
			if (bufferSize < sizeof(LOGGER_PACKET_FIRST_HDR)) {
                *retHdr = NULL;
                return ERR_LOGGER_HUFFMAN_INVALID_PACKET;
            }
            *retHdr = (LOGGER_PACKET_FIRST_HDR*) buffer;
			return 0;
		default:
            *retHdr = NULL;
		 	return ERR_LOGGER_HUFFMAN_INVALID_PACKET;
	}
}

int16_t extractSecondHdr(
	LOGGER_PACKET_SECOND_HDR **retHdr,
	const void *buffer,
	size_t bufferSize
)
{
	size_t sz;
	LOGGER_PACKET_TYPE t = extractLoggerPacketType(&sz, buffer, bufferSize);
	switch (t) {
 		case LOGGER_PACKET_PKT_2:		// with packet header (second)
        case LOGGER_PACKET_DELTA_2:
			if (bufferSize < sizeof(LOGGER_PACKET_SECOND_HDR)) {
                *retHdr = NULL;
                return ERR_LOGGER_HUFFMAN_INVALID_PACKET;
            }
			*retHdr = (LOGGER_PACKET_SECOND_HDR*) buffer;
			return 0;
		default:
            *retHdr = NULL;
		 	return ERR_LOGGER_HUFFMAN_INVALID_PACKET;
	}
}

LOGGER_MEASUREMENT_HDR_DIFF *extractDiffHdr(
    const void *buffer,
    size_t bufferSize
)
{
    if (bufferSize < sizeof(LOGGER_PACKET_FIRST_HDR) + sizeof(LOGGER_MEASUREMENT_HDR_DIFF))
        return NULL;
    return (LOGGER_MEASUREMENT_HDR_DIFF *) ((char *) buffer + sizeof(LOGGER_PACKET_FIRST_HDR));
}

LOGGER_DATA_TEMPERATURE_RAW *extractSecondHdrData(
	int p,
	const void *buffer,
	size_t bufferSize
)
{
    LOGGER_DATA_TEMPERATURE_RAW *r =
            (LOGGER_DATA_TEMPERATURE_RAW *) (buffer + sizeof(LOGGER_PACKET_SECOND_HDR))
            + p;
    if ((char *) r >= (char *) buffer + bufferSize)
        return NULL;
	return r;
}

double extractMeasurementHeaderData(
	LOGGER_DATA_TEMPERATURE_RAW **retval,
	int idx,
	const void *buffer,
	size_t bufferSize
)
{
	LOGGER_DATA_TEMPERATURE_RAW *p = (LOGGER_DATA_TEMPERATURE_RAW *) ((char *) buffer
            + sizeof(LOGGER_MEASUREMENT_HDR) + sizeof(LOGGER_DATA_TEMPERATURE_RAW) * idx);
	if (retval)
		*retval = p;
	return TEMPERATURE_2_BYTES_2_double(p->value);
}

/**
 * @param value
 * @return
 */
double TEMPERATURE_2_BYTES_2_double(
	TEMPERATURE_2_BYTES value
)
{
	int16_t v = HTON2(value.t.t00625);
	double r = (v / 16);
	if (v >= 0)
		r += 0.0625 * (v & 0xf);
	else
		r -= 0.0625 * (((~v & 0xf) + 1) & 0xf);
	return r;
}

/**
 * @brief Return bus voltage in V as (94 - value) * 0.05814 + 2.6
 * @param value read value from the device
 * @return double voltage in volts
 */
double vcc2double(
        uint8_t value
) {
	return (94 - value) * 0.05814 + 2.6; 
}

/**
 * @brief Return battery voltage in V as (value * 4) * 1100.0 / 1023.0 * 6.1 / 1000.0 - 0.08;
 * @param value read value from the device
 * @return double voltage in volts
 */
double vbat2double(
        uint8_t value
) {
	return (value * 4) * 1100.0 / 1023.0 * 6.1 / 1000.0 - 0.08;
}

uint16_t LOGGER_MEASUREMENT_HDR_USED(uint16_t value) {
#if BYTE_ORDER == BIG_ENDIAN
    return value;
#else
    return NTOH2(value);
#endif
}

time_t LOGGER_MEASUREMENT_HDR2time_t(
    LOGGER_MEASUREMENT_HDR *header,
    int isLocaltime
)
{
    struct tm m;
    if (!header)
        return 0;
    m.tm_sec = header->seconds;			// 0-60 (1 leap second)
    m.tm_min = header->minutes;			// 0-59
    m.tm_hour = header->hours;			// 0-23
    m.tm_mday = header->day;			// 1-31
    m.tm_mon = header->month;			// 1-12
    m.tm_year = header->year + 200;	    // Year	- 1900
    m.tm_wday = 0;      				// Day of week.	0-6
    m.tm_yday = 0;		        	    // Days in year. 0-365
    m.tm_isdst = 0;			            // DST.		-1/0/1]

    time_t r = mktime(&m);

    if (!isLocaltime)
        r -= __timezone;
    return r;
}

time_t logger2time(
    uint8_t year2000,
    uint8_t month,
    uint8_t date,
    uint8_t hours,
    uint8_t minutes,
    uint8_t seconds,
    int isLocaltime
)
{
    struct tm m;
    m.tm_sec = seconds;			// 0-60 (1 leap second)
    m.tm_min = minutes;			// 0-59
    m.tm_hour = hours;			// 0-23
    m.tm_mday = date;			// 1-31
    m.tm_mon = month;			// 1-12
    m.tm_year = year2000 + 100;	// Year	- 1900
    m.tm_wday = 0;				// Day of week.	0-6
    m.tm_yday = 0;			    // Days in year. 0-365
    m.tm_isdst = 0;			    // DST.		-1/0/1]

    time_t r = mktime(&m);

    if (!isLocaltime)
        r -= __timezone;
    return r;
}

void LOGGER_DATA_TEMPERATURE_RAW_delta(
    int16_t *retval,
    LOGGER_DATA_TEMPERATURE_RAW *value1,
    LOGGER_DATA_TEMPERATURE_RAW *value0
)
{
    *retval = value1->value.t.t00625 - value0->value.t.t00625;
}

void LOGGER_MEASUREMENT_HDR_delta(
    LOGGER_MEASUREMENT_HDR_DIFF *retval,
    LOGGER_MEASUREMENT_HDR *h1,
    LOGGER_MEASUREMENT_HDR *h0
)
{
    // retval.used = h1->used - h0->used;						// 0 record number diff
    retval->used = h1->used;                                    // does not compress
    retval->delta_sec = LOGGER_MEASUREMENT_HDR2time_t(h1, 1) - LOGGER_MEASUREMENT_HDR2time_t(h0, 1);				        // 2 seconds
    retval->kosa = h1->kosa - h0->kosa;							// 3 номер косы в году
    retval->kosa_year = h1->kosa_year - h0->kosa_year;			// 4 год косы - 2000 (номер года последние 2 цифры)
    retval->rfu1 = h1->rfu1 - h0->rfu1;							// 5 reserved
    retval->rfu2 = h1->rfu2 - h0->rfu2;							// 6 reserved
    retval->vcc = h1->vcc - h0->vcc;						    // 7 V cc bus voltage, V
    retval->vbat = h1->vbat - h0->vbat;							// 8 V battery, V
    retval->pcnt = h1->pcnt - h0->pcnt;							// 9 pages count, Pcnt = ((ds1820_devices << 2) | pages_to_recods)
}

/**
 * Return diff value
 * @param buffer packet
 * @param bits 1 or 2
 * @param index zero based index
 * @return diff value
 */
int getDiff(
    const void *buffer,
    int bits,
    int index
)
{
    switch (bits) {
        case 2:
#if BYTE_ORDER == BIG_ENDIAN
            return htobe16(((int16_t*) buffer)[index]);
#else
            return ((int16_t*) buffer)[index];
#endif
        default:
            return ((int8_t*) buffer)[index];
    }
}
