#include <sstream>
#include <iomanip>

#include "logger-huffman-impl.h"
#include "errlist.h"

std::string LOGGER_PACKET_TYPE_2_string(
	const LOGGER_PACKET_TYPE &value
)
{
	switch (value) {
		case LOGGER_PACKET_RAW:			// raw w/o packet headers. замер, разбитый по пакетам в 24 байта (в hex 48 байт). Используется для передачи 0 замера
			return "raw";
 		case LOGGER_PACKET_PKT_1:		// with packet header (first). К данным замера добавляются шапки пакетов, для первого 8 байт, для следующих 4 байта/.Используется для передачи 0 замера
			return "pkt1";
 		case LOGGER_PACKET_PKT_2:		// with packet header (next)
			return "pkt2";
 		case LOGGER_PACKET_DELTA_1:		// дельты замеров от 0 замера.
			return "delta1";
		case LOGGER_PACKET_DELTA_2:		// дельты замеров от 0 замера.
			return "delta2";
 		case LOGGER_PACKET_HUFF_1:		// дельты замеров от 0 сжаты каноническим Хафманом по таблице +-4.
			return "huff1";
		case LOGGER_PACKET_HUFF_2:		// дельты замеров от 0 сжаты каноническим Хафманом по таблице +-4.
			return "huff2";
		default:
			return "unknown";
	}
}

LOGGER_PACKET_TYPE LOGGER_PACKET_TYPE_2_string(
	const std::string &value
)
{
	if (value == "raw")
		return LOGGER_PACKET_RAW;
	if (value == "pkt1")
 		return LOGGER_PACKET_PKT_1;		// with packet header (first). К данным замера добавляются шапки пакетов, для первого 8 байт, для следующих 4 байта/.Используется для передачи 0 замера
	if (value == "pkt2")
 		return LOGGER_PACKET_PKT_2;		// with packet header (next)
	if (value == "delta1")
 		return LOGGER_PACKET_DELTA_1;		// with packet header (next)
	if (value == "delta2")
 		return LOGGER_PACKET_DELTA_2;		// with packet header (next)
	if (value == "huff1")
 		return LOGGER_PACKET_HUFF_1;		// дельты замеров от 0 сжаты каноническим Хафманом по таблице +-4.
	if (value == "huff2")
 		return LOGGER_PACKET_HUFF_2;		// дельты замеров от 0 сжаты каноническим Хафманом по таблице +-4.
	return LOGGER_PACKET_UNKNOWN;
}

std::string LOGGER_MEASUREMENT_HDR_2_string(
	const LOGGER_MEASUREMENT_HDR &value
) {

	std::stringstream ss;
	ss << (int) value.memblockoccupation << "\t"
		<< std::setfill('0') << std::setw(2)
		<< (int) value.day << "." << (int) value.month << "." << (int) value.year + 2000
		<< " "
		<< (int) value.hours << ":" << (int) value.minutes << ":" << (int) value.seconds
		<< std::setw(0) << " "
		<< (int) value.kosa << "-" << (int) value.kosa_year
		<<  " "
		<< (int) value.vcc << " " << (int) value.vbat
		<<  " "
		<< (int) value.pcnt << " " << (int) value.used;
	return ss.str();
}

#define		ERR_LIST_COUNT		8
static char * error_list_en[ERR_LIST_COUNT] = 
{
	"Segmentation fault",
	"Program aborted"
	"Invalid command line option "
	"Insufficient memory",
	"Insufficient parameters",
	"Invalid parameter",
	"No data"
	"Invalid packet"
};

const char *strerror_logger_huffman(int errCode)
{
	int idx = (- errCode) - 700;
	if (idx < 0 || idx >= ERR_LIST_COUNT)
		return "";
	return error_list_en[idx];
}
