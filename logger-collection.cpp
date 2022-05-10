#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <locale>

#include "util-time-fmt.h"
#include "logger-collection.h"
#include "errlist.h"
#ifdef ENABLE_LOGGER_PASSPORT
#include "logger-passport/logger-passport.h"
#endif

/**
 * Conditional defines:
 *  PRINT_DEBUG - print out temperature raw lo, hi bytes in hex, rfu1 value
 */


/**
 * To string
 * @param value
 * @return
 */
std::string LOGGER_PACKET_TYPE_2_string(
	const LOGGER_PACKET_TYPE &value
)
{
	switch (value) {
		case LOGGER_PACKET_RAW:			// 0x00 raw w/o packet headers. замер, разбитый по пакетам в 24 байта (в hex 48 байт). Используется для передачи 0 замера
			return "raw";
 		case LOGGER_PACKET_PKT_1:		// 0x4a with packet header (first). К данным замера добавляются шапки пакетов, для первого 8 байт, для следующих 4 байта/.Используется для передачи 0 замера
			return "pkt1";
 		case LOGGER_PACKET_PKT_2:		// 0x4b with packet header (next)
			return "pkt2";
 		case LOGGER_PACKET_DELTA_1:		// 0x48 deltas(first)
			return "delta1";
		case LOGGER_PACKET_DELTA_2:		// 0x49 deltas(next)
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
       << "vcc\t" << vcc2double(value.vcc) << std::endl
       << "vbat\t" << vbat2double(value.vbat) << std::endl
		<< "pcnt\t" << (int) value.pcnt << std::endl
		<< "used\t" << LOGGER_MEASUREMENT_HDR_USED(value.used) << std::endl;
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
            << ", \"vcc\": " << std::fixed << std::setprecision(2) << vcc2double(value.vcc)
            << ", \"vbat\": " << vbat2double(value.vbat)
		<< ", \"pcnt\": " << (int) value.pcnt
		<< ", \"used\": " << LOGGER_MEASUREMENT_HDR_USED(value.used)
		<< "}";
	return ss.str();
}

std::string vcc2string(
    uint8_t value
) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << vcc2double(value);
    return ss.str();
}

std::string vbat2string(
    uint8_t value
) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << vbat2double(value);
    return ss.str();
}

