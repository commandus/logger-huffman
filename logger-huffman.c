#include <string.h>
#include <stdio.h>
#include <math.h>

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
	LOGGER_PACKET_TYPE t = (LOGGER_PACKET_TYPE)	*(char *) buffer;
	size_t sz = getLoggerPacketTypeSize(t, bufferSize);
	if (retSize)
		*retSize = sz;
	if (sz == 0)
		t = LOGGER_PACKET_UNKNOWN;
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
			r = sizeof(LOGGER_PACKET_FIRST_HDR) + sizeof(LOGGER_MEASUREMENT_HDR);	// 24
			break;
 		case LOGGER_PACKET_PKT_2:		// with packet header (next)
			r = sizeof(LOGGER_PACKET_SECOND_HDR) + 5 * sizeof(LOGGER_DATA_TEMPERATURE_RAW);	// 24
			break;
 		case LOGGER_PACKET_DELTA_1:		// дельты замеров от 0 замера.
		 	r = 24; // max
			break;
		case LOGGER_PACKET_DELTA_2:		// дельты замеров от 0 замера.
			r = 24; // max
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
			if (bufferSize < sizeof(LOGGER_PACKET_PKT_1) + sizeof(LOGGER_MEASUREMENT_HDR))
				return ERR_LOGGER_HUFFMAN_INVALID_PACKET;
		 	*retHdr = (LOGGER_MEASUREMENT_HDR*) (char *) buffer + sizeof(LOGGER_PACKET_PKT_1);
			break;
 		case LOGGER_PACKET_PKT_2:		// with packet header (next)
		 	*retHdr = NULL;
			break;
 		case LOGGER_PACKET_DELTA_1:		// дельты замеров от 0 замера.
		 	*retHdr = NULL;
			break;
		case LOGGER_PACKET_DELTA_2:		// дельты замеров от 0 замера.
			*retHdr = NULL;
			break;
 		case LOGGER_PACKET_HUFF_1:		// дельты замеров от 0 сжаты каноническим Хафманом по таблице +-4.
		 	*retHdr = NULL;
			break;
		case LOGGER_PACKET_HUFF_2:		// дельты замеров от 0 сжаты каноническим Хафманом по таблице +-4.
			*retHdr = NULL;
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
 * @param retMeasurement, return measurement header pointer
 * @param buffer data
 * @param size buffer size
 */
int extractFirstHdr(
	LOGGER_PACKET_FIRST_HDR **retHdr,
	LOGGER_MEASUREMENT_HDR **retMeasurement,
	const void *buffer,
	size_t bufferSize
)
{
	size_t sz;
	LOGGER_PACKET_TYPE t = extractLoggerPacketType(&sz, buffer, bufferSize);
	switch (t) {
 		case LOGGER_PACKET_PKT_1:		// with packet header (first). К данным замера добавляются шапки пакетов, для первого 8 байт, для следующих 4 байта/.Используется для передачи 0 замера
			if (bufferSize < sizeof(LOGGER_PACKET_FIRST_HDR) + + sizeof (LOGGER_MEASUREMENT_HDR))	// 24 bytes
				return ERR_LOGGER_HUFFMAN_INVALID_PACKET;
			*retHdr = (LOGGER_PACKET_FIRST_HDR*) buffer;
			*retMeasurement = (LOGGER_MEASUREMENT_HDR *) ((char *) buffer + sizeof (LOGGER_PACKET_FIRST_HDR));
			return 0;
		default:
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
			if (bufferSize < sizeof(LOGGER_PACKET_SECOND_HDR))
				return ERR_LOGGER_HUFFMAN_INVALID_PACKET;
			*retHdr = (LOGGER_PACKET_SECOND_HDR*) buffer;
			return 0;
		default:
		 	return ERR_LOGGER_HUFFMAN_INVALID_PACKET;
	}
}

LOGGER_DATA_TEMPERATURE_RAW *extractSecondHdrData(
	int p,
	const void *buffer,
	size_t bufferSize
)
{
	LOGGER_DATA_TEMPERATURE_RAW *r = (LOGGER_DATA_TEMPERATURE_RAW *) buffer + p + 1;
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

double TEMPERATURE_2_BYTES_2_double(
	TEMPERATURE_2_BYTES value
)
{
#if BYTE_ORDER == BIG_ENDIAN
	return HTON2(value.t.t00625) * 0.0625;
#else
	return (int16_t) value.t.t00625 * 0.0625;
#endif	
}

double temperature_2_double(
	uint16_t value
)
{
#if BYTE_ORDER == BIG_ENDIAN
	return HTON2(value) * 0.0625;
#else
	return (int16_t) value * 0.0625;
#endif	
}
