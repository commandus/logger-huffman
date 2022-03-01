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
 		return LOGGER_PACKET_PKT_1;			// with packet header (first). К данным замера добавляются шапки пакетов, для первого 8 байт, для следующих 4 байта/.Используется для передачи 0 замера
	if (value == "pkt2")
 		return LOGGER_PACKET_PKT_2;			// with packet header (next)
	if (value == "delta1")
		return LOGGER_PACKET_DELTA_1;		// with packet header (first)
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
		<< (int) value.year + 2000 << (int) value.month << (int) value.day
		<< "T"
		<< (int) value.hours << ":" << (int) value.minutes << ":" << (int) value.seconds
		<< std::setw(0) << " "
		<< (int) value.kosa << "-" << (int) value.kosa_year
		<<  " "
		<< (int) value.vcc << " " << (int) value.vbat
		<<  " "
		<< (int) value.pcnt << " " << (int) value.used;
	return ss.str();
}

#define		ERR_LIST_COUNT		9

static const char *error_list_en[ERR_LIST_COUNT] = 
{
	"Segmentation fault",
	"Program aborted",
	"Invalid command line option",
	"Help screen",
	"Insufficient memory",
	"Insufficient parameters",
	"Invalid parameter",
	"No data",
	"Invalid packet or no packet"
};

const char *strerror_logger_huffman(
	int errCode
)
{
	int idx = (- errCode) - 700;
	if (idx < 0 || idx >= ERR_LIST_COUNT) {
		std::stringstream ss;
		ss << ERR_UNKNOWN_ERROR_CODE << errCode;
		return ss.str().c_str();
	}
		
	return error_list_en[idx];
}

/**
 * @see http://stackoverflow.com/questions/673240/how-do-i-print-an-unsigned-char-as-hex-in-c-using-ostream
 */
static void bufferPrintHex(std::ostream &ostream, const void* value, size_t size)
{
	if (value == NULL)
		return;
	unsigned char *p = (unsigned char*) value;
	for (size_t i = 0; i < size; i++)
	{
		ostream << std::setfill('0') << std::setw(2) << std::hex << (int) *p;
		p++;
	}
}

/**
 * @brief Return binary data string
 * @param hexChars hex string
 * @param size hex string size
 * @return binary data string
 */
std::string hex2binString(
	const char *hexChars,
	size_t size
)
{
	std::string r(size / 2, '\0');
	std::stringstream ss(hexChars);
	ss >> std::noskipws;
	char c[3] = {0, 0, 0};
	int i = 0;
	while (ss >> c[0]) {
		if (!(ss >> c[1]))
			break;
		unsigned char x = (unsigned char) strtol(c, NULL, 16);
		r[i] = x;
		i++;
	}
	return r;
}

std::string bin2hexString(
	const char *binChars,
	size_t size
)
{
	std::stringstream r;
	bufferPrintHex(r, binChars, size);
	return r.str();
}

// std::string errDescription;
LoggerPacket::LoggerPacket()
	: buffer(NULL), size(0), errCode(0)
{

}

LoggerPacket::LoggerPacket(
	const void *abuffer,
	size_t asize
)
{
	errCode = setBinary(abuffer, asize);
}

LoggerPacket::~LoggerPacket()
{

}

int LoggerPacket::setBinary(const void *abuffer, size_t asize)
{
	buffer = (const char *) abuffer;
	size = asize;
	LOGGER_MEASUREMENT_HDR *hdr;
	int r = exractMeasurementHeader(&hdr, buffer, size);
	return r;
}

std::string LoggerPacket::toString() const
{
	LOGGER_MEASUREMENT_HDR *hdr;
	int r = exractMeasurementHeader(&hdr, buffer, size);
	if (r)
		return "";
	std::string s = LOGGER_MEASUREMENT_HDR_2_string(*hdr);
	return s;
}

std::string LoggerPacket::toJsonString() const
{

}
