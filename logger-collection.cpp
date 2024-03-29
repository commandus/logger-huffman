#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <set>

#include "platform.h"
#include "util-time-fmt.h"
#include "logger-collection.h"
#include "util-compress.h"
#include "errlist.h"

#define MAX_BUFFER_SIZE 256
#define NO_MATCH_YEAR   255

#ifdef ENABLE_LOGGER_PASSPORT
// uncomment if you want passport in output
// #define PRINT_PASSPORT_TEXT  1
// #define PRINT_PASSPORT_JSON  1
#include "logger-passport/logger-passport.h"
#endif

const std::string JS_NULL = "null";
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

int LoggerItem::getHuffmanPacketMeasurementOffset(
    int packetNum1
) const {
    packetNum1--;   // it is offset, skip current
    if (!collection || (packetNum1 < 1))
        return 0;
    int r = 0;
    std::set<uint8_t> packetsCalculated;
    for (int i = 0; i < collection->items.size(); i++) {
        std::string s;
        LOGGER_MEASUREMENT_HDR *hdr;
        LOGGER_PACKET_TYPE t = extractMeasurementHeader(&hdr, collection->items[i].packet.c_str(), collection->items[i].packet.size());
        switch (t) {
            case LOGGER_PACKET_HUFF_1:
                if (packetsCalculated.find(1) != packetsCalculated.end())
                    continue;
                packetsCalculated.insert(1);
                s = decompressLoggerString(collection->items[i].packet.substr(sizeof(LOGGER_PACKET_FIRST_HDR)));
                r += (int) (s.size() - sizeof(LOGGER_MEASUREMENT_HDR_DIFF));
                break;
            case LOGGER_PACKET_HUFF_2:
            {
                LOGGER_PACKET_SECOND_HDR *h;
                int16_t notOk = extractSecondHdr(&h, collection->items[i].packet.c_str(), collection->items[i].packet.size());
                if (notOk)
                    break;
                if (h->packet > packetNum1)
                    break;
                if (packetsCalculated.find(h->packet) != packetsCalculated.end())
                    continue;
                packetsCalculated.insert(h->packet);
                s = decompressLoggerString(collection->items[i].packet.substr(sizeof(LOGGER_PACKET_SECOND_HDR)));
                r += (int) s.size();
            }
                break;
            default:
                break;    
        }
    }
    return r;
}

double LoggerItem::correctTemperatureByPassport(
    uint8_t sensor,
    double value
) const
{
#ifdef ENABLE_LOGGER_PASSPORT
    if (collection && collection->collector && collection->collector->passportDescriptor)
        return calcTemperature(collection->collector->passportDescriptor, id.kosa, id.kosa_year, sensor, value);
    return value;
#else
    return value;
#endif
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
        << "vcc\t" << std::setprecision(3) <<  vcc2double(value.vcc) << std::endl
        << "vbat\t" << std::setprecision(3) << vbat2double(value.vbat) << std::endl
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
 *	kosa_year год косы + 2000 Идентификатор прибора берется из паспорта косы при формате логгера, пишется из епром логгера, пишется в шапку замера.
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
            << "status\t" << (int) *(uint8_t *)&value.status << std::endl
            << "data_bits\t" << (int) value.status.data_bits << std::endl
        << "command_change\t" << (int) value.status.command_change << std::endl
        << "rfu\t" << (int) value.status.rfu << std::endl
		<< "measure\t" << (int) value.measure << std::endl
		<< "packets\t" << (int) value.packets << std::endl
		<< "kosa\t" << (int) value.kosa  << std::endl
		<< "kosa_year\t" << (int) value.kosa_year + 2000 << std::endl;
    return ss.str();
}

std::string LOGGER_PACKET_FIRST_HDR_2_json(
	const LOGGER_PACKET_FIRST_HDR &value
)
{
	std::stringstream ss;
	ss << "{\"type\": " << (int) value.typ
		<< ", \"size\": " << (int) value.size
		<<  ", \"status\": " << (int) *(uint8_t *)&value.status
		<<  ", \"data_bits\": " << (int) value.status.data_bits
		<<  ", \"command_change\": " << (int) value.status.command_change
        <<  ", \"rfu\": " << (int) value.status.rfu
		<<  ", \"measure\": " << (int) value.measure
		<<  ", \"packets\": " << (int) value.packets
		<<  ", \"kosa\": " << (int) value.kosa
		<<  ", \"kosa_year\": " << (int) value.kosa_year
		<< "}";
	return ss.str();
}

