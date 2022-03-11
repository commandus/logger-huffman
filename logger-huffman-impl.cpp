#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <algorithm>

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

std::string LOGGER_MEASUREMENT_HDR_2_json(
	const LOGGER_MEASUREMENT_HDR &value
) {
	std::stringstream ss;
	ss 
		<< "{"
		<< "\"memblockoccupation\": " << (int) value.memblockoccupation
		<< std::setfill('0') << std::setw(2)
		<< ", \"time\": \"" << (int) value.year + 2000 << (int) value.month << (int) value.day
		<< "T"
		<< (int) value.hours << (int) value.minutes << (int) value.seconds
		<< std::setw(0) << " "
		<< "\", \"kosa\": " << (int) value.kosa
		<< ", \"kosa_year\": " << (int) value.kosa_year
		<< ", \"vcc\": " << (int) value.vcc
		<< ", \"vbat\": " << (int) value.vbat
		<< ", \"pcnt\": " << (int) value.pcnt
		<< ", \"used\": " << (int) value.used
		<< "}";
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

std::string LOGGER_PACKET_FIRST_HDR_2_json(
	const LOGGER_PACKET_FIRST_HDR &value
)
{
	std::stringstream ss;
	ss << "{\"type\": " << (int) value.typ
		<< ", \"size\": " << (int) value.size
		<<  ", \"status\": " << (int) value.status.b
		<<  ", \"data_bits\": " << (int) value.status.data_bits
		<<  ", \"command_change\": " << (int) value.status.command_change
		<<  ", \"measure\": " << (int) value.measure
		<<  ", \"packets\": " << (int) value.packets
		<<  ", \"kosa\": " << (int) value.kosa
		<<  ", \"kosa_year\": " << (int) value.kosa_year
		<< "}";
	return ss.str();
}

std::string LOGGER_PACKET_SECOND_HDR_2_json(
	const LOGGER_PACKET_SECOND_HDR &value
)
{
	std::stringstream ss;
	ss << "{\"type\": " << (int) value.typ
		<<  ", \"kosa\": " << (int) value.kosa
		<<  ", \"measure\": " << (int) value.measure
		<<  ", \"packet\": " << (int) value.packet
		<< "}";
	return ss.str();
}

std::string LOGGER_DATA_TEMPERATURE_RAW_2_json(
	const LOGGER_DATA_TEMPERATURE_RAW *value
) {
	std::stringstream ss;
	if (value)
		ss << "{\"sensor\": " << (int) value->sensor
			<< std::fixed << std::setprecision(2)
			<< ", \"t\": " << TEMPERATURE_2_BYTES_2_double(value->value)
			<< std::hex << std::setw(2) << std::setfill('0')
			<< ", \"hi\": \"" << (int) value->value.t.f.hi
			<< "\", \"lo\": \"" << (int) value->value.t.f.lo
			<< std::dec << std::setw(0)
			<< "\", \"rfu1\": " << (int) value->rfu1
			<< "}";
	return ss.str();
}

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
	uint8_t apacket,						// packet number
	uint8_t akosa_year						// reserved for first packe
)
	: kosa(akosa), measure(ameasure), packet(apacket), kosa_year(akosa_year)
{

}

LoggerItemId& LoggerItemId::operator=(
	const LoggerItemId& other
)
{
	kosa = other.kosa;							// идентификатор косы (номер, дата)
	measure = other.measure;					// мл. Байт номера замера, lsb used (или addr_used?)
	packet = other.packet;						// packet number
	kosa_year = other.kosa_year;				// reserved for first packet
}

bool LoggerItemId::operator==(
	const LoggerItemId &another
) const
{
	return (kosa == another.kosa) && (measure == another.measure) && (packet == another.packet);
}

bool LoggerItemId::operator!=(
	const LoggerItemId &another
) const
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
	int8_t apacket,						// packet number
	uint8_t akosa_year
)
{
	kosa = akosa;
	measure = ameasure;
	packet = apacket;
	kosa_year = akosa_year;
}

LoggerItem::LoggerItem()
	: errCode(0), measurement(NULL)
{
	time(&received);
}

LoggerItem::LoggerItem(time_t value)
	: errCode(0), measurement(NULL), received(value)
{
}

LoggerItem::LoggerItem(
	const LoggerItem &value
)
	: packet(value.packet), errCode(value.errCode), measurement(NULL), received(value.received)
{
}

LoggerItem::LoggerItem(
	const void *abuffer,
	size_t asize
)
{
	size_t sz;
	uint8_t packets;
	LOGGER_PACKET_TYPE t = set(packets, sz, abuffer, asize);
	if (t == LOGGER_PACKET_UNKNOWN)
		errCode = ERR_LOGGER_HUFFMAN_INVALID_PACKET;
	else
		errCode = 0;
}

LoggerItem& LoggerItem::operator=(
	const LoggerItem& other
)
{
	id = other.id;
	errCode = other.errCode;
	packet = other.packet;
	received = other.received;
}

