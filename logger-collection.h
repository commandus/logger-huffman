#ifndef LOGGER_COLLECTION_H_
#define LOGGER_COLLECTION_H_ 1

#include <string>
#include <vector>
#include <map>

#include "logger-huffman.h"

// следующий пакет typ 4b- plain 49- delta 4d- huffman
class LoggerItemId {
	public:
		uint8_t kosa;							// идентификатор косы (номер, дата)
		uint8_t measure;						// мл. Байт номера замера, lsb used (или addr_used?)
		int8_t packet;							// packet number
		uint8_t kosa_year;						// reserved for first packet

		LoggerItemId();
		LoggerItemId(
			uint8_t kosa,						// идентификатор косы (номер, дата)
			uint8_t measure,					// мл. Байт номера замера, lsb used (или addr_used?)
			uint8_t packet,						// packet number
			uint8_t kosa_year
		);

		/**
		 * Set identifier
		 * @param akosa kosa number
		 * @param ameasure measurement no
		 * @param apacket -1- first packet (w/o data)
		 */ 
		void set(
			uint8_t kosa,						// идентификатор косы (номер, дата)
			uint8_t measure,					// мл. Байт номера замера, lsb used (или addr_used?)
			int8_t packet,						// packet number
			uint8_t kosa_year
		);
		void assign(
			LOGGER_MEASUREMENT_HDR *retval
		);

		LoggerItemId& operator=(const LoggerItemId& other);
		bool operator==(const LoggerItemId &another) const;
		bool operator!=(const LoggerItemId &another) const;

		std::string toString() const;
		std::string toJsonString() const;

        std::string kosaString() const;							// идентификатор косы (номер, дата)
        std::string measureString() const;						// мл. Байт номера замера, lsb used (или addr_used?)
        std::string packetStrng() const;						// packet number
        std::string kosaYearString() const; 					// reserved for first packet
        void set(const LOGGER_MEASUREMENT_HDR &param);
};

/**
 * Keep measurements
 */
class LoggerItem {
	public:
		LoggerItemId id;
		std::string packet;
		time_t parsed;
		int errCode;

		LOGGER_MEASUREMENT_HDR *measurement;
		// std::string errDescription;
		LoggerItem();
		LoggerItem(time_t t);
		LoggerItem(const LoggerItem &value);
		LoggerItem(const void *aBuffer, size_t aSize);
		virtual ~LoggerItem();

		LoggerItem& operator=(const LoggerItem& other);
		bool operator==(const LoggerItem &another) const;
		bool operator==(const LoggerItemId &id) const;
		
		bool operator!=(const LoggerItem &another) const;
		bool operator!=(const LoggerItemId &id) const;

		LOGGER_PACKET_TYPE set(uint8_t &retPackets, size_t &retSize, const void *buffer, size_t size);
		bool get(std::map<uint8_t, double> &t) const;

		std::string toString() const;
		std::string toJsonString() const;
		std::string toTableString() const;
};

/**
 * keep LOGGER_MEASUREMENT_HDR
 */
class LoggerMeasurementHeader {
public:
	const LOGGER_MEASUREMENT_HDR *header;
    LoggerItemId id;
    time_t start;
    uint8_t vcc;
    uint8_t vbat;

    LoggerMeasurementHeader();
    LoggerMeasurementHeader(const LoggerMeasurementHeader &value);
    LoggerMeasurementHeader(const LOGGER_MEASUREMENT_HDR *pheader , size_t sz);

    LoggerMeasurementHeader& operator=(const LOGGER_MEASUREMENT_HDR &value);
    bool operator==(const LoggerItemId &another) const;
    bool operator==(const LoggerMeasurementHeader &value) const;

    bool operator!=(const LoggerItemId &another) const;
    bool operator!=(const LoggerMeasurementHeader &value) const;

    bool setHdr(const LOGGER_MEASUREMENT_HDR *pHeader, size_t sz);

    void assign(LOGGER_MEASUREMENT_HDR &retval) const;
};

/** 
 * Raw collection of packets
 */
class LoggerCollection {
	public:
		std::vector<LoggerItem> items;
		uint8_t expectedPackets;	// keep expected packets
		int errCode;

		LoggerCollection();
		LoggerCollection(const LoggerCollection &value);
		virtual ~LoggerCollection();

		void push(const LoggerItem &value);
        /**
          * Put char buffer
          */
        LOGGER_PACKET_TYPE put(size_t &retSize, std::vector<LoggerMeasurementHeader> *retHeaders, const void *buffer, size_t size);
		/**
		 * Put collection of strings
		 */
		LOGGER_PACKET_TYPE put(
                std::vector<LoggerMeasurementHeader> *retHeaders,
                const std::vector<std::string> values);