std::string LOGGER_PACKET_SECOND_HDR_2_string(
	const LOGGER_PACKET_SECOND_HDR &value
)
{
	std::stringstream ss;
	ss << "type:\t" << (int) value.typ << " "
		<<  "kosa:\t" << (int) value.kosa << " "
		<<  "measure:\t" << (int) value.measure << " "
		<<  "packet:\t " << (int) value.packet;
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
           << std::fixed << std::setprecision(4)
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

/**
 * Error description, locale: en
 */
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

static std::string lastUnknownErrorDescription;

const char *strerror_logger_huffman(
	int errCode
)
{
	int idx = (- errCode) - 700;
	if (idx < 0 || idx >= ERR_LIST_COUNT) {
		std::stringstream ss;
		ss << ERR_UNKNOWN_ERROR_CODE << errCode;
        lastUnknownErrorDescription = ss.str();
		return lastUnknownErrorDescription.c_str();
	}
	return error_list_en[idx];
}

/**
 * @see http://stackoverflow.com/questions/673240/how-do-i-print-an-unsigned-char-as-hex-in-c-using-ostream
 */
static void bufferPrintHex(std::ostream &ostream, const void* value, size_t size)
{
	if (value == nullptr)
		return;
	unsigned char *p = (unsigned char*) value;
	for (size_t i = 0; i < size; i++)
	{
		ostream << std::setfill('0') << std::setw(2) << std::hex << (int) *p;
		p++;
	}
}

int bytesRequiredForBits(
    int bits
)
{
    if (bits > 8)
        return 2;
    return 1;
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
		unsigned char x = (unsigned char) strtol(c, nullptr, 16);
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
	uint8_t akosa_year						// reserved for first packet
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
 * @param akosa plume number
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
		<< 'N' << (int) kosa_year << '-' << (int) kosa << " #" << (int) measure;
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

std::string LoggerItemId::packetString() const
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

void LoggerItemId::clear() {
    kosa = 0;
    measure = 0;
    packet = 0;
    kosa_year = 0;
}

LoggerItem::LoggerItem()
	: errCode(0), collection(nullptr), addr(0)
{
	time(&parsed);
}

LoggerItem::LoggerItem(LoggerCollection *aCollection)
    : errCode(0), collection(aCollection), addr(0)
{
    time(&parsed);
}

LoggerItem::LoggerItem(time_t value)
	: errCode(0), parsed(value), collection(nullptr), addr(0)
{
}

LOGGER_MEASUREMENT_HDR *LoggerItem::getMeasurementHeaderIfExists() const
{
    LOGGER_MEASUREMENT_HDR *r;
    extractMeasurementHeader(&r, packet.c_str(), packet.size());
    return r;
}

LoggerItem::LoggerItem(
	const LoggerItem &value
)
	: id(value.id), packet(value.packet), errCode(value.errCode), parsed(value.parsed), collection(value.collection),
    addr(value.addr)
{
}

LoggerItem::LoggerItem(
    uint32_t aAddr,
    const void *aBuffer,
    size_t aSize
)
    : collection(nullptr), addr(aAddr)
{
	size_t sz;
	uint8_t packets;
	LOGGER_PACKET_TYPE t = set(packets, sz, 0, aBuffer, aSize);
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
    addr = other.addr;
	return *this;
}

bool LoggerItem::operator==(
	const LoggerItem &another
) const
{
	return 
		(id.kosa == another.id.kosa)
        && (addr == another.addr)   // diff packet does not contain plume production year, use address instead
		&& (id.measure == another.id.measure);
}

bool LoggerItem::operator==(
	const LoggerItemId &aid
) const
{
	return 
		(id.kosa == aid.kosa)
        && (id.kosa_year == aid.kosa_year)
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
				int cnt = (int) ((packet.size() - sizeof(LOGGER_MEASUREMENT_HDR)) / sizeof(LOGGER_DATA_TEMPERATURE_RAW));
				for (int i = 0; i < cnt; i++) {
                    LOGGER_DATA_TEMPERATURE_RAW *tp;
					double r = extractMeasurementHeaderData(&tp, i, packet.c_str(), packet.size());
                    ss << LOGGER_DATA_TEMPERATURE_RAW_2_text(tp);
				}
				s = ss.str();
			}
			break;
		case LOGGER_PACKET_PKT_1:
			{
				LOGGER_PACKET_FIRST_HDR *h1;
				int r = extractFirstHdr(&h1, packet.c_str(), packet.size());
				if (r)
					break;
                const LOGGER_MEASUREMENT_HDR *measurementHeader = getMeasurementHeaderIfExists();
                std::stringstream ss;
				ss << LOGGER_PACKET_FIRST_HDR_2_string(*h1) << std::endl;
                if (measurementHeader)
                   ss << LOGGER_MEASUREMENT_HDR_2_string(*measurementHeader) << std::endl;
				s = ss.str();
			}
			break;
		case LOGGER_PACKET_PKT_2:
			{
				std::stringstream ss;
				bool first = true;
				for (int p = 0; p < 5; p++) {
					LOGGER_DATA_TEMPERATURE_RAW *v = extractSecondHdrData(p, packet.c_str(), packet.size());
					if (!v)
						break;
					ss
						<< (int) v->sensor
						<< std::fixed << std::setprecision(4)
						<< "\t" << TEMPERATURE_2_BYTES_2_double(v->value)
						<< std::endl;
				}
				s = ss.str();
			}
            break;
        case LOGGER_PACKET_DELTA_1:		// 0x48 deltas(first)
            s = delta1ToString(packet);
            break;
        case LOGGER_PACKET_DELTA_2:		// 0x49 deltas(next)
            s = delta2ToString(packet);
            break;
        case LOGGER_PACKET_HUFF_1:		// 0x4с huffman encoded deltas(next)
            {
                std::string p = decompressLoggerString(packet);
                s = delta1ToString(p);
            }
            break;
        case LOGGER_PACKET_HUFF_2:		// 0x4d huffman encoded deltas(next)
            {
                std::string p = decompressLoggerString(packet);
                s = delta2ToString(p);
            }
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
				int cnt = (int) ((packet.size() - sizeof(LOGGER_MEASUREMENT_HDR)) / sizeof(LOGGER_DATA_TEMPERATURE_RAW));
				for (int i = 0; i < cnt; i++) {
					LOGGER_DATA_TEMPERATURE_RAW *tp;
					double r = extractMeasurementHeaderData(&tp, i, packet.c_str(), packet.size());
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
				int r = extractFirstHdr(&h1, packet.c_str(), packet.size());
				if (r)
					break;
                const LOGGER_MEASUREMENT_HDR *measurementHeader = getMeasurementHeaderIfExists();
				ss << "\"first_header\": " << LOGGER_PACKET_FIRST_HDR_2_json(*h1) << ", "
                   << "\"measurement_header\": ";
                if (measurementHeader)
                    ss << LOGGER_MEASUREMENT_HDR_2_json(*measurementHeader) << std::endl;
			}
			break;
		case LOGGER_PACKET_PKT_2:
			{
				LOGGER_PACKET_SECOND_HDR *h2;
				int r = extractSecondHdr(&h2, packet.c_str(), packet.size());
				if (r == 0) {
					ss << "\"second_header\": " << LOGGER_PACKET_SECOND_HDR_2_json(*h2) << ", \"measurements\": [";
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
            ss << delta1ToJson(packet);
            break;
        case LOGGER_PACKET_DELTA_2:		// 0x48 deltas(next)
            ss << delta2ToJson(packet);
            break;
        case LOGGER_PACKET_HUFF_1:		// 0x4с huffman encoded deltas(next)
            {
                std::string s = packet.substr(0, sizeof(LOGGER_PACKET_FIRST_HDR))
                    + decompressLoggerString(packet.substr(sizeof(LOGGER_PACKET_FIRST_HDR)));
                ss << delta1ToJson(s);
            }
            break;
        case LOGGER_PACKET_HUFF_2:		// 0x4с huffman encoded deltas(next)
            {
                std::string s = packet.substr(0, sizeof(LOGGER_PACKET_SECOND_HDR))
                    + decompressLoggerString(packet.substr(sizeof(LOGGER_PACKET_SECOND_HDR)));
                ss << delta2ToJson(s);
            }
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

bool LoggerItem::get(
    std::map<uint8_t,
    TEMPERATURE_2_BYTES> &retval
) const
{
    LOGGER_MEASUREMENT_HDR *hdr;
    LOGGER_PACKET_TYPE t = extractMeasurementHeader(&hdr, packet.c_str(), packet.size());
    switch (t) {
        case LOGGER_PACKET_RAW: {
            int cnt = (int) ((packet.size() - sizeof(LOGGER_MEASUREMENT_HDR)) / sizeof(LOGGER_DATA_TEMPERATURE_RAW));
            for (int i = 0; i < cnt; i++) {
                LOGGER_DATA_TEMPERATURE_RAW *p = (LOGGER_DATA_TEMPERATURE_RAW *) ((char *) packet.c_str()
                    + sizeof(LOGGER_MEASUREMENT_HDR) + sizeof(LOGGER_DATA_TEMPERATURE_RAW) * i);
                retval[p->sensor] = p->value;
            }
        }
            break;
        case LOGGER_PACKET_PKT_1:
            break;
        case LOGGER_PACKET_PKT_2:
            {
                int cnt = (int) (packet.size() - sizeof(LOGGER_PACKET_SECOND_HDR)) / sizeof(LOGGER_DATA_TEMPERATURE_RAW);
                for (int p = 0; p < cnt; p++) {
                    LOGGER_DATA_TEMPERATURE_RAW *v = extractSecondHdrData(p, packet.c_str(), packet.size());
                    if (!v)
                        break;
                    retval[v->sensor] = v->value;
                }
            }
            break;
        case LOGGER_PACKET_DELTA_1:
            getByDiff(&retval, nullptr, packet, 0);
            break;
        case LOGGER_PACKET_DELTA_2:
            {
                // get packet number from header
                LOGGER_PACKET_SECOND_HDR *h;
                int16_t r = extractSecondHdr(&h, packet.c_str(), packet.size());
                if (r)
                    return false;

                getByDiff(&retval, nullptr, packet, 6 + (h->packet - 2) * 20);
            }
            break;
        case LOGGER_PACKET_HUFF_1:
            {
                std::string s = packet.substr(0, sizeof(LOGGER_PACKET_FIRST_HDR))
                    + decompressLoggerString(packet.substr(sizeof(LOGGER_PACKET_FIRST_HDR)));
                getByDiff(&retval, nullptr, s, 0);
            }
            break;
        case LOGGER_PACKET_HUFF_2:
            {
                std::string s = packet.substr(0, sizeof(LOGGER_PACKET_SECOND_HDR))
                    + decompressLoggerString(packet.substr(sizeof(LOGGER_PACKET_SECOND_HDR)));
                LOGGER_PACKET_SECOND_HDR *h;
                int16_t r = extractSecondHdr(&h, s.c_str(), s.size());
                if (r)
                    return false;
                getByDiff(&retval, nullptr, s, getHuffmanPacketMeasurementOffset(h->packet));
            }
            break;
        default:
            return false;
    }
    return true;
}

/**
 * Get sensor value (not corrected by passport approximation)
 * @param t receive sensor values
 * @return true- success
 */
bool LoggerItem::getTemperature(std::map<uint8_t, double> &retval) const
{
	LOGGER_MEASUREMENT_HDR *hdr;
	LOGGER_PACKET_TYPE t = extractMeasurementHeader(&hdr, packet.c_str(), packet.size());
	switch (t) {
		case LOGGER_PACKET_RAW:
			{
				int cnt = (int) ((packet.size() - sizeof(LOGGER_MEASUREMENT_HDR)) / sizeof(LOGGER_DATA_TEMPERATURE_RAW));
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
                int cnt = (int) ((packet.size() - sizeof(LOGGER_PACKET_SECOND_HDR)) / sizeof(LOGGER_DATA_TEMPERATURE_RAW));
				for (int p = 0; p < cnt; p++) {
					LOGGER_DATA_TEMPERATURE_RAW *v = extractSecondHdrData(p, packet.c_str(), packet.size());
					if (!v)
						break;
                    retval[v->sensor] = TEMPERATURE_2_BYTES_2_double(v->value);
				}
			}
            break;
        case LOGGER_PACKET_DELTA_1:
            getByDiff(nullptr, &retval, packet, 0);
            break;
        case LOGGER_PACKET_DELTA_2:
        {
            // get packet number from header
            LOGGER_PACKET_SECOND_HDR *h;
            int16_t r = extractSecondHdr(&h, packet.c_str(), packet.size());
            if (r)
                return false;
            getByDiff(nullptr, &retval, packet, getHuffmanPacketMeasurementOffset(h->packet));
            break;
        }
        case LOGGER_PACKET_HUFF_1:
            {
                std::string s = packet.substr(0, sizeof(LOGGER_PACKET_FIRST_HDR))
                    + decompressLoggerString(packet.substr(sizeof(LOGGER_PACKET_FIRST_HDR)));
                getByDiff(nullptr, &retval, s, 0);
            }
            break;
        case LOGGER_PACKET_HUFF_2:
            {
                std::string s = packet.substr(0, sizeof(LOGGER_PACKET_SECOND_HDR))
                    + decompressLoggerString(packet.substr(sizeof(LOGGER_PACKET_SECOND_HDR)));
                LOGGER_PACKET_SECOND_HDR *h;
                int16_t r = extractSecondHdr(&h, s.c_str(), s.size());
                if (r)
                    return false;
                int ofs = getHuffmanPacketMeasurementOffset(h->packet);
                getByDiff(nullptr, &retval, s, ofs);
            }
            break;
        default:
			return false;
	}
	return true;
}

/**
 * Get sensor value (not corrected by passport approximation)
 * @param t receive sensor values
 * @return true- success
 */
bool LoggerItem::getCorrectedTemperature(std::map<uint8_t, double> &retval) const
{
    for (auto it(retval.begin()); it != retval.end(); it++) {
        it->second = correctTemperatureByPassport(it->first, it->second);
    }
    return true;
}

void LOGGER_MEASUREMENT_HDR_DIFF_2_LOGGER_MEASUREMENT_HDR(
    LOGGER_MEASUREMENT_HDR *retval,
    LOGGER_MEASUREMENT_HDR_DIFF *value,
    LOGGER_MEASUREMENT_HDR &baseHeader
)
{
    if (!retval)
        return;
    retval->memblockoccupation = baseHeader.memblockoccupation;				// 0 0- memory block occupied
    retval->seconds = baseHeader.seconds + value->delta_sec;           						// 1 0..59
    retval->minutes = baseHeader.minutes;           						// 2 0..59
    retval->hours = baseHeader.hours;           							// 3 0..23
    retval->day = baseHeader.day;				                			// 4 1..31
    retval->month = baseHeader.month;							            // 5 1..12
    retval->year = baseHeader.year;             							// 6 0..99 year - 2000 = last 2 digits
    retval->kosa = baseHeader.kosa;				                			// 7 номер косы в году
    retval->kosa_year = baseHeader.kosa_year;					        	// 8 год косы - 2000 (номер года последние 2 цифры)
    retval->rfu1 = baseHeader.rfu1;							                // 9 reserved
    retval->rfu2 = baseHeader.rfu2;							                // 10 reserved
    retval->vcc = baseHeader.vcc;							                // 11 V cc bus voltage, V
    retval->vbat = baseHeader.vbat;							                // 12 V battery, V
    retval->pcnt = baseHeader.pcnt;							                // 13 pages count, Pcnt = ((ds1820_devices << 2) | pages_to_recods)
    retval->used = baseHeader.used;             			                // 14 record number, 1..65535

    if (value) {
        retval->seconds += value->delta_sec;           						// 1 0..59
        retval->rfu1 += value->rfu1;							            // 9 reserved
        retval->rfu2 += value->rfu2;							            // 10 reserved
        retval->vcc += value->vcc;							                // 11 V cc bus voltage, V
        retval->vbat += value->vbat;							            // 12 V battery, V
        retval->pcnt += value->pcnt;						                // 13 pages count, Pcnt = ((ds1820_devices << 2) | pages_to_recods)
        retval->used += value->used;			                            // 14 record number, 1..65535
    }
}

LOGGER_PACKET_TYPE LoggerItem::set(
    uint8_t &retPackets,
    size_t &retSize,
    uint32_t aAddr,
    const void *aBuffer,
    size_t aSize
)
{
	LOGGER_PACKET_TYPE t = extractLoggerPacketType(&retSize, aBuffer, aSize);	//
	if (t == LOGGER_PACKET_UNKNOWN) {
		retSize = aSize; // go to the end
		return t;
	}
    packet = std::string((const char *) aBuffer, retSize);

	switch (t) {
        case LOGGER_PACKET_RAW:
            {
                LOGGER_MEASUREMENT_HDR *hdr;
                extractMeasurementHeader(&hdr, packet.c_str(), aSize);
                id.set(hdr->kosa, 0, 1, hdr->kosa_year);    // -1: first packet (with no data)
                // retPackets unknown
            }
			break;
		case LOGGER_PACKET_PKT_1:
			{
				LOGGER_PACKET_FIRST_HDR *h1;
                if (extractFirstHdr(&h1, packet.c_str(), aSize))
                    break;  // error occurs
                id.set(h1->kosa, h1->measure, 1, h1->kosa_year);	// -1: first packet (with no data)
				retPackets = h1->packets;
			}
			break;
		case LOGGER_PACKET_PKT_2:
			{
				LOGGER_PACKET_SECOND_HDR *h2;
				if (extractSecondHdr(&h2, packet.c_str(), aSize))
                    break; // some errors
                uint8_t kosaYear = getKosaYearFromFirstPacketOrLoad(h2->kosa);
				id.set(h2->kosa, h2->measure, h2->packet, kosaYear);
			}
			break;
        case LOGGER_PACKET_DELTA_1:
            {
                LOGGER_PACKET_FIRST_HDR *h1;
                if (extractFirstHdr(&h1, packet.c_str(), aSize))
                    break;  // some errors
                id.set(h1->kosa, h1->measure, 1, h1->kosa_year);    // -1: first packet (with no data)
                retPackets = h1->packets;
            }
            break;
        case LOGGER_PACKET_DELTA_2:
            {
                LOGGER_PACKET_SECOND_HDR *h2;
                if (extractSecondHdr(&h2, packet.c_str(), aSize))
                    break;  // some errors
                uint8_t kosaYear = getKosaYearFromFirstPacketOrLoad(h2->kosa);
                id.set(h2->kosa, h2->measure, h2->packet, kosaYear);
                id.packet = h2->packet;
            }
            break;
        case LOGGER_PACKET_HUFF_1:
            {
                LOGGER_PACKET_FIRST_HDR *h1;
                if (extractFirstHdr(&h1, packet.c_str(), aSize)) {
                    id.clear();
                    break;  // some errors
                }
                id.set(h1->kosa, h1->measure, 1, h1->kosa_year);    // -1: first packet (with no data)
                // std::string p = decompressLoggerString(packet.substr(sizeof(LOGGER_PACKET_FIRST_HDR)));
                retPackets = h1->packets;
            }
            break;
        case LOGGER_PACKET_HUFF_2:
            {
                LOGGER_PACKET_SECOND_HDR *h2;
                if (extractSecondHdr(&h2, packet.c_str(), aSize)) {
                    id.clear();
                    break;  // some errors
                }
                // std::string p = decompressLoggerString(packet.substr(sizeof(LOGGER_PACKET_SECOND_HDR)));
                uint8_t kosaYear = getKosaYearFromFirstPacketOrLoad(h2->kosa);
                id.set(h2->kosa, h2->measure, h2->packet, kosaYear);
                id.packet = h2->packet;
            }
            break;
		default: //case LOGGER_PACKET_UNKNOWN:
			retSize = aSize;
			break;
	}
	return t;
}

bool LoggerItem::setMeasurementHeaderFromDiffIfExists() {
    if (!collection)
        return false;
    if (!collection->plumePackets)
        return false;
    LoggerKosaPackets *baseKosa = collection->plumePackets->loadBaseKosa(0);
    if (!baseKosa)
        return false;
    LOGGER_MEASUREMENT_HDR_DIFF *headerMeasurement = extractDiffHdr(packet.c_str(), packet.size());
    if (!headerMeasurement)
        return false;
    LOGGER_MEASUREMENT_HDR_DIFF_2_LOGGER_MEASUREMENT_HDR(&collection->plumePackets->measurementHeader,
        headerMeasurement, baseKosa->measurementHeader);
    return true;
}

/**
 * Return data bits for diff packets
 * @return
 */
int LoggerItem::getDataBits() const {
    int dataBits = 1;
    if (collection) {
        LOGGER_PACKET_FIRST_HDR *h1 = collection->getFirstHeader();
        if (h1) {
            dataBits = h1->status.data_bits;
        }
    }
    return dataBits;
}

uint8_t LoggerItem::getKosaYearFromFirstPacketOrLoad(uint8_t kosa) {
    if (collection) {
        LOGGER_PACKET_FIRST_HDR *firstHeader = collection->getFirstHeader();
        if (!firstHeader)
            firstHeader = collection->getCollectedFirstHeader(kosa);    // try to find in already collected
        if (firstHeader)
            return firstHeader->kosa_year;
    }
    // try load from base loader by address
    if (collection && collection->collector && collection->collector->loggerKosaPacketsLoader) {
        LoggerKosaPackets kp;
        if (collection->collector->loggerKosaPacketsLoader->load(kp, addr)) {
            return kp.id.kosa_year;
        }
    }
    return 0;
}

bool LoggerItem::getByDiff(
    std::map<uint8_t, TEMPERATURE_2_BYTES> *retVal,
    std::map<uint8_t, double> *retValT,
    const std::string &aPacket,
    int offset
) const
{
    if (!collection)
        return false;
    if (!collection->plumePackets)
        return false;

    // get temperature base
    LoggerKosaPackets *base = collection->plumePackets->loadBaseKosa(addr);
    if (!base)
        return false;
    std::map<uint8_t, TEMPERATURE_2_BYTES> baseT;
    base->packets.get(baseT);

    // get data bits in diff: 1 or 2 bytes
    int dataBits = getDataBits();
    int dataBytes = bytesRequiredForBits(dataBits);

    // get count
    int cnt;
    // get sensor index, 3 or 6 sensor data in first (measurement header) packet plus 10 or 20 in the next packets

    uint8_t ofs;
    if (offset) {
        cnt = (uint8_t) ((aPacket.size() - sizeof(LOGGER_PACKET_SECOND_HDR)) / dataBytes);
        ofs = offset / dataBytes;
    } else {
        // first aPacket
        cnt = (int) ((aPacket.size() - sizeof(LOGGER_PACKET_FIRST_HDR) - sizeof(LOGGER_MEASUREMENT_HDR_DIFF)) / dataBytes);
        ofs = 0;
    }

    // packet number => value index
    int i = 0;
    for (int c = ofs; c < ofs + cnt; c++) {
        int diff;
        if (offset)
            diff = getDiff(aPacket.c_str() + sizeof(LOGGER_PACKET_SECOND_HDR), dataBytes, i);
        else
            diff = getDiff(aPacket.c_str() + sizeof(LOGGER_PACKET_FIRST_HDR) + sizeof(LOGGER_MEASUREMENT_HDR_DIFF), dataBytes, i);
        TEMPERATURE_2_BYTES v;
        if (c >= baseT.size())
            break;
        v.t.t00625 = HTON2(HTON2(baseT[c].t.t00625) + diff);
        if (retVal)
            (*retVal)[c] = v;
        if (retValT)
            (*retValT)[c] = TEMPERATURE_2_BYTES_2_double(v);
        i++;
    }
    return true;
}

std::string LoggerItem::delta1ToString(
    const std::string &aPacket
) const
{
    std::stringstream ss;
    LOGGER_PACKET_FIRST_HDR *h1;
    if (extractFirstHdr(&h1, aPacket.c_str(), aPacket.size()))
        return ss.str();
    ss << LOGGER_PACKET_FIRST_HDR_2_string(*h1) << std::endl;
    if (collection && collection->plumePackets)
        ss << LOGGER_MEASUREMENT_HDR_2_string(collection->plumePackets->measurementHeader) << std::endl;
    LOGGER_MEASUREMENT_HDR_DIFF *headerMeasurement = extractDiffHdr(aPacket.c_str(), aPacket.size());
    if (headerMeasurement) {
        ss << LOGGER_MEASUREMENT_HDR_DIFF_2_string(headerMeasurement) << std::endl;
    }
    return ss.str();
}

std::string LoggerItem::delta2ToString(
    const std::string &packet
) const
{
    LOGGER_PACKET_SECOND_HDR *h2;
    int r = extractSecondHdr(&h2, packet.c_str(), packet.size());
    std::stringstream ss;
    if (r == 0)
        ss << "second_header:\t" << LOGGER_PACKET_SECOND_HDR_2_string(*h2) << std::endl;

    int dataBits = getDataBits();
    int dataBytes = bytesRequiredForBits(dataBits);
    int cnt = (int) ((packet.size() - sizeof(LOGGER_PACKET_SECOND_HDR )) / dataBytes);

    // get diffs
    std::vector<int> diffs;
    diffs.resize(cnt);
    for (int i = 0; i < cnt; i++) {
        diffs[i] = getDiff(packet.c_str() + sizeof(LOGGER_PACKET_SECOND_HDR), dataBytes, i);
    }

    // put diffs
    ss << "diff_measurements:\t";
    for (int i = 0; i < cnt; i++) {
        ss << diffs[i] << " ";
    }
    ss << std::endl;
    return ss.str();
}

std::string LoggerItem::delta1ToJson(
    const std::string &aPacket
) const
{
    LOGGER_MEASUREMENT_HDR_DIFF *headerMeasurement = extractDiffHdr(aPacket.c_str(), aPacket.size());
    LOGGER_PACKET_FIRST_HDR *h1;
    int r = extractFirstHdr(&h1, aPacket.c_str(), aPacket.size());
    std::stringstream ss;
    if (!r) {
        const LOGGER_MEASUREMENT_HDR *measurementHeader = getMeasurementHeaderIfExists();
        ss << "\"first_header\": " << LOGGER_PACKET_FIRST_HDR_2_json(*h1) << ", ";
    }
    if (headerMeasurement) {
        ss << "\"delta_header\": "
           << LOGGER_MEASUREMENT_HDR_DIFF_2_json(headerMeasurement) << ", ";
    }

    if (collection) {
        if (collection->plumePackets) {
            ss << "\"measurement_header\": "
               << LOGGER_MEASUREMENT_HDR_2_json(collection->plumePackets->measurementHeader) << ", ";
        }

        int dataBits;
        if (h1)
            dataBits = h1->status.data_bits;
        else
            dataBits = getDataBits();
        int dataBytes = bytesRequiredForBits(dataBits);

        int cnt = (int)((aPacket.size() - sizeof(LOGGER_PACKET_FIRST_HDR) - sizeof(LOGGER_MEASUREMENT_HDR_DIFF)) / dataBytes);

        // get diffs
        std::vector<int> diffs;
        if (cnt > 0)
            diffs.resize(cnt);
        for (int i = 0; i < cnt; i++) {
            diffs[i] = getDiff(aPacket.c_str() + sizeof(LOGGER_PACKET_FIRST_HDR) + sizeof(LOGGER_MEASUREMENT_HDR_DIFF), dataBytes, i);
        }

        // put diffs
        ss << "\"diff_measurements\": [";
        bool isFirst = true;
        for (int i = 0; i < cnt; i++) {
            if (isFirst)
                isFirst = false;
            else
                ss << ", ";
            ss << diffs[i];
        }
        ss << "]";
    }
    return ss.str();
}

std::string LoggerItem::delta2ToJson(
    const std::string &aPacket
) const {
    LOGGER_PACKET_SECOND_HDR *h2;
    int r = extractSecondHdr(&h2, aPacket.c_str(), aPacket.size());
    std::stringstream ss;
    if (r == 0)
        ss << "\"second_header\": " << LOGGER_PACKET_SECOND_HDR_2_json(*h2);

    int dataBits = getDataBits();
    int dataBytes = bytesRequiredForBits(dataBits);
    int cnt = (int) ((aPacket.size() - sizeof(LOGGER_PACKET_SECOND_HDR )) / dataBytes);

    // get diffs
    std::vector<int> diffs;
    diffs.resize(cnt);
    for (int i = 0; i < cnt; i++) {
        diffs[i] = getDiff(aPacket.c_str() + sizeof(LOGGER_PACKET_SECOND_HDR), dataBytes, i);
    }

    // put diffs
    ss << ", \"diff_measurements\": [";
    bool isFirst = true;
    for (int i = 0; i < cnt; i++) {
        if (isFirst)
            isFirst = false;
        else
            ss << ", ";
        ss << diffs[i];
    }
    ss << "]";
    return ss.str();
}

LoggerMeasurementHeader::LoggerMeasurementHeader()
    : header(nullptr), start(0), vcc(0), vbat(0)
{

}

LoggerMeasurementHeader::LoggerMeasurementHeader(
    const LoggerMeasurementHeader &value
)
    : header(value.header), id(value.id), start(value.start), vcc(value.vcc), vbat(value.vbat)
{

}

LoggerMeasurementHeader::LoggerMeasurementHeader(
    const LOGGER_MEASUREMENT_HDR *aHeader,
    size_t sz
)
{
    setHdr(aHeader, sz);
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

LoggerCollection::LoggerCollection()
	: errCode(0), expectedPackets(0), plumePackets(nullptr), collector(nullptr)
{
}

LoggerCollection::LoggerCollection(
	const LoggerCollection &value
)
	: expectedPackets(value.expectedPackets), errCode(value.errCode), plumePackets(value.plumePackets), collector(value.collector)
{
	std::copy(value.items.begin(), value.items.end(), std::back_inserter(items));
}

LoggerCollection::LoggerCollection(
    LoggerKosaCollector *aCollector
)
    : errCode(0), expectedPackets(0), plumePackets(nullptr), collector(aCollector)
{
}

LoggerCollection::~LoggerCollection()
{

}

void LoggerCollection::push(
	LoggerItem &value
)
{
    value.collection = this;
	items.push_back(value);
}

LOGGER_PACKET_TYPE LoggerCollection::put1(
    size_t &retSize,
    std::vector<LOGGER_MEASUREMENT_HDR> *retHeaders,
    uint32_t addr,
    const void *buffer, size_t size
)
{
	LoggerItem item(this);
    item.addr = addr;
	LOGGER_PACKET_TYPE t = item.set(expectedPackets, retSize, addr, buffer, size);

    if (retHeaders) {
        switch (t) {
            case LOGGER_PACKET_RAW:
                retHeaders->push_back(*(LOGGER_MEASUREMENT_HDR *) buffer);
                break;
            case LOGGER_PACKET_PKT_1:
                {
                    // memmove(&item.measurement->id, &item.id, sizeof();
                    LOGGER_MEASUREMENT_HDR *mh;
                    extractMeasurementHeader(&mh, item.packet.c_str(), item.packet.size());
                    item.id.assign(mh);
                    retHeaders->push_back(*mh);
                }
                break;
            case LOGGER_PACKET_DELTA_1:
            case LOGGER_PACKET_HUFF_1:
                {
                    LOGGER_PACKET_FIRST_HDR *h1;
                    int r = extractFirstHdr(&h1, item.packet.c_str(), item.packet.size());
                    if (r)
                        break;
                    item.id.set(h1->kosa, h1->measure, 1, h1->kosa_year);    // -1: first packet (with no data)
                    if (collector) {
                        if (collector->loggerKosaPacketsLoader) {
                            LoggerKosaPackets lkpBase;
                            if (collector->loggerKosaPacketsLoader->load(lkpBase, item.addr)) {
                                LOGGER_MEASUREMENT_HDR_DIFF *mhd;
                                if (t == LOGGER_PACKET_HUFF_1) {
                                    std::string s = item.packet.substr(0, sizeof(LOGGER_PACKET_FIRST_HDR))
                                        + decompressLoggerString(item.packet.substr(sizeof(LOGGER_PACKET_FIRST_HDR)));
                                    mhd = extractDiffHdr(s.c_str(), s.size());
                                } else
                                    mhd = extractDiffHdr(item.packet.c_str(), item.packet.size());
                                LOGGER_MEASUREMENT_HDR_DIFF_2_LOGGER_MEASUREMENT_HDR(&lkpBase.measurementHeader, mhd,
                                    lkpBase.measurementHeader);
                                retHeaders->push_back(lkpBase.measurementHeader);
                            }
                        }
                    }
                }
                break;
            case LOGGER_PACKET_DELTA_2:
            case LOGGER_PACKET_HUFF_2:
                {
                    LOGGER_PACKET_SECOND_HDR *h2;
                    extractSecondHdr(&h2, buffer, size);
                    if (collector) {
                        if (collector->loggerKosaPacketsLoader) {
                            LoggerKosaPackets kp;
                            if (collector->loggerKosaPacketsLoader->load(kp, item.addr)) {
                                item.id = kp.id;
                            }
                        }
                    }
                    item.id.packet = h2->packet;
                }
                break;
            default:
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
        uint32_t addr,
        const void *buffer,
        size_t size
)
{
	retSize = size;
	if (items.empty()) {
        LoggerItem li;
        items.push_back(li);
    }
	LoggerItem &item = items.front();
    item.addr = addr;
    if (item.packet.size() < MAX_BUFFER_SIZE)
	    item.packet = item.packet + std::string((const char *) buffer, size);
	expectedPackets = (uint8_t) items.size();
}

LOGGER_PACKET_TYPE
LoggerCollection::put(
        size_t &retSize,
        std::vector<LOGGER_MEASUREMENT_HDR> *retHeaders,
        uint32_t addr,
        const void *buffer,
        size_t aSize
)
{
    // check does it raw packet
    LOGGER_PACKET_TYPE packetType = extractLoggerPacketType(nullptr, buffer, aSize);
    size_t sz;
    void *next = (void *) buffer;
    retSize = aSize;

    while (true) {
        if (packetType == LOGGER_PACKET_RAW) {
            putRaw(sz, addr, next, retSize);
        } else {
            packetType = put1(sz, retHeaders, addr, next, retSize);
        }
        if (sz >= retSize)
            break;
        retSize -= sz;
        next = (char *) next + sz;
    }
    return packetType;
}

LOGGER_PACKET_TYPE LoggerCollection::put(
        std::vector<LOGGER_MEASUREMENT_HDR> *retHeaders,
        uint32_t addr,
        const std::vector<std::string> &values
)
{
	LOGGER_PACKET_TYPE t = LOGGER_PACKET_UNKNOWN;
	for (std::vector<std::string>::const_iterator it(values.begin()); it != values.end(); it++) {
		size_t sz;
        t = put(sz, retHeaders, addr, it->c_str(), it->size());
	}
	return t;
}

bool LoggerCollection::completed() const
{
	return (items.size() >= expectedPackets) && (expectedPackets > 0);
}

bool LoggerCollection::get(
    std::map<uint8_t, TEMPERATURE_2_BYTES> &retval
) const
{
    for (std::vector<LoggerItem>::const_iterator it(items.begin()); it != items.end(); it++) {
        it->get(retval);
    }
    return true;
}

bool LoggerCollection::getTemperature(
    std::map<uint8_t, double> &retval
) const
{
	for (std::vector<LoggerItem>::const_iterator it(items.begin()); it != items.end(); it++) {
        it->getTemperature(retval);
	}
	return true;
}

bool LoggerCollection::getCorrectedTemperature(
    std::map<uint8_t, double> &retval
) const
{
    for (std::vector<LoggerItem>::const_iterator it(items.begin()); it != items.end(); it++) {
        it->getCorrectedTemperature(retval);
    }
    return true;
}

time_t LoggerKosaPackets::measured() const
{
    time_t t = logger2time(
        measurementHeader.year,
        measurementHeader.month - 1,
        measurementHeader.day,
        measurementHeader.hours,
        measurementHeader.minutes,
        measurementHeader.seconds,
        true
    );
    return t;
}

/**
 * Find out first packet header in packet collection
 * Get bits size 1 or 2 bits in
 * @return nullptr if not exists
 */
LOGGER_PACKET_FIRST_HDR *LoggerCollection::getFirstHeader()
{
    for (std::vector<LoggerItem>::const_iterator it(items.begin()); it != items.end(); it++ ) {
        size_t sz;
        LOGGER_PACKET_TYPE t = extractLoggerPacketType(&sz, it->packet.c_str(), it->packet.size());
        switch (t) {
            case LOGGER_PACKET_PKT_1:
            case LOGGER_PACKET_DELTA_1:
            case LOGGER_PACKET_HUFF_1:
                {
                    LOGGER_PACKET_FIRST_HDR *h1;
                    int r = extractFirstHdr(&h1, it->packet.c_str(), it->packet.size());
                    if (r)
                        return nullptr;
                    return h1;
                }
            default:
                return nullptr;
        }
    }
    return nullptr;
}

/**
 * Find out first packet header in packet collection in collector
 * Get bits size 1 or 2 bits in
 * @return nullptr if not exists
 */
LOGGER_PACKET_FIRST_HDR *LoggerCollection::getCollectedFirstHeader(
    uint8_t kosa
)
{
    if (!collector)
        return nullptr;
    LoggerItemId id = { kosa, 0, 0, NO_MATCH_YEAR };
    std::vector<LoggerKosaPackets>::iterator it(std::find(collector->koses.begin(), collector->koses.end(), id));
    if (it == collector->koses.end())
        return nullptr;
    return it->packets.getFirstHeader();
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
		ss << it->toJsonString();
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

	if (getTemperature(r)) {
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
	: collector(nullptr), start(0), baseKosa(nullptr)
{
    packets.plumePackets = this;
    clear_LOGGER_MEASUREMENT_HDR(measurementHeader);
}

LoggerKosaPackets::LoggerKosaPackets(
    LoggerKosaCollector *aCollector
)
	: collector(aCollector), start(0), baseKosa(nullptr)
{
    packets.plumePackets = this;
    clear_LOGGER_MEASUREMENT_HDR(measurementHeader);
}

LoggerKosaPackets::LoggerKosaPackets(
	const LoggerKosaPackets &value
)
	: collector(value.collector), id(value.id), start(value.start), measurementHeader(value.measurementHeader), baseKosa(nullptr)
{
    // do not copy baseKosa
	std::copy(value.packets.items.begin(), value.packets.items.end(), std::back_inserter(packets.items));
    packets.plumePackets = this;
}

LoggerKosaPackets::LoggerKosaPackets(
	const LoggerItem &value
)
	: collector(nullptr), baseKosa(nullptr)
{
    start = time(nullptr);
	id = value.id;
    packets.items.push_back(value);
    packets.plumePackets = this;
    clear_LOGGER_MEASUREMENT_HDR(measurementHeader);
}

LoggerKosaPackets::~LoggerKosaPackets()
{
    if (baseKosa) {
        delete baseKosa;
        baseKosa = nullptr;
    }
}

bool LoggerKosaPackets::expired() const 
{
	return (time(nullptr) - start) > MAX_SECONDS_WAIT_KOSA_PACKETS;
}

bool LoggerKosaPackets::add(
	LoggerItem &value
)
{
    LOGGER_MEASUREMENT_HDR *mh = value.getMeasurementHeaderIfExists();
	if (mh)
		memmove(&measurementHeader, mh, sizeof(LOGGER_MEASUREMENT_HDR));
	bool newOne = packets.items.empty();
	if (newOne || (value.id.kosa == id.kosa && value.id.kosa_year == id.kosa_year)) {
		if (newOne) {
			start = time(nullptr);
			id = value.id;
		}
        value.collection = &this->packets;
		packets.items.push_back(value);
		return true;
	}
	return false;
}

bool LoggerKosaPackets::operator==(
	const LoggerItemId &another
) const
{
	return (id.kosa == another.kosa) 
        && (
            (another.kosa_year == NO_MATCH_YEAR)
            || (id.kosa_year == another.kosa_year)
        );
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
    bool usePassport = false;
    std::string passportText;
#ifdef ENABLE_LOGGER_PASSPORT
    if (collector && collector->passportDescriptor) {
        usePassport = hasPassport(collector->passportDescriptor, FORMAT_PASSPORT_TEXT, &passportText, id.kosa, id.kosa_year);
    }
#endif
    std::stringstream ss;
    ss << id.toString() << " " << time2string(measured(), true) << std::endl;
#ifdef PRINT_PASSPORT_TEXT
    if (usePassport) {
        ss << "Passport: " << passportText << std::endl;
    }
#endif
    ss << "T  ";
    temperatureCommaString(ss, " ", "N/A");
    if (usePassport) {
        ss << std::endl << "Tp ";
        temperatureCorrectedCommaString(ss, " ", "N/A");
    }
    ss << std::endl << "Th ";
    valueCommaString(ss, " ", "", "", "N/A");
    ss << std::endl;
    return ss.str();
}

std::string LoggerKosaPackets::packetsToString() const
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
    bool usePassport = false;
    std::string passportJson;
#ifdef ENABLE_LOGGER_PASSPORT
    if (collector && collector->passportDescriptor) {
        usePassport = hasPassport(collector->passportDescriptor, FORMAT_PASSPORT_JSON, &passportJson, id.kosa, id.kosa_year);
    }
#endif
    std::stringstream ss;
    ss << "{\"id\": " <<  id.toJsonString()
        << ", \"hasPassport\": " << (usePassport ? "true" : "false");
#ifdef PRINT_PASSPORT_JSON
    if (usePassport && !passportJson.empty())
        ss << ", \"passport\": " << passportJson;
#endif
    ss << ", \"t\": [";
    temperatureCommaString(ss, ", ", JS_NULL);
    ss << "], \"tp\": [";
    temperatureCorrectedCommaString(ss, ", ", JS_NULL);
    ss << "], \"raw\": [";
    rawCommaString(ss, ", ", "\"", "\"");
    ss << "], \"th\": [";
    valueCommaString(ss, ", ", "\"", "\"", JS_NULL);
    ss << "]}";
    return ss.str();
}

std::string LoggerKosaPackets::packetsToJsonString() const
{
	std::stringstream ss;
	ss << "{\"id\": " <<  id.toJsonString()
		<< ", \"start\": " << start
		<< ", \"expired\": " << (expired() ? "true" : "false")
		<< ", \"completed\": " << (packets.completed() ? "true" : "false")
        << ", \"measurement_header\": " << LOGGER_MEASUREMENT_HDR_2_json(measurementHeader)
		<< ", \"packets\": " << packets.toJsonString()
		<< "}";
	return ss.str();
}

std::string LoggerKosaPackets::toTableString() const
{
    std::stringstream ss;
    ss << id.toString() << "\t" << time2string(measured(), true) << "\t";
    temperatureCorrectedCommaString(ss, "\t", "N/A");
    return ss.str();
}

void LoggerKosaPackets::valueCommaString(
        std::ostream &ostrm,
        const std::string &separator,
        const std::string &oparen,
        const std::string &cparen,
        const std::string &substEmptyValue
) const
{
    std::map<uint8_t, TEMPERATURE_2_BYTES> r;
    if (packets.get(r)) {
        bool isFirst = true;
        int c = 0;
        // map is sorted by the first
        for (std::map<uint8_t, TEMPERATURE_2_BYTES>::const_iterator it(r.begin()); it != r.end(); it++) {
            if (isFirst)
                isFirst = false;
            else
                ostrm << separator;
            // some sensors can be missed
            int skipped = it->first - c;
            for (int i = 0; i < skipped; i++) {
                ostrm << substEmptyValue << separator;
            }
            ostrm << oparen
                << std::setfill('0') << std::setw(2) << std::hex
                << (int) (uint8_t) it->second.t.f.lo
                << std::setw(2)
                << (int) (uint8_t) it->second.t.f.hi
                << cparen;
            c++;
        }
    }
}

void LoggerKosaPackets::temperatureCommaString(
    std::ostream &ostrm,
    const std::string &separator,
    const std::string &substEmptyValue
) const
{
    std::map<uint8_t, double> r;
    if (packets.getTemperature(r)) {
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
            ostrm << std::fixed << std::setprecision(4) << it->second;
            c++;
        }
    }
}

void LoggerKosaPackets::temperatureCorrectedCommaString(
    std::ostream &ostrm,
    const std::string &separator,
    const std::string &substEmptyValue
) const
{
#ifdef ENABLE_LOGGER_PASSPORT
	if (!(collector && collector->passportDescriptor))
		return temperatureCommaString(ostrm, separator, substEmptyValue);

    std::map<uint8_t, double> r;
    if (packets.getTemperature(r)) {
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
				ostrm << std::fixed << std::setprecision(4) << it->second;
			else
            	ostrm << std::fixed << std::setprecision(4) << calcTemperature(collector->passportDescriptor,
                    id.kosa, id.kosa_year, it->first - 1, it->second);
            c++;
        }
    }
#else
	return temperatureCommaString(ostrm, separator, substEmptyValue);
#endif
}

void LoggerKosaPackets::rawCommaString(
    std::ostream &ostrm,
    const std::string &separator,
    const std::string &oparen,
    const std::string &cparen
) const
{
    bool isFirst = true;
    // map is sorted by the first
    for (std::vector<LoggerItem>::const_iterator it(packets.items.begin()); it != packets.items.end(); it++) {
        if (isFirst) {
            isFirst = false;
        } else
            ostrm << separator;
        ostrm << oparen << bin2hexString(it->packet) << cparen;
    }
}

LoggerKosaPackets *LoggerKosaPackets::loadBaseKosa(uint32_t addr)
{
    if (baseKosa)
        return baseKosa;
    if (!collector)
        return nullptr;
    if (!collector->loggerKosaPacketsLoader)
        return nullptr;
    baseKosa = new LoggerKosaPackets;

    if (!collector->loggerKosaPacketsLoader->load(*baseKosa, addr)) {
        delete baseKosa;
        baseKosa = nullptr;
    }
    return baseKosa;
}

/**
 * SQL fields: plumePackets, year, no, measured, parsed, vcc, vbat, t, tp, raw, temperature raw integer hex value, temperature(not corrected)
 * @param retval out param values
 * @param substEmptyValue if value is not set, substitute this value, e.g. "null"
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
    retval.push_back(vcc2string(measurementHeader.vcc));
    retval.push_back(vbat2string(measurementHeader.vbat));

    std::stringstream ss;
    temperatureCommaString(ss, ",", substEmptyValue);
    retval.push_back(ss.str());

    std::stringstream ssp;
    temperatureCorrectedCommaString(ssp, ",", substEmptyValue);
    retval.push_back(ssp.str());

    std::stringstream ssr;
    rawCommaString(ssr, " ", "", "");
    retval.push_back(ssr.str());

    // 2 byte int as hex temperatures
    std::stringstream ssti;
    valueCommaString(ssti, ",", "", "", substEmptyValue);
    retval.push_back(ssti.str());
}

void LoggerKosaPackets::updateKosaAfterCopy()
{
    packets.plumePackets = this;
    // set parent pointer
    for (auto it(packets.items.begin()); it != packets.items.end(); it++) {
        it->collection = &packets;
    }
}

LoggerKosaCollector::LoggerKosaCollector()
    : passportDescriptor(nullptr), loggerKosaPacketsLoader(nullptr)
{
}

LoggerKosaCollector::~LoggerKosaCollector()
{

}

/**
 * @return removed items count
 */
int LoggerKosaCollector::rmExpired()
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
void LoggerKosaCollector::add(
    LoggerCollection &value
)
{
    for (std::vector<LoggerItem>::iterator itItem(value.items.begin()); itItem != value.items.end(); itItem++) {
        bool found = false;
        for (std::vector<LoggerKosaPackets>::iterator itKosa(koses.begin()); itKosa != koses.end(); itKosa++) {
            if (*itKosa == itItem->id) {
                itKosa->add(*itItem);
                if (value.expectedPackets) {
                    itKosa->packets.expectedPackets = value.expectedPackets;
                }
                itKosa->packets.plumePackets = &*itKosa;
                found = true;
                break;
            }
        }
        if (!found) {
            LoggerKosaPackets p(this);
            p.add(*itItem);
            koses.push_back(p);
            LoggerKosaPackets *kp = &koses[0];
            kp->updateKosaAfterCopy();
            if (value.expectedPackets) {
                kp->packets.expectedPackets = value.expectedPackets;
            }
        }
    }
}

/**
 * Put char buffer
 */
LOGGER_PACKET_TYPE
LoggerKosaCollector::put(
        LoggerKosaCollector *aCollector,
        size_t &retSize,
        uint32_t addr,
        const void *buffer,
        size_t size
)
{
	LoggerCollection c(aCollector);
    // collect headers from the packet(s)
    std::vector<LOGGER_MEASUREMENT_HDR> mhs;
	LOGGER_PACKET_TYPE r = c.put(retSize, &mhs, addr, buffer, size);
    // copy items from raw collection group by logger
	add(c);

    // copy header(s)
    for (std::vector<LOGGER_MEASUREMENT_HDR>::const_iterator it(mhs.begin()); it != mhs.end(); it++) {
        addHeader(*it);
    }
    return r;
}

/**
 * Put collection of strings
 */
LOGGER_PACKET_TYPE LoggerKosaCollector::put(
    uint32_t addr,
    const std::vector<std::string> &values
)
{
	// temporary raw collection
	LoggerCollection c(this);
    std::vector<LOGGER_MEASUREMENT_HDR> mhs;
	LOGGER_PACKET_TYPE r = c.put(&mhs, addr, values);

	// copy items from raw collection group by logger
    add(c);

    // copy header(s)
    for (std::vector<LOGGER_MEASUREMENT_HDR>::const_iterator it(mhs.begin()); it != mhs.end(); it++) {
        addHeader(*it);
    }
    return r;
}

/**
 * Put one string
 */
LOGGER_PACKET_TYPE LoggerKosaCollector::put(
    uint32_t addr,
    const std::string &value
)
{
    size_t sz;
    // return put(nullptr, sz, addr, value.c_str(), value.size());

    // temporary raw collection
    LoggerCollection c(this);
    std::vector<LOGGER_MEASUREMENT_HDR> mhs;
    LOGGER_PACKET_TYPE r = c.put(sz, &mhs, addr, value.c_str(), value.size());

    // copy items from raw collection group by logger
    add(c);

    // copy header(s)
    for (std::vector<LOGGER_MEASUREMENT_HDR>::const_iterator it(mhs.begin()); it != mhs.end(); it++) {
        addHeader(*it);
    }
    return r;

}

std::string LoggerKosaCollector::toString() const
{
    std::stringstream ss;
    for (std::vector<LoggerKosaPackets>::const_iterator it(koses.begin()); it != koses.end(); it++) {
        ss << it->toString() << std::endl;
    }
    return ss.str();
}

std::string LoggerKosaCollector::packetsToString() const
{
	std::stringstream ss;
	bool first = true;
	for (std::vector<LoggerKosaPackets>::const_iterator it(koses.begin()); it != koses.end(); it++) {
		if (first)
			first = false;
		else
			ss << ", ";
		ss << it->packetsToString();
	}
	return ss.str();
}

std::string LoggerKosaCollector::toJsonString() const
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

std::string LoggerKosaCollector::packetsToJsonString() const
{
	std::stringstream ss;
	bool first = true;
	ss << "[";
	for (std::vector<LoggerKosaPackets>::const_iterator it(koses.begin()); it != koses.end(); it++) {
		if (first)
			first = false;
		else
			ss << ", ";
		ss << it->packetsToJsonString();
	}
	ss << "]";
	return ss.str();
}

std::string LoggerKosaCollector::toTableString() const
{
	std::stringstream ss;
	for (std::vector<LoggerKosaPackets>::const_iterator it(koses.begin()); it != koses.end(); it++) {
		ss << it->toTableString() << std::endl;
	}
	return ss.str();
}

bool LoggerKosaCollector::addHeader(
    const LOGGER_MEASUREMENT_HDR &value
)
{
    LoggerMeasurementHeader v(&value, sizeof(LOGGER_MEASUREMENT_HDR));
    std::vector<LoggerKosaPackets>::iterator it(std::find(koses.begin(), koses.end(), v.id));
    if (it == koses.end())
        return false;
    v.assign(it->measurementHeader);
    return true;
}

void LoggerKosaCollector::setPassports(
	void *value
)
{
	passportDescriptor = value;
}

void LoggerKosaCollector::setLoggerKosaPacketsLoader(LoggerKosaPacketsLoader *value)
{
    loggerKosaPacketsLoader = value;
}

void clear_LOGGER_MEASUREMENT_HDR(
    LOGGER_MEASUREMENT_HDR &value
)
{
    std::fill((char *) &value, ((char *) &value) + sizeof(LOGGER_MEASUREMENT_HDR), 0);
    // memset(&value, 0, sizeof(LOGGER_MEASUREMENT_HDR));
}
