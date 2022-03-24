#ifndef LOGGER_HUFFMAN_H_
#define LOGGER_HUFFMAN_H_ 1

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <inttypes.h>

#include "platform.h"
#include "util-compress.h"

#define MAX_SENSOR_COUNT			28
#define MAX_HDR_DATA_SIZE			24	// bytes

typedef enum {
	LOGGER_PACKET_RAW = 0,			// raw w/o packet headers. замер, разбитый по пакетам в 24 байта (в hex 48 байт). Используется для передачи 0 замера
	LOGGER_PACKET_UNKNOWN = 1,		// reserved for unknown
 	LOGGER_PACKET_PKT_1 = 0x4a,		// with packet header (first). К данным замера добавляются шапки пакетов, для первого 8 байт, для следующих 4 байта/.Используется для передачи 0 замера
 	LOGGER_PACKET_PKT_2 = 0x4b,		// with packet header (next)
 	LOGGER_PACKET_DELTA_1 = 0x48,	// дельты замеров от 0 замера.
	LOGGER_PACKET_DELTA_2 = 0x49,	// дельты замеров от 0 замера.
 	LOGGER_PACKET_HUFF_1 = 0x4c,	// дельты замеров от 0 сжаты каноническим Хафманом по таблице +-4.
	LOGGER_PACKET_HUFF_2 = 0x4d	// дельты замеров от 0 сжаты каноническим Хафманом по таблице +-4.
} LOGGER_PACKET_TYPE;

typedef ALIGN struct {
	uint8_t memblockoccupation;				// 0- memory block occupied
	uint8_t seconds;						// 0..59
	uint8_t minutes;						// 0..59
	uint8_t hours;							// 0..23
	uint8_t day;							// 1..31
	uint8_t month;							// 1..12
	uint8_t year;							// 0..99 year - 2000 = last 2 digits
	uint8_t kosa;							// номер косы в году
	uint8_t kosa_year;						// год косы - 2000 (номер года последние 2 цифры)
	uint8_t rfu1;							// reserved
	uint8_t rfu2;							// reserved
	uint8_t vcc;							// V cc bus voltage, V
	uint8_t vbat;							// V battery, V
	uint8_t pcnt;							// pages count, Pcnt = ((ds1820_devices << 2) | pages_to_recods)
	uint16_t used;							// record number, 1..65535
} PACKED LOGGER_MEASUREMENT_HDR;			// 16 bytes

// первый пакет typ 4a- plain 48- delta 4c- huffman
/**
 * typ:
 * 		LOGGER_PACKET_RAW		0x00 формируется, но не отправляется, 
 *		LOGGER_PACKET_PKT_1		0х4A без сжатия, 0x4B- последующие пакеты
 *		LOGGER_PACKET_DELTA_1	0х48 просто сжатие дельта, 49 последующие пакет
 *		LOGGER_PACKET_HUFF_1	0x4c хафман, 4D последующие пакеты
 */
typedef ALIGN struct {
	uint8_t typ;						    // 	LOGGER_PACKET_RAW, LOGGER_PACKET_PKT_1, LOGGER_PACKET_DELTA_1, LOGGER_PACKET_HUFF_1
	union {
		uint8_t data_bits: 3;
		uint8_t rfu: 4;
		uint8_t command_change: 1;
		uint8_t b;							// статус замера, биты 0-3 битовая длина тела данных замера, бит 7 – получена команда на смену 0 замера.
	} status;
	uint16_t size;							// (compressed) общая длина данных, bytes
	uint8_t measure;						// мл. Байт номера замера, lsb used (или addr_used?)
	uint8_t packets;						// количество пакетов в замере! (лора по 24 байта с шапками пакетов)
	uint8_t kosa;							// идентификатор косы (номер, дата)
	uint8_t kosa_year;						// год косы + 2000 Идентификатор прибора берется из паспорта косы при формате логгера, пишется из епром логгера, пишется в шапку замера.
} PACKED LOGGER_PACKET_FIRST_HDR;			// 8 bytes

// следующий пакет typ 4b- plain 49- delta 4d- huffman
typedef ALIGN struct {
	uint8_t typ;						    // 49 просто сжатие дельта, 0х4b 4d хафман
	uint8_t kosa;							// идентификатор косы (номер, дата)
	uint8_t measure;						// мл. Байт номера замера, lsb used (или addr_used?)
	uint8_t packet;							// номер пакета в замере
} PACKED LOGGER_PACKET_SECOND_HDR;			// 4 bytes

typedef ALIGN struct {
	uint8_t lo;						    	// Temperature * 0.625, lo byte
	uint8_t hi;								// Temperature * 0.625, hi byte
} PACKED TEMPERATURE_12_BITS;				// 2 bytes

typedef ALIGN struct {
	union {
		int16_t t00625;						// temperature * 0.0625, C. 12 bits, 
		TEMPERATURE_12_BITS f;
	} t;
} PACKED TEMPERATURE_2_BYTES;				// 2 bytes

typedef ALIGN struct {
	uint8_t sensor;						    // номер датчика 0..255
	TEMPERATURE_2_BYTES value;
	uint8_t rfu1;							// angle,. not used
} PACKED LOGGER_DATA_TEMPERATURE_RAW;		// 4 bytes

typedef ALIGN struct {
	union {
		int16_t t;							// temperature, C. 12 bits
		TEMPERATURE_12_BITS f;
	} value;
} PACKED LOGGER_DATA_TEMPERATURE;			// 2 bytes

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
);

/**
 * Extract header only
 * @param retHdr return header pointer
 * @param buffer data
 * @param size buffer size
 */
LOGGER_PACKET_TYPE extractMeasurementHeader(
	LOGGER_MEASUREMENT_HDR **retHdr,
	const void *buffer,
	size_t bufferSize
);

/**
 * Return expected packet size in bytes
 * @param typ packet type
 * @param bufferSize size
 */
size_t getLoggerPacketTypeSize(
	LOGGER_PACKET_TYPE typ,
	size_t bufferSize
);

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
);

int16_t extractSecondHdr(
	LOGGER_PACKET_SECOND_HDR **retHdr,
	const void *buffer,
	size_t bufferSize
);

double extractMeasurementHeaderData(
	LOGGER_DATA_TEMPERATURE_RAW **retval,
	int idx,
	const void *buffer,
	size_t bufferSize
);

LOGGER_DATA_TEMPERATURE_RAW *extractSecondHdrData(
	int p,
	const void *buffer,
	size_t bufferSize
);

double TEMPERATURE_2_BYTES_2_double(
	TEMPERATURE_2_BYTES value
);

double vcc2double(
		uint8_t value
);

#ifdef __cplusplus
}
#endif

#endif