bool LoggerItem::operator==(
	const LoggerItem &another
) const
{
	return 
		(id.kosa == another.id.kosa)
		&& (id.measure == another.id.measure);
}

bool LoggerItem::operator==(
	const LoggerItemId &aid
) const
{
	return 
		(id.kosa == aid.kosa)
		&& (id.measure == aid.measure);
}

bool LoggerItem::operator!=(
	const LoggerItem &another
) const
{
	return !(*this == another);
}

bool LoggerItem::operator!=(
	const LoggerItemId &aid
) const
{
	return !(*this == aid);
}

LoggerItem::~LoggerItem()
{

}

LOGGER_PACKET_TYPE LoggerItem::set(
	uint8_t &retPackets,
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
			id.set(hdr->kosa, 0, -1, hdr->kosa_year);	// -1: first packet (with no data)
			retSize = 0;
			break;
		case LOGGER_PACKET_PKT_1:
			{
				LOGGER_PACKET_FIRST_HDR *h1;
				// LOGGER_MEASUREMENT_HDR *measurementHeader;
				extractFirstHdr(&h1, &measurement, abuffer, asize);
				id.set(h1->kosa, h1->measure, -1, h1->kosa_year);	// -1: first packet (with no data)
				retPackets = h1->packets;
			}
			break;
		case LOGGER_PACKET_PKT_2:
			{
				LOGGER_PACKET_SECOND_HDR *h2;
				extractSecondHdr(&h2, abuffer, asize);
				id.set(h2->kosa, h2->measure, h2->packet, 0);
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
	: errCode(0), expectedPackets(0)
{
}

LoggerCollection::LoggerCollection(
	const LoggerCollection &value
)
	: expectedPackets(value.expectedPackets), errCode(value.errCode)

{
	std::copy(value.items.begin(), value.items.end(), std::back_inserter(items));
}

LoggerCollection::~LoggerCollection()
{

}

void LoggerCollection::push(
	const LoggerItem &value
)
{
	items.push_back(value);
}

LOGGER_PACKET_TYPE LoggerCollection::put(
	size_t &retSize,
	const void *buffer,
	size_t size
)
{
	LoggerItem item;
	LOGGER_PACKET_TYPE t = item.set(expectedPackets, retSize, buffer, size);
	// check operation
	if (t == LOGGER_PACKET_UNKNOWN)
		return t;
	// add item
	push(item);
	return t;
}

LOGGER_PACKET_TYPE LoggerCollection::put(
	const std::vector<std::string> values
)
{
	LOGGER_PACKET_TYPE t;
	for (std::vector<std::string>::const_iterator it(values.begin()); it != values.end(); it++) {
		size_t sz;
		void *next = (void *) it->c_str();	
		size_t size = it->size();

		while (true) {
			t = put(sz, next, size);
			if (sz >= size)
				break;
			size -= sz;
			next = (char *) next + sz;	
		}
	}
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
		ss << "{\"item\": " << it->toJsonString() << "}";
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
					uint16_t t = extractMeasurementHeaderData(NULL, i, packet.c_str(), packet.size());
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
						<< std::fixed << std::setprecision(2)
						<< ", \"t\": " << TEMPERATURE_2_BYTES_2_double(v->value);
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
	LOGGER_MEASUREMENT_HDR *hdr;
	LOGGER_PACKET_TYPE t = extractMeasurementHeader(&hdr, packet.c_str(), packet.size());
	std::stringstream ss;
	ss << "{";
	bool first = true;
	switch (t) {
		case LOGGER_PACKET_RAW:
			{
				ss << "\"measurement_header\": " <<  LOGGER_MEASUREMENT_HDR_2_json(*hdr)
					<< ", \"measurements\": [";
				for (int i = 0; i < 0; i++) {
					LOGGER_DATA_TEMPERATURE_RAW *tp;
					uint16_t t = extractMeasurementHeaderData(&tp, i, packet.c_str(), packet.size());
					if (first)
						first = false;
					else
						ss << ", ";
					ss << LOGGER_DATA_TEMPERATURE_RAW_2_json(tp);
				}
				ss << "]";
			}
			break;
		case LOGGER_PACKET_PKT_1:
			{
				LOGGER_PACKET_FIRST_HDR *h1;
				LOGGER_MEASUREMENT_HDR *measurement;
				int r = extractFirstHdr(&h1, &measurement, packet.c_str(), packet.size());
				if (r)
					break;
				ss << "\"first_packet\": " << LOGGER_PACKET_FIRST_HDR_2_json(*h1) << ", "
					<< "\"measurement_header\": " << LOGGER_MEASUREMENT_HDR_2_json(*measurement) << std::endl;
			}
			break;
		case LOGGER_PACKET_PKT_2:
			{
				LOGGER_PACKET_SECOND_HDR *h2;
				int r = extractSecondHdr(&h2, packet.c_str(), packet.size());
				if (r == 0) {
					ss << "\"second_packet\": " << LOGGER_PACKET_SECOND_HDR_2_json(*h2) << ", \"measurements\": [";
					for (int p = 0; p < 5; p++) {
						LOGGER_DATA_TEMPERATURE_RAW *v = extractSecondHdrData(p, packet.c_str(), packet.size());
						if (!v)
							break;
						if (first)
							first = false;
						else
							ss << ", ";
						ss << LOGGER_DATA_TEMPERATURE_RAW_2_json(v);
					}
				}
				ss << "]";
			}
		default:
			break;
	}
	ss << "}";
	return ss.str();
}

LoggerKosaPackets::LoggerKosaPackets()
	: start(0)
{

}

LoggerKosaPackets::LoggerKosaPackets(
	const LoggerKosaPackets &value
)
	: id(value.id), start(value.start)
{
	std::copy(value.packets.items.begin(), value.packets.items.end(), std::back_inserter(packets.items));
}

LoggerKosaPackets::LoggerKosaPackets(
	const LoggerItem &value
)
{
	start = time(NULL);
	id = value.id;
	packets.items.push_back(value);
}

LoggerKosaPackets::~LoggerKosaPackets()
{

}

bool LoggerKosaPackets::expired()
{
	return (time(NULL) - start) > MAX_SECONDS_WAIT_KOSA_PACKETS;
}

bool LoggerKosaPackets::completed()
{
	packets.items.size() == packets.expectedPackets;
}

bool LoggerKosaPackets::add(
	const LoggerItem &value
)
{
	bool newOne = packets.items.empty();
	if (newOne || value == id) {
		if (newOne) {
			start = time(NULL);
			id = value.id;
		}
		packets.items.push_back(value);
		return true;
	}
	return false;
}

bool LoggerKosaPackets::operator==(
	const LoggerItemId &another
) const
{
	return (id.kosa == another.kosa) && (id.measure == another.measure);	
}

bool LoggerKosaPackets::operator!=(
	const LoggerItemId &another
) const
{
	return !(*this == another);	
}

bool LoggerKosaPackets::operator==(
	uint8_t akosa
) const
{
	return id.kosa == akosa;
}

bool LoggerKosaPackets::operator!=(
	uint8_t akosa
) const
{
	return id.kosa != akosa;
}

LoggerKosaCollection::LoggerKosaCollection()
{

}

LoggerKosaCollection::~LoggerKosaCollection()
{

}

/**
 * @return removed items count
 */
int LoggerKosaCollection::rmExpired()
{
	int r = 0;
	for (std::vector<LoggerKosaPackets>::iterator it(koses.begin()); it != koses.end(); ) {
		if (it->expired()) {
			it = koses.erase(it);
			r++;
		} else {
			it++;
		}
	}
	return r;
}

/**
 * @return does any packets exists before
 */
bool LoggerKosaCollection::add(
	const LoggerItem &value
)
{
	bool found = false;
	for (std::vector<LoggerKosaPackets>::iterator it(koses.begin()); it != koses.end(); it++) {
		if (*it == value.id) {
			it->add(value);
			found = true;
			break;
		}
	}
	if (!found) {
		LoggerKosaPackets p;
		p.add(value);
		koses.push_back(p);
	}
	return found;
}

/**
 * Put char buffer
 */
LOGGER_PACKET_TYPE LoggerKosaCollection::put(
	size_t &retSize, const void *buffer, size_t size
)
{
	LoggerCollection c;
	LOGGER_PACKET_TYPE r = c.put(retSize, buffer, size);
	for (std::vector<LoggerItem>::const_iterator it(c.items.begin()); it != c.items.end(); it++) {
		add(*it);
	}
	return r;
}

/**
 * Put collection of strings
 */
LOGGER_PACKET_TYPE LoggerKosaCollection::put(
	const std::vector<std::string> values
)
{
	LoggerCollection c;
	LOGGER_PACKET_TYPE r = c.put(values);
	for (std::vector<LoggerItem>::const_iterator it(c.items.begin()); it != c.items.end(); it++) {
		add(*it);
	}
	return r;
}

std::string LoggerKosaCollection::toString() const
{
	std::stringstream ss;
	bool first = false;
	for (std::vector<LoggerKosaPackets>::const_iterator it(koses.begin()); it != koses.end(); it++) {
		if (first)
			first = false;
		else
			ss << ", ";
		ss << it->packets.toString();
	}
	return ss.str();
}

std::string LoggerKosaCollection::toJsonString() const
{
	std::stringstream ss;
	bool first = false;
	ss << "[";
	for (std::vector<LoggerKosaPackets>::const_iterator it(koses.begin()); it != koses.end(); it++) {
		if (first)
			first = false;
		else
			ss << ", ";
		ss << it->packets.toJsonString();
	}
	ss << "]";
	return ss.str();
}