std::string LOGGER_MEASUREMENT_HDR_2_table(
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

    ss  << (int) value.memblockoccupation << "\t"
        << t << "\t"
        << time2string(t, true) << "\t"
        << time2string(t, false) << "\t"
        << (int) value.kosa << "\t"
        << (int) value.kosa_year << "\t"
        << vcc2double(value.vcc) << "\t"
        << vbat2double(value.vbat) << "\t"
        << (int) value.pcnt << "\t"
        << LOGGER_MEASUREMENT_HDR_USED(value.used) << "\t";
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
			<< std::fixed << std::setprecision(4)
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

std::string LOGGER_DATA_TEMPERATURE_RAW_2_text(
        const LOGGER_DATA_TEMPERATURE_RAW *value
)
{
    std::stringstream ss;
    if (value)
        ss << (int) value->sensor
           << std::fixed << std::setprecision(2)
           << "\t" << TEMPERATURE_2_BYTES_2_double(value->value)
           << "\t";
    return ss.str();
}

std::string LOGGER_MEASUREMENT_HDR_DIFF_2_json(
        const LOGGER_MEASUREMENT_HDR_DIFF *value
)
{
    std::stringstream ss;
    if (value)
        ss
            << "{\"used\": " << (int) value->used
            << ", \"delta_sec\": " << (int) value->delta_sec		        // 2 seconds
            << ", \"kosa\": " << (int) value->kosa							// 3 номер косы в году
            << ", \"year\": " << (int) value->kosa_year						// 4 год косы - 2000 (номер года последние 2 цифры)
            << ", \"rfu1\": " << (int) value->rfu1							// 5 reserved
            << ", \"rfu2\": " << (int) value->rfu2							// 6 reserved
            << ", \"vcc\": " << (int) value->vcc 							// 7 V cc bus voltage, V
            << ", \"vbat\": " << (int) value->vbat							// 8 V battery, V
            << ", \"pcnt\": " << (int) value->pcnt							// 9 pages count, Pcnt = ((ds1820_devices << 2) | pages_to_recods)
        << "}";
    return ss.str();
}

std::string LOGGER_MEASUREMENT_HDR_DIFF_2_string(
    const LOGGER_MEASUREMENT_HDR_DIFF *value
)
{
    std::stringstream ss;
    if (value)
        ss << (int) value->used
            << "\t" << (int) value->delta_sec				        // 2 seconds
            << "\t" << (int) value->kosa							// 3 номер косы в году
            << "\t" << (int) value->kosa_year						// 4 год косы - 2000 (номер года последние 2 цифры)
            << "\t" << (int) value->rfu1							// 5 reserved
            << "\t" << (int) value->rfu2							// 6 reserved
            << "\t" << (int) value->vcc 							// 7 V cc bus voltage, V
            << "\t" << (int) value->vbat							// 8 V battery, V
            << "\t" << (int) value->pcnt							// 9 pages count, Pcnt = ((ds1820_devices << 2) | pages_to_recods)
        ;
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

std::string hex2binString(
    const std::string &value
)
{
    return hex2binString(value.c_str(), value.size());
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
	uint8_t ameasure,						// мл. Байт номера замера, lsb (или addr_use?)
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
	measure = other.measure;					// мл. Байт номера замера, lsb use (или addr_use?)
	packet = other.packet;						// packet number
	kosa_year = other.kosa_year;				// reserved for first packet
	return *this;
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
	uint8_t ameasure,					// мл. Байт номера замера, lsb use (или addr_use?)
	int8_t apacket,						// packet number
	uint8_t akosa_year
)
{
	kosa = akosa;
	measure = ameasure;
	packet = apacket;
	kosa_year = akosa_year;
}

void LoggerItemId::assign(
	LOGGER_MEASUREMENT_HDR *retval
)
{
	retval->kosa = kosa;
	retval->kosa_year = kosa_year;
}

std::string LoggerItemId::toString() const
{
	std::stringstream ss;
	ss 
		<< "kosa\t" << (int) kosa << std::endl								// идентификатор косы (номер, дата)
		<< "measure\t" << (int) measure << std::endl						// мл. Байт номера замера, lsb use (или addr_use?)
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
		<< ", \"measure\": " << (int) measure		// мл. Байт номера замера, lsb use (или addr_use?)
		<< ", \"packet\": " << (int) packet			// packet number
		<< ", \"kosa_year\": " << (int) kosa_year	// reserved for first packet
		<< "}";
	return ss.str();
}

std::string LoggerItemId::kosaString() const
{
    std::stringstream ss;
    ss << (int) kosa;
    return ss.str();
}

std::string LoggerItemId::measureString() const
{
    std::stringstream ss;
    ss << (int) measure;
    return ss.str();
}

std::string LoggerItemId::packetStrng() const
{
    std::stringstream ss;
    ss << (int) packet;
    return ss.str();
}

std::string LoggerItemId::kosaYearString() const
{
    std::stringstream ss;
    ss << (int) kosa_year;
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
	time(&parsed);
}

LoggerItem::LoggerItem(time_t value)
	: errCode(0), measurement(NULL), parsed(value)
{
}

LoggerItem::LoggerItem(
	const LoggerItem &value
)
	: id(value.id), packet(value.packet), errCode(value.errCode), parsed(value.parsed)
{
	if (value.measurement)
		measurement = (LOGGER_MEASUREMENT_HDR *) (packet.c_str() + sizeof(LOGGER_PACKET_FIRST_HDR));
	else
		measurement = NULL;
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
    parsed = other.parsed;
	return *this;
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
                    LOGGER_DATA_TEMPERATURE_RAW *tp;
					uint16_t t = extractMeasurementHeaderData(&tp, i, packet.c_str(), packet.size());
                    ss << LOGGER_DATA_TEMPERATURE_RAW_2_text(tp);
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
            break;
        case LOGGER_PACKET_DELTA_1:		// 0x48 deltas(first)
            {
                LOGGER_MEASUREMENT_HDR_DIFF *headerMeasurement = extractDiffHdr(packet.c_str(), packet.size());
                if (!headerMeasurement)
                    break;
                std::stringstream ss;
                ss << LOGGER_MEASUREMENT_HDR_DIFF_2_string(headerMeasurement) << std::endl;
                s = ss.str();
            }
            break;
        case LOGGER_PACKET_DELTA_2:		// 0x48 deltas(next)
            break;
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
            break;
        case LOGGER_PACKET_DELTA_1:		// 0x48 deltas(first)
            {
                LOGGER_MEASUREMENT_HDR_DIFF *headerMeasurement = extractDiffHdr(packet.c_str(), packet.size());
                if (!headerMeasurement)
                    break;
                ss << LOGGER_MEASUREMENT_HDR_DIFF_2_json(headerMeasurement) << std::endl;
            }
            break;
        case LOGGER_PACKET_DELTA_2:		// 0x48 deltas(next)
            break;

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

/**
 * Get sensor value
 * @param retval
 * @return
 */
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

void LOGGER_MEASUREMENT_HDR_DIFF_2_LOGGER_MEASUREMENT_HDR(
    LOGGER_MEASUREMENT_HDR *retval,
    LOGGER_MEASUREMENT_HDR_DIFF *value
)
{
    if (retval && value) {
        retval->memblockoccupation = 0;				// 0 0- memory block occupied
        retval->seconds = 0;						// 1 0..59
        retval->minutes = 0;						// 2 0..59
        retval->hours = 0;							// 3 0..23
        retval->day = 0;							// 4 1..31
        retval->month = 0;							// 5 1..12
        retval->year = 0;							// 6 0..99 year - 2000 = last 2 digits
        retval->kosa = 0;							// 7 номер косы в году
        retval->kosa_year = 0;						// 8 год косы - 2000 (номер года последние 2 цифры)
        retval->rfu1 = 0;							// 9 reserved
        retval->rfu2 = 0;							// 10 reserved
        retval->vcc = 0;							// 11 V cc bus voltage, V
        retval->vbat = 0;							// 12 V battery, V
        retval->pcnt = 0;							// 13 pages count, Pcnt = ((ds1820_devices << 2) | pages_to_recods)
        retval->used = 0;							// 14 record number, 1..65535
    }
}

std::string LOGGER_MEASUREMENT_HDR_DIFF_2_LOGGER_MEASUREMENT_HDR_string(
    const void *aBuffer,
    size_t aSize
)
{
    std::string r;
    if (aSize < sizeof(LOGGER_MEASUREMENT_HDR_DIFF))
        return "";
    r.resize(aSize + sizeof(LOGGER_MEASUREMENT_HDR) - sizeof(LOGGER_MEASUREMENT_HDR_DIFF));
    LOGGER_MEASUREMENT_HDR h;
    memmove((void *) r.c_str(), &h, sizeof(LOGGER_MEASUREMENT_HDR));
    memmove((void *) (r.c_str() + sizeof(LOGGER_MEASUREMENT_HDR)),
            (char *)aBuffer + sizeof(LOGGER_MEASUREMENT_HDR_DIFF), aSize);
    return r;
}

LOGGER_PACKET_TYPE LoggerItem::set(
	uint8_t &retPackets,
	size_t &retSize,
	const void *aBuffer,
	size_t aSize
)
{
	LOGGER_PACKET_TYPE t = extractLoggerPacketType(&retSize, aBuffer, aSize);	//
	if (t == LOGGER_PACKET_UNKNOWN) {
		retSize = aSize; // go to the end
		return t;
	}
    switch (t) {
        case LOGGER_PACKET_DELTA_1:
            packet = LOGGER_MEASUREMENT_HDR_DIFF_2_LOGGER_MEASUREMENT_HDR_string(aBuffer, retSize);
            break;
        default:
            packet = std::string((const char *) aBuffer, retSize);
    }

    LOGGER_MEASUREMENT_HDR *hdr;
	
	switch (t) {
		case LOGGER_PACKET_RAW:
			extractMeasurementHeader(&hdr, packet.c_str(), aSize);
			id.set(hdr->kosa, 0, -1, hdr->kosa_year);	// -1: first packet (with no data)
			// retPackets unknown
			break;
		case LOGGER_PACKET_PKT_1:
			{
				LOGGER_PACKET_FIRST_HDR *h1;
                extractFirstHdr(&h1, &measurement, packet.c_str(), aSize);
                id.set(h1->kosa, h1->measure, -1, h1->kosa_year);	// -1: first packet (with no data)

				// LOGGER_MEASUREMENT_HDR *measurementHeader;
                // extractMeasurementHeader(&measurementHeader, packet.c_str(), aSize);
				retPackets = h1->packets;
			}
			break;
		case LOGGER_PACKET_PKT_2:
			{
				LOGGER_PACKET_SECOND_HDR *h2;
				extractSecondHdr(&h2, packet.c_str(), aSize);
				id.set(h2->kosa, h2->measure, h2->packet, 0);
			}
			break;
        case LOGGER_PACKET_DELTA_1:
            {
                extractMeasurementHeader(&measurement, packet.c_str(), aSize);
                id.set(measurement->kosa, measurement->used, 0, measurement->kosa_year);
                retPackets = 0;
            }
            break;
		default: //case LOGGER_PACKET_UNKNOWN:
			retSize = aSize;
			break;
	}
	return t;
}

LoggerMeasurementHeader::LoggerMeasurementHeader()
    : header(NULL), start(0), vcc(0), vbat(0)
{

}

LoggerMeasurementHeader::LoggerMeasurementHeader(
    const LoggerMeasurementHeader &value
)
    : header(value.header), id(value.id), start(value.start), vcc(value.vcc), vbat(value.vbat)
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
	return *this;
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
	header = pHeader;
    id.set(*pHeader);
    start = logger2time(pHeader->year, pHeader->month, pHeader->day,
                        pHeader->hours, pHeader->minutes, pHeader->seconds, true);
    vcc = pHeader->vcc;
    vbat = pHeader->vbat;
	return true;
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

LOGGER_PACKET_TYPE LoggerCollection::put1(
	size_t &retSize,
    std::vector<LoggerMeasurementHeader> *retHeaders,
	const void *buffer,
	size_t size
)
{
	LoggerItem item;
	LOGGER_PACKET_TYPE t = item.set(expectedPackets, retSize, buffer, size);

    if (retHeaders) {
        switch (t) {
            case LOGGER_PACKET_RAW:
                {
                    LoggerMeasurementHeader mh((LOGGER_MEASUREMENT_HDR *) buffer, size);
                    retHeaders->push_back(mh);
                }
                break;
            case LOGGER_PACKET_PKT_1:
                {
                    // memmove(&item.measurement->id, &item.id, sizeof();
                    item.id.assign(item.measurement);
                    LoggerMeasurementHeader mh(item.measurement, sizeof(LOGGER_MEASUREMENT_HDR));
                    retHeaders->push_back(mh);
                }
                break;
            case LOGGER_PACKET_DELTA_1:
                {
                    item.id.assign(item.measurement);
                    LoggerMeasurementHeader mh(item.measurement, sizeof(LOGGER_MEASUREMENT_HDR));
                    retHeaders->push_back(mh);
                }
                break;
        }
    }

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
	expectedPackets = items.size();
}

LOGGER_PACKET_TYPE LoggerCollection::put(
        size_t &retSize,
        std::vector<LoggerMeasurementHeader> *retHeaders,
        const void *buffer,
        size_t asize
)
{
    LOGGER_PACKET_TYPE t = LOGGER_PACKET_UNKNOWN;
    size_t sz;
    void *next = (void *) buffer;
    retSize = asize;

    while (true) {
        if (t == LOGGER_PACKET_RAW) {
            putRaw(sz, next, retSize);
        } else {
            t = put1(sz, retHeaders, next, retSize);
        }
        if (sz >= retSize)
            break;
        retSize -= sz;
        next = (char *) next + sz;
    }
    return t;
}

LOGGER_PACKET_TYPE LoggerCollection::put(
    std::vector<LoggerMeasurementHeader> *retHeaders,
	const std::vector<std::string> values
)
{
	LOGGER_PACKET_TYPE t = LOGGER_PACKET_UNKNOWN;
	for (std::vector<std::string>::const_iterator it(values.begin()); it != values.end(); it++) {
		size_t sz;
        t = put(sz, retHeaders, it->c_str(), it->size());
	}
	return t;
}

bool LoggerCollection::completed() const
{
	return (items.size() >= expectedPackets) && (expectedPackets > 0);
}

bool LoggerCollection::get(std::map<uint8_t, double> &retval) const
{
	// if (!completed()) return false;
	for (std::vector<LoggerItem>::const_iterator it(items.begin()); it != items.end(); it++) {
		it->get(retval);
	}
	return true;
}

time_t LoggerKosaPackets::measured() const
{
    time_t t = logger2time(
            header.year,
            header.month - 1,
            header.day,
            header.hours,
            header.minutes,
            header.seconds,
            true
    );
    return t;
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
    const time_t &t,
    const LOGGER_MEASUREMENT_HDR &header
) const
{
	std::stringstream ss;
	std::map<uint8_t, double> r;
	ss
		<< (int) id.kosa << "\t" 
		<< (int) id.kosa_year + 2000 << "\t" 
		<< (int) id.measure << "\t"
        << LOGGER_MEASUREMENT_HDR_2_table(header);

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
	: collection(nullptr), start(0)
{
    clear_LOGGER_MEASUREMENT_HDR(header);
}

LoggerKosaPackets::LoggerKosaPackets(
	LoggerKosaCollection *value
)
	: collection(value), start(0)
{

}

LoggerKosaPackets::LoggerKosaPackets(
	const LoggerKosaPackets &value
)
	: collection(value.collection), id(value.id), start(value.start), header(value.header)
{
	std::copy(value.packets.items.begin(), value.packets.items.end(), std::back_inserter(packets.items));
}

LoggerKosaPackets::LoggerKosaPackets(
	const LoggerItem &value
)
	: collection(nullptr)
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
	if (value.measurement)
		memmove(&header, value.measurement, sizeof(LOGGER_MEASUREMENT_HDR));
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
	ss << packets.toTableString(id, start, header);
	return ss.str();
}

void LoggerKosaPackets::temperatureCommaString(
    std::ostream &ostrm,
    const std::string &separator,
    const std::string &substEmptyValue
) const
{
    std::map<uint8_t, double> r;
    if (packets.get(r)) {
        bool isFirst = true;
        int c = 0;
        // map is sorted by the first
        for (std::map<uint8_t, double>::const_iterator it(r.begin()); it != r.end(); it++) {
            if (isFirst) {
                isFirst = false;
            } else
                ostrm << separator;
            // some sensors can be missed
            int skipped = it->first - c;
            for (int i = 0; i < skipped; i++) {
                ostrm << substEmptyValue << separator;
            }
            ostrm << it->second;
            c++;
        }
    }
}

void LoggerKosaPackets::temperaturePolyCommaString(
    std::ostream &ostrm,
    const std::string &separator,
    const std::string &substEmptyValue
) const
{
#ifdef ENABLE_LOGGER_PASSPORT
	if (!(collection && collection->passportDescriptor))
		return temperatureCommaString(ostrm, separator, substEmptyValue);

    std::map<uint8_t, double> r;
    if (packets.get(r)) {
        bool isFirst = true;
        int c = 0;
        // map is sorted by the first
        for (std::map<uint8_t, double>::const_iterator it(r.begin()); it != r.end(); it++) {
            if (isFirst) {
                isFirst = false;
            } else
                ostrm << separator;
            // some sensors can be missed
            int skipped = it->first - c;
            for (int i = 0; i < skipped; i++) {
                ostrm << substEmptyValue << separator;
            }
			// first one is bus controller, skip it
			if (it->first == 0)
				ostrm << it->second;
			else
            	ostrm << calcTemperature(collection->passportDescriptor, id.kosa, id.kosa_year, it->first - 1, it->second);
            c++;
        }
    }
#else
	return temperatureCommaString(ostrm, separator, substEmptyValue);
#endif
}

void LoggerKosaPackets::rawCommaString(
        std::ostream &ostrm,
        const std::string &separator
) const
{
    bool isFirst = true;
    // map is sorted by the first
    for (std::vector<LoggerItem>::const_iterator it(packets.items.begin()); it != packets.items.end(); it++) {
        if (isFirst) {
            isFirst = false;
        } else
            ostrm << separator;
        ostrm << bin2hexString(it->packet);
    }
}

/**
 * SQL fields: kosa, year, no, measured, parsed, vcc, vbat, t, tp, raw
 * @param retval
 */
void LoggerKosaPackets::toStrings(
        std::vector<std::string> &retval,
        const std::string &substEmptyValue
) const {
    retval.push_back(id.kosaString());
    retval.push_back(id.kosaYearString());
    retval.push_back(id.measureString());
    retval.push_back(time2unixepochstring(measured()));
    retval.push_back(time2unixepochstring(start));
    retval.push_back(vcc2string(header.vcc));
    retval.push_back(vbat2string(header.vbat));

    std::stringstream ss;
    temperatureCommaString(ss, ",", substEmptyValue);
    retval.push_back(ss.str());

	std::stringstream ssp;
    temperaturePolyCommaString(ssp, ",", substEmptyValue);
    retval.push_back(ssp.str());

    std::stringstream ssr;
    rawCommaString(ssr, " ");
    retval.push_back(ssr.str());
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
void LoggerKosaCollection::add(
    LoggerCollection &value
)
{
    for (std::vector<LoggerItem>::const_iterator itc(value.items.begin()); itc != value.items.end(); itc++) {
        bool found = false;
        for (std::vector<LoggerKosaPackets>::iterator it(koses.begin()); it != koses.end(); it++) {
            if (*it == itc->id) {
                it->add(*itc);
                if (value.expectedPackets) {
                    it->packets.expectedPackets = value.expectedPackets;
                }
                found = true;
                break;
            }
        }
        if (!found) {
            LoggerKosaPackets p(this);
            p.add(*itc);
            koses.push_back(p);
            if (value.expectedPackets) {
                koses.begin()->packets.expectedPackets = value.expectedPackets;
            }
        }
    }
}

/**
 * Put char buffer
 */
LOGGER_PACKET_TYPE LoggerKosaCollection::put(
    size_t &retSize, const void *buffer, size_t size
)
{
	LoggerCollection c;
    std::vector<LoggerMeasurementHeader> mhs;
	LOGGER_PACKET_TYPE r = c.put(retSize, &mhs, buffer, size);
    // copy items from raw collection group by logger
	add(c);

    // copy header(s)
    for (std::vector<LoggerMeasurementHeader>::const_iterator it(mhs.begin()); it != mhs.end(); it++) {
        addHeader(*it);
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

    add(c);

    // copy header(s)
    for (std::vector<LoggerMeasurementHeader>::const_iterator it(mhs.begin()); it != mhs.end(); it++) {
        addHeader(*it);
    }
    return r;
}

/**
 * Put one string
 */
LOGGER_PACKET_TYPE LoggerKosaCollection::put(
    const std::string &value
)
{
    size_t sz;
    return put(sz, value.c_str(), value.size());
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

void LoggerKosaCollection::setPassports(
	void *value
)
{
	passportDescriptor = value;
}

void clear_LOGGER_MEASUREMENT_HDR(
    LOGGER_MEASUREMENT_HDR &value
)
{
    std::fill((char *) &value, ((char *) &value) + sizeof(LOGGER_MEASUREMENT_HDR), 0);
}
