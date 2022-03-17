#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <algorithm>

#include "util-time-fmt.h"
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
	time_t t = logger2time(
		value.year,
		value.month - 1,
		value.day,
		value.hours,
		value.minutes,
		value.seconds,
		true
	);

	ss << "measurement header:" << std::endl
		<< "mem\t" << (int) value.memblockoccupation << std::endl
		<< "time\t" << t << std::endl
		<< "local\t" << time2string(t, true) << std::endl
		<< "gmt\t" << time2string(t, false) << std::endl
		<< "kosa\t" << (int) value.kosa << std::endl
		<< "year\t" << (int) value.kosa_year + 2000 << std::endl
		<< "vcc\t" << vcc_2_double(value.vcc) << std::endl
		<< "vbat\t" << vcc_2_double(value.vbat) << std::endl
		<< "pcnt\t" << (int) value.pcnt << std::endl
		<< "used\t" << (int) value.used << std::endl;
	return ss.str();
}

std::string LOGGER_MEASUREMENT_HDR_2_json(
	const LOGGER_MEASUREMENT_HDR &value
) {
	std::stringstream ss;
	bool isLocaltime = true;

	time_t t = logger2time(
		value.year,
		value.month - 1,
		value.day,
		value.hours,
		value.minutes,
		value.seconds,
		isLocaltime
	);

	ss 
		<< "{"
		<< "\"memblockoccupation\": " << (int) value.memblockoccupation
		<< std::setfill('0') << std::setw(2)
		<< ", \"time\": " << t
		<< ", \"localtime\": \"" << time2string(t, true)
		<< "\", \"gmt\": \"" << time2string(t, false) << "\""
		<< std::setw(0) << " "
		<< ", \"kosa\": " << (int) value.kosa
		<< ", \"kosa_year\": " << (int) value.kosa_year
		<< ", \"vcc\": " << vcc_2_double(value.vcc)
		<< ", \"vbat\": " << vcc_2_double(value.vbat)
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
	ss
		<< "header:" << std::endl
		<< "type\t" << (int) value.typ << std::endl
		<< "size\t" << (int) value.size << std::endl
		<< "status\t" << (int) value.status.b << std::endl
		<< "bits\t" <<  (int) value.status.data_bits << std::endl
		<< "cmd ch\t" << (int) value.status.command_change << std::endl
		<< "measure\t" << (int) value.measure << std::endl
		<< "packets\t" << (int) value.packets << std::endl
		<< "kosa\t" << (int) value.kosa  << std::endl
		<< "year\t" << (int) value.kosa_year + 2000 << std::endl;
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
#ifdef PRINT_DEBUG
			<< std::hex << std::setw(2) << std::setfill('0')
			<< ", \"hi\": \"" << (int) value->value.t.f.hi
			<< "\", \"lo\": \"" << (int) value->value.t.f.lo
			<< std::dec << std::setw(0)
			<< "\", \"rfu1\": " << (int) value->rfu1
#endif
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

std::string bin2hexString(
    const std::string &value
)
{
    std::stringstream r;
    bufferPrintHex(r, value.c_str(), value.size());
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

std::string LoggerItemId::toString() const
{
	std::stringstream ss;
	ss 
		<< "kosa\t" << (int) kosa << std::endl								// идентификатор косы (номер, дата)
		<< "measure\t" << (int) measure << std::endl						// мл. Байт номера замера, lsb used (или addr_used?)
		<< "packet\t" << (int) packet << std::endl							// packet number
		<< "year\t" << (int) 2000 + kosa_year << std::endl;					// reserved for first packet
	return ss.str();	
}

std::string LoggerItemId::toJsonString() const
{
	std::stringstream ss;
	ss 
		<< "{\"kosa\": "
		<< (int) kosa								// идентификатор косы (номер, дата)
		<< ", \"measure\": " << (int) measure		// мл. Байт номера замера, lsb used (или addr_used?)
		<< ", \"packet\": " << (int) packet			// packet number
		<< ", \"kosa_year\": " << (int) kosa_year	// reserved for first packet
		<< "}";
	return ss.str();
}

void LoggerItemId::set(
        const LOGGER_MEASUREMENT_HDR &value
) {
    kosa = value.kosa;
    measure = 0;
    packet = 0;
    kosa_year = value.kosa_year;
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
	: id(value.id), packet(value.packet), errCode(value.errCode), measurement(NULL), received(value.received)
{

}

LoggerItem::LoggerItem(
	const void *aBuffer,
	size_t aSize
)
{
	size_t sz;
	uint8_t packets;
	LOGGER_PACKET_TYPE t = set(packets, sz, aBuffer, aSize);
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
				int cnt = (packet.size() - sizeof(LOGGER_MEASUREMENT_HDR)) / sizeof(LOGGER_DATA_TEMPERATURE_RAW);
				for (int i = 0; i < cnt; i++) {
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
					ss
						<< (int) v->sensor
						<< std::fixed << std::setprecision(2)
						<< "\t" << TEMPERATURE_2_BYTES_2_double(v->value)
						<< std::endl;
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
				int cnt = (packet.size() - sizeof(LOGGER_MEASUREMENT_HDR)) / sizeof(LOGGER_DATA_TEMPERATURE_RAW);
				for (int i = 0; i < cnt; i++) {
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

std::string LoggerItem::toTableString() const
{
	return "Not implemented";
}

bool LoggerItem::get(std::map<uint8_t, double> &retval) const
{
	LOGGER_MEASUREMENT_HDR *hdr;
	LOGGER_PACKET_TYPE t = extractMeasurementHeader(&hdr, packet.c_str(), packet.size());
	switch (t) {
		case LOGGER_PACKET_RAW:
			{
				int cnt = (packet.size() - sizeof(LOGGER_MEASUREMENT_HDR)) / sizeof(LOGGER_DATA_TEMPERATURE_RAW);
				for (int i = 0; i < cnt; i++) {
                    LOGGER_DATA_TEMPERATURE_RAW *p = (LOGGER_DATA_TEMPERATURE_RAW *) ((char *) packet.c_str()
                        + sizeof(LOGGER_MEASUREMENT_HDR) + sizeof(LOGGER_DATA_TEMPERATURE_RAW) * i);
                    retval[p->sensor] = TEMPERATURE_2_BYTES_2_double(p->value);
				}
			}
			break;
		case LOGGER_PACKET_PKT_1:
			break;
		case LOGGER_PACKET_PKT_2:
			{
				for (int p = 0; p < 5; p++) {
					LOGGER_DATA_TEMPERATURE_RAW *v = extractSecondHdrData(p, packet.c_str(), packet.size());
					if (!v)
						break;
					retval[v->sensor] = TEMPERATURE_2_BYTES_2_double(v->value);
				}
			}
		default:
			break;
	}
	return true;
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
		retSize = asize; // go to the end
		return t;
	}
    packet = std::string((const char *) abuffer, retSize);
    LOGGER_MEASUREMENT_HDR *hdr;
	
	switch (t) {
		case LOGGER_PACKET_RAW:
			extractMeasurementHeader(&hdr, abuffer, asize);
			id.set(hdr->kosa, 0, -1, hdr->kosa_year);	// -1: first packet (with no data)
			break;
		case LOGGER_PACKET_PKT_1:
			{
				LOGGER_PACKET_FIRST_HDR *h1;
                extractFirstHdr(&h1, &measurement, abuffer, asize);
                id.set(h1->kosa, h1->measure, -1, h1->kosa_year);	// -1: first packet (with no data)

				// LOGGER_MEASUREMENT_HDR *measurementHeader;
                // extractMeasurementHeader(&measurementHeader, abuffer, asize);
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
			retSize = asize;
			break;
	}
	return t;
}

LoggerMeasurementHeader::LoggerMeasurementHeader()
    : start(0), vcc(0), vbat(0)
{

}

LoggerMeasurementHeader::LoggerMeasurementHeader(
    const LoggerMeasurementHeader &value
)
    : id(value.id), start(value.start), vcc(value.vcc), vbat(value.vbat)
{

}

LoggerMeasurementHeader::LoggerMeasurementHeader(
    const LOGGER_MEASUREMENT_HDR *pheader,
    size_t sz
)
{
    setHdr(pheader, sz);
}

LoggerMeasurementHeader &LoggerMeasurementHeader::operator=(
    const LOGGER_MEASUREMENT_HDR& value
)
{
    this->setHdr(&value, sizeof(LOGGER_MEASUREMENT_HDR));
}

bool LoggerMeasurementHeader::operator==(const LoggerItemId &another) const
{
    return
        (id.kosa == another.kosa)
        && (id.measure == another.measure);
}

bool LoggerMeasurementHeader::operator==(const LoggerMeasurementHeader &another) const
{
    return
        (id.kosa == another.id.kosa)
        && (id.measure == another.id.measure);
}

bool LoggerMeasurementHeader::operator!=(const LoggerItemId &another) const
{
    return !(*this == another);
}

bool LoggerMeasurementHeader::operator!=(const LoggerMeasurementHeader &another) const
{
    return !(*this == another);
}

bool LoggerMeasurementHeader::setHdr(
    const LOGGER_MEASUREMENT_HDR *pHeader,
    size_t sz
) {
    if (sz < sizeof(LOGGER_MEASUREMENT_HDR))
        return false;
    if (!pHeader)
        return false;
    id.set(*pHeader);
    start = logger2time(pHeader->year, pHeader->month, pHeader->day,
                        pHeader->hours, pHeader->minutes, pHeader->seconds, true);
    vcc = pHeader->vcc;
    vbat = pHeader->vbat;
}

void LoggerMeasurementHeader::assign(
    LOGGER_MEASUREMENT_HDR &retval
) const
{
    retval.memblockoccupation = 0;			// 0- memory block occupied
    struct tm *ti = localtime (&start);
    retval.seconds = ti->tm_sec;			// 0..59
    retval.minutes = ti->tm_min;			// 0..59
    retval.hours = ti->tm_hour;				// 0..23
    retval.day = ti->tm_mday;				// 1..31
    retval.month = ti->tm_mon + 1;			// 1..12
    retval.year =  ti->tm_year - 100;		// 0..99 year - 2000 = last 2 digits
    retval.kosa = id.kosa;					// номер косы в году
    retval.kosa_year = id.kosa_year;		// год косы - 2000 (номер года последние 2 цифры)
    retval.rfu1 = 0;						// reserved
    retval.rfu2 = 0;						// reserved
    retval.vcc = vcc;						// V cc bus voltage, V
    retval.vbat = vbat;						// V battery, V
    retval.pcnt = 0;						// pages count, Pcnt = ((ds1820_devices << 2) | pages_to_recods)
    retval.used = 0;						// record number, 1..65535
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
	if (t != LOGGER_PACKET_UNKNOWN) {
		// add item
		push(item);
	}
	return t;
}

void LoggerCollection::putRaw(
	size_t &retSize,
	const void *buffer,
	size_t size
)
{
	retSize = size;
	if (items.empty())
		return;
	LoggerItem &item = items.front();
	item.packet = item.packet + std::string((const char *) buffer, size);
}

LOGGER_PACKET_TYPE LoggerCollection::put(
    std::vector<LoggerMeasurementHeader> *retHeaders,
	const std::vector<std::string> values
)
{
	LOGGER_PACKET_TYPE t = LOGGER_PACKET_UNKNOWN;
	for (std::vector<std::string>::const_iterator it(values.begin()); it != values.end(); it++) {
		size_t sz;
		void *next = (void *) it->c_str();	
		size_t size = it->size();

		while (true) {
			if (t == LOGGER_PACKET_RAW) {
				putRaw(sz, next, size);
			} else {
                t = put(sz, next, size);
                if (retHeaders) {
                    switch (t) {
                        case LOGGER_PACKET_RAW:
                            {
                                LoggerMeasurementHeader mh((LOGGER_MEASUREMENT_HDR *) next, sz);
                                retHeaders->push_back(mh);
                            }
                            break;
                        case LOGGER_PACKET_PKT_1:
                            {
                                LOGGER_MEASUREMENT_HDR *measurementHeader;
                                extractMeasurementHeader(&measurementHeader, next, sz);
                                LoggerMeasurementHeader mh(measurementHeader, sizeof(LOGGER_MEASUREMENT_HDR));
                                retHeaders->push_back(mh);
                            }
                            break;
                        }
                }
            }
			if (sz >= size)
				break;
			size -= sz;
			next = (char *) next + sz;	
		}
	}
	return t;
}

bool LoggerCollection::completed() const
{
	items.size() == expectedPackets;
}

bool LoggerCollection::get(std::map<uint8_t, double> &rerval) const
{
	// if (!completed()) return false;
	for (std::vector<LoggerItem>::const_iterator it(items.begin()); it != items.end(); it++) {
		it->get(rerval);
	}
	return true;
}

std::string LoggerCollection::toString() const
{
	std::stringstream ss;
	for (std::vector<LoggerItem>::const_iterator it(items.begin()); it != items.end(); it++) {
		ss << it->toString() << std::endl;
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

std::string LoggerCollection::toTableString(
	const LoggerItemId &id,
	const time_t &t
) const
{
	std::stringstream ss;
	std::map<uint8_t, double> r;
	ss
		<< (int) id.kosa << "\t" 
		<< (int) id.kosa_year + 2000 << "\t" 
		<< (int) id.measure << "\t";
	if (get(r)) {
		// by default order by key operator<
		for (std::map<uint8_t, double>::const_iterator it(r.begin()); it != r.end(); it++) 
		{
			ss
				<< (int) it->first << "\t" 
				<< it->second << "\t";
		}
	}
	return ss.str();
}

LoggerKosaPackets::LoggerKosaPackets()
	: start(0)
{
    clear_LOGGER_MEASUREMENT_HDR(header);
}

LoggerKosaPackets::LoggerKosaPackets(
	const LoggerKosaPackets &value
)
	: id(value.id), start(value.start), header(value.header)
{
	std::copy(value.packets.items.begin(), value.packets.items.end(), std::back_inserter(packets.items));
}

LoggerKosaPackets::LoggerKosaPackets(
	const LoggerItem &value
)
{
	start = time(NULL);
	id = value.id;
    clear_LOGGER_MEASUREMENT_HDR(header);
    packets.items.push_back(value);
}

LoggerKosaPackets::~LoggerKosaPackets()
{

}

bool LoggerKosaPackets::expired() const 
{
	return (time(NULL) - start) > MAX_SECONDS_WAIT_KOSA_PACKETS;
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

std::string LoggerKosaPackets::toString() const
{
	std::stringstream ss;
	ss <<  std::endl
		<< id.toString() << std::endl
		<< "start\t" << start << std::endl
		<< "expired\t" << (expired() ? "true" : "false") << std::endl
		<< "completed\t" << (packets.completed() ? "true" : "false") << std::endl
		<< std::endl << packets.toString();
	return ss.str();
}

std::string LoggerKosaPackets::toJsonString() const
{
	std::stringstream ss;
	ss << "{\"id\": " <<  id.toJsonString()
		<< ", \"start\": " << start
		<< ", \"expired\": " << (expired() ? "true" : "false")
		<< ", \"completed\": " << (packets.completed() ? "true" : "false")
        << ", \"measurement_header\": " << LOGGER_MEASUREMENT_HDR_2_json(header)
		<< ", \"packets\": " << packets.toJsonString()
		<< "}";
	return ss.str();
}

std::string LoggerKosaPackets::toTableString() const
{
	std::stringstream ss;
	ss << packets.toTableString(id, start);
	return ss.str();
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
	// temporary raw collection
	LoggerCollection c;
    std::vector<LoggerMeasurementHeader> mhs;
	LOGGER_PACKET_TYPE r = c.put(&mhs, values);
	// copy items from raw collection group by logger
	for (std::vector<LoggerItem>::const_iterator it(c.items.begin()); it != c.items.end(); it++) {
		add(*it);
	}

    // copy header(s)
    for (std::vector<LoggerMeasurementHeader>::const_iterator it(mhs.begin()); it != mhs.end(); it++) {
        addHeader(*it);
    }
    return r;
}

std::string LoggerKosaCollection::toString() const
{
	std::stringstream ss;
	bool first = true;
	for (std::vector<LoggerKosaPackets>::const_iterator it(koses.begin()); it != koses.end(); it++) {
		if (first)
			first = false;
		else
			ss << ", ";
		ss << it->toString();
	}
	return ss.str();
}

std::string LoggerKosaCollection::toJsonString() const
{
	std::stringstream ss;
	bool first = true;
	ss << "[";
	for (std::vector<LoggerKosaPackets>::const_iterator it(koses.begin()); it != koses.end(); it++) {
		if (first)
			first = false;
		else
			ss << ", ";
		ss << it->toJsonString();
	}
	ss << "]";
	return ss.str();
}

std::string LoggerKosaCollection::toTableString() const
{
	std::stringstream ss;
	for (std::vector<LoggerKosaPackets>::const_iterator it(koses.begin()); it != koses.end(); it++) {
		ss << it->toTableString() << std::endl;
	}
	return ss.str();
}

bool LoggerKosaCollection::addHeader(
    const LoggerMeasurementHeader &value
)
{
    std::vector<LoggerKosaPackets>::iterator it(std::find(koses.begin(), koses.end(), value.id));
    if (it == koses.end())
        return false;
    value.assign(it->header);
    return true;
}

void clear_LOGGER_MEASUREMENT_HDR(
    LOGGER_MEASUREMENT_HDR &value
)
{
    std::fill((char *) &value, ((char *) &value) + sizeof(LOGGER_MEASUREMENT_HDR), 0);
}
