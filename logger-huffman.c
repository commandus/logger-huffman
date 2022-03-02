#include <string.h>

#include "logger-huffman.h"
#include "errlist.h"

/**
 * Return LOGGER_PACKET_UNKNOWN if buffer is NULL or size = 0
 */ 
LOGGER_PACKET_TYPE extractLoggerPacketType(
	const void *buffer,
	size_t bufferSize
)
{
	if ((!buffer) || (bufferSize == 0))
		return LOGGER_PACKET_UNKNOWN;
	return (LOGGER_PACKET_TYPE)	*(char *) buffer;
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
	LOGGER_PACKET_TYPE t = extractLoggerPacketType(buffer, bufferSize);
	switch (t) {
		case LOGGER_PACKET_RAW:			// raw w/o packet headers. замер, разбитый по пакетам в 24 байта (в hex 48 байт). Используется для передачи 0 замера
			if (bufferSize < sizeof(LOGGER_MEASUREMENT_HDR))
				return ERR_LOGGER_HUFFMAN_INVALID_PACKET;
			*retHdr = (LOGGER_MEASUREMENT_HDR*) buffer;
			break;
 		case LOGGER_PACKET_PKT_1:		// with packet header (first). К данным замера добавляются шапки пакетов, для первого 8 байт, для следующих 4 байта/.Используется для передачи 0 замера
			break;
 		case LOGGER_PACKET_PKT_2:		// with packet header (next)
			break;
 		case LOGGER_PACKET_DELTA_1:		// дельты замеров от 0 замера.
			break;
		case LOGGER_PACKET_DELTA_2:		// дельты замеров от 0 замера.
			break;
 		case LOGGER_PACKET_HUFF_1:		// дельты замеров от 0 сжаты каноническим Хафманом по таблице +-4.
			break;
		case LOGGER_PACKET_HUFF_2:		// дельты замеров от 0 сжаты каноническим Хафманом по таблице +-4.
			break;
		default:
			// case LOGGER_PACKET_UNKNOWN:
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
	LOGGER_PACKET_TYPE t = extractLoggerPacketType(buffer, bufferSize);
	switch (t) {
 		case LOGGER_PACKET_PKT_1:		// with packet header (first). К данным замера добавляются шапки пакетов, для первого 8 байт, для следующих 4 байта/.Используется для передачи 0 замера
			if (bufferSize < sizeof(LOGGER_PACKET_FIRST_HDR))
				return ERR_LOGGER_HUFFMAN_INVALID_PACKET;
			*retHdr = (LOGGER_PACKET_FIRST_HDR*) buffer;
			return 0;
		default:
		 	return ERR_LOGGER_HUFFMAN_INVALID_PACKET;
	}
}

int extractFirstHdrData(
	uint8_t *sensor,
	int idx,
	const void *buffer,
	size_t bufferSize
)
{
	LOGGER_DATA_TEMPERATURE_RAW *p = (LOGGER_DATA_TEMPERATURE_RAW *) ((char *) buffer + sizeof(LOGGER_MEASUREMENT_HDR)) + idx;	
	if (sensor)
		*sensor = p->sensor;
	return NTOH2(p->t);
}

int extractSecondHdrData(
	uint8_t *sensor,
	int idx,
	int p,
	const void *buffer,
	size_t bufferSize
)
{
	char *pp = (char *) buffer + sizeof(LOGGER_PACKET_FIRST_HDR) + (idx * sizeof(LOGGER_PACKET_SECOND_HDR) * (4 * sizeof(LOGGER_DATA_TEMPERATURE_RAW)));
	LOGGER_DATA_TEMPERATURE_RAW *r = (LOGGER_DATA_TEMPERATURE_RAW *) pp + p;
	if (sensor)
		*sensor = r->sensor;
	return NTOH2(r->t);
}

int extractMeasurementHeaderData(
	int idx,
	const void *buffer,
	size_t bufferSize
)
{
	LOGGER_DATA_TEMPERATURE_RAW *p = (LOGGER_DATA_TEMPERATURE_RAW *) ((char *) buffer + sizeof(LOGGER_MEASUREMENT_HDR)) + idx;	
	return  NTOH2(p->t);
}