		bool completed() const;
		bool get(std::map<uint8_t, double> &retval) const;

		std::string toString() const;
		std::string toJsonString() const;
		std::string toTableString(const LoggerItemId &id, const time_t &t, const LOGGER_MEASUREMENT_HDR &header) const;
private:
    LOGGER_PACKET_TYPE put1(size_t &retSize, std::vector<LoggerMeasurementHeader> *retHeaders,
        const void *buffer, size_t size);
    void putRaw(size_t &retSize, const void *buffer, size_t size);
};

//  5'
#define MAX_SECONDS_WAIT_KOSA_PACKETS 5 * 60

/** 
 * Kosa packets collection
 */
class LoggerKosaPackets {
	public:
		LoggerItemId id;
		time_t start;
        LOGGER_MEASUREMENT_HDR header;
		LoggerCollection packets;

		LoggerKosaPackets();
		LoggerKosaPackets(const LoggerKosaPackets &value);
		LoggerKosaPackets(const LoggerItem &value);
		virtual ~LoggerKosaPackets();

		bool expired() const;

		bool add(const LoggerItem &value);

		// do not compare with packet!
		bool operator==(const LoggerItemId &another) const;
		bool operator!=(const LoggerItemId &another) const;

		bool operator==(uint8_t kosa) const;
		bool operator!=(uint8_t kosa) const;

        time_t measured() const;

        std::string toString() const;
		std::string toJsonString() const;
		std::string toTableString() const;

        void temperatureCommaString(std::ostream &ostrm, const std::string &separator, const std::string &substEmptyValue) const;
        void rawCommaString(std::ostream &ostrm, const std::string &separator) const;
        void toStrings(std::vector<std::string> &retval, const std::string &substEmptyValue) const;
};

/** 
 * Make an collection of kosa
 */
class LoggerKosaCollection {
	public:
		std::vector<LoggerKosaPackets> koses;

		LoggerKosaCollection();
		virtual ~LoggerKosaCollection();

		int rmExpired();

		void add(LoggerCollection &value);
		// helper functions
		/**
		 * Put char buffer
		 */
		LOGGER_PACKET_TYPE put(size_t &retSize, const void *buffer, size_t size);
        /**
        * Put one string
        */
        LOGGER_PACKET_TYPE put(const std::string &value);
        /**
		 * Put collection of strings
		 */
		LOGGER_PACKET_TYPE put(const std::vector<std::string> values);

		std::string toString() const;
		std::string toJsonString() const;
		std::string toTableString() const;
        bool addHeader(const LoggerMeasurementHeader &header);
};

std::string LOGGER_PACKET_TYPE_2_string(const LOGGER_PACKET_TYPE &value);

LOGGER_PACKET_TYPE LOGGER_PACKET_TYPE_2_string(const std::string &value);

std::string LOGGER_MEASUREMENT_HDR_2_string(const LOGGER_MEASUREMENT_HDR &value);
std::string LOGGER_MEASUREMENT_HDR_2_json(const LOGGER_MEASUREMENT_HDR &value);
std::string LOGGER_MEASUREMENT_HDR_2_table(const LOGGER_MEASUREMENT_HDR &value);
std::string vcc2string(uint8_t value);

std::string LOGGER_DATA_TEMPERATURE_RAW_2_json(const LOGGER_DATA_TEMPERATURE_RAW *value);
std::string LOGGER_DATA_TEMPERATURE_RAW_2_text(const LOGGER_DATA_TEMPERATURE_RAW *value);
std::string LOGGER_PACKET_FIRST_HDR_2_string(const LOGGER_PACKET_FIRST_HDR &value);
std::string LOGGER_PACKET_FIRST_HDR_2_json(const LOGGER_PACKET_FIRST_HDR &value);
std::string LOGGER_PACKET_SECOND_HDR_2_json(const LOGGER_PACKET_SECOND_HDR &value);

/** hexadecimal data represented string to binary */
std::string hex2binString(const char *hexChars, size_t size);
std::string hex2binString(const std::string &value);
/** binary data to hexadecimal represented data string to binary */
std::string bin2hexString(const char *binChars, size_t size);
std::string bin2hexString(const std::string &value);

const char *strerror_logger_huffman(int errCode);

void clear_LOGGER_MEASUREMENT_HDR(LOGGER_MEASUREMENT_HDR &value);

std::string compressLoggerString(const std::string &value);
std::string decompressLoggerString(const std::string &value);

#endif
