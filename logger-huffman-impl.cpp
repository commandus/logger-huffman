#include <sstream>
#include <iomanip>
#include <cstdlib>

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
	ss << (int) value.memblockoccupation << " "
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

/**
 * Prints out:
 *  type 74 always
 * 	size (compressed) общая длина данных, bytes
 *  status.data_bits 
 *  status.data_bits 
 *  status.command_change
 *	measure мл. Байт номера замера, lsb used (или addr_used?)
 *	packets  количество пакетов в замере! (лора по 24 байта с шапками пакетов)
 *	kosa идентификатор косы (номер, дата)
 *	year год косы + 2000 Идентификатор прибора берется из паспорта косы при формате логгера, пишется из епром логгера, пишется в шапку замера.
 */
std::string LOGGER_PACKET_FIRST_HDR_2_string(
	const LOGGER_PACKET_FIRST_HDR &value
)
{
	std::stringstream ss;
	ss << (int) value.typ << " "
		<< (int) value.size << " "
		<< (int) value.status.b << " "
		<< (int) value.status.data_bits << " "
		<< (int) value.status.command_change << " "
		<< (int) value.measure << " "
		<< (int) value.packets << " "
		<< (int) value.kosa << "-" << (int) value.kosa_year;
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
	return "";
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
	char c[3] = { 0, 0, 0 };
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

LoggerItemId::LoggerItemId()
	: kosa(0), measure(0)
{
}

LoggerItemId::LoggerItemId(
	uint8_t akosa,							// идентификатор косы (номер, дата)
	uint8_t ameasure,						// мл. Байт номера замера, lsb used (или addr_used?)
	uint8_t apacket							// packet number
)
	: kosa(akosa), measure(ameasure), packet(apacket)
{

}

bool LoggerItemId::operator==(
	const LoggerItemId &another
)
{
	return (kosa == another.kosa) && (measure == another.measure) && (packet == another.packet);
}

bool LoggerItemId::operator!=(
	const LoggerItemId &another
)
{
	return !(*this == another);
}

/**
 * Set identifier
 * @param akosa kosa number
 * @param ameasure measurement no
 * @param apacket -1- first packet (w/o data)
 */ 
void LoggerItemId::set(
	uint8_t akosa,						// идентификатор косы (номер, дата)
	uint8_t ameasure,					// мл. Байт номера замера, lsb used (или addr_used?)
	int8_t apacket						// packet number
)
{
	kosa = akosa;
	measure = ameasure;
	packet = apacket;
}

// std::string errDescription;
LoggerItem::LoggerItem()
	: errCode(0), measurement(NULL)
{

}

LoggerItem::LoggerItem(
	const LoggerItem &value
)
	: packet(value.packet), errCode(value.errCode), measurement(NULL)
{
	
}

LoggerItem::LoggerItem(
	const void *abuffer,
	size_t asize
)
{
	size_t sz;
	LOGGER_PACKET_TYPE t = set(sz, abuffer, asize);
	if (t == LOGGER_PACKET_UNKNOWN)
		errCode = ERR_LOGGER_HUFFMAN_INVALID_PACKET;
	else
		errCode = 0;
}

LoggerItem& LoggerItem::operator=(
	const LoggerItem& other
)
{
	errCode = other.errCode;
	packet = other.packet;
}

bool LoggerItem::operator==(
	const LoggerItem &another
)
{
	return false;
}

bool LoggerItem::operator!=(
	const LoggerItem &another
)
{
	return !(*this == another);
}

LoggerItem::~LoggerItem()
{

}

LOGGER_PACKET_TYPE LoggerItem::set(
	size_t &retSize,
	const void *abuffer,
	size_t asize
)
{
	LOGGER_PACKET_TYPE t = extractLoggerPacketType(&retSize, abuffer, asize);	// 
	if (t == LOGGER_PACKET_UNKNOWN) {
		retSize = 0;
		return t;
	}

	packet = std::string((const char *) abuffer, retSize);

	LOGGER_MEASUREMENT_HDR *hdr;
	
	switch (t) {
		case LOGGER_PACKET_RAW:
			extractMeasurementHeader(&hdr, abuffer, asize);
			id.set(hdr->kosa, 0, -1);	// -1: first packet (with no data)
			retSize = 0;
			break;
		case LOGGER_PACKET_PKT_1:
			{
				LOGGER_PACKET_FIRST_HDR *h1;
				// LOGGER_MEASUREMENT_HDR *measurementHeader;
				extractFirstHdr(&h1, &measurement, abuffer, asize);
				id.set(h1->kosa, h1->measure, -1);	// -1: first packet (with no data)
			}
			break;
		case LOGGER_PACKET_PKT_2:
			{
				LOGGER_PACKET_SECOND_HDR *h2;
				extractSecondHdr(&h2, abuffer, asize);
				id.set(h2->kosa, h2->measure, h2->packet);
			}
			break;
		default: //case LOGGER_PACKET_UNKNOWN:
			retSize = 0;
			break;
	}
	return t;
}

// std::string errDescription;
LoggerCollection::LoggerCollection()
	: errCode(0)
{
}

LoggerCollection::~LoggerCollection()
{

}

LOGGER_PACKET_TYPE LoggerCollection::put(
	size_t &retSize,
	const void *buffer,
	size_t size
)
{
	LoggerItem item;
	
	LOGGER_PACKET_TYPE t = item.set(retSize, buffer, size);
	// check operation
	if (t == LOGGER_PACKET_UNKNOWN)
		return t;
	// if (item.errCode) return LOGGER_PACKET_UNKNOWN;
	
	// add atiem
	items.push_back(item);
	return t;
}

std::string LoggerCollection::toString() const
{
	std::stringstream ss;
	for (std::vector<LoggerItem>::const_iterator it(items.begin()); it != items.end(); it++) {
		ss << it->toString() << " ";
	}
	return ss.str();
}

std::string LoggerCollection::toJsonString() const
{
	std::stringstream ss;
	ss << "[";
	bool first = true;
	for (std::vector<LoggerItem>::const_iterator it(items.begin()); it != items.end(); it++) {
		if (first)
			first = false;
		else
			ss << ", ";
		ss << it->toJsonString() << " ";
	}
	ss << "]";
	return ss.str();
}

std::string LoggerItem::toString() const
{
	LOGGER_MEASUREMENT_HDR *hdr;
	LOGGER_PACKET_TYPE t = extractMeasurementHeader(&hdr, packet.c_str(), packet.size());
	std::string s;
	switch (t) {
		case LOGGER_PACKET_RAW:
			{
				std::stringstream ss;
				ss << LOGGER_MEASUREMENT_HDR_2_string(*hdr) << std::endl;
				for (int i = 0; i < 0; i++) {
					uint16_t t = extractMeasurementHeaderData(i, packet.c_str(), packet.size());
					ss << (int) t << " ";
				}
				s = ss.str();
			}
			break;
		case LOGGER_PACKET_PKT_1:
			{
				LOGGER_PACKET_FIRST_HDR *h1;
				LOGGER_MEASUREMENT_HDR *measurement;
				int r = extractFirstHdr(&h1, &measurement, packet.c_str(), packet.size());
				if (r)
					break;
				std::stringstream ss;
				ss << LOGGER_PACKET_FIRST_HDR_2_string(*h1) << std::endl
					<< LOGGER_MEASUREMENT_HDR_2_string(*measurement) << std::endl;
				s = ss.str();
			}
			break;
		case LOGGER_PACKET_PKT_2:
			{
				std::stringstream ss;
				LOGGER_PACKET_SECOND_HDR *h2;
				bool first = true;
				for (int p = 0; p < 5; p++) {
					LOGGER_DATA_TEMPERATURE_RAW *v = extractSecondHdrData(p, packet.c_str(), packet.size());
					if (!v)
						break;
					if (first)
						first = false;
					else
						ss << ", ";
					ss
						<< (int) v->sensor << ": "
						<< v->t;
				}
				s = ss.str();
			}
		default:
			break;
	}
	return s;
}

std::string LoggerItem::toJsonString() const
{
	return toString();
}
