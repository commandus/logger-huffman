#ifndef LOGGER_HUFFMAN_IMPL_H_
#define LOGGER_HUFFMAN_IMPL_H_ 1

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
		LoggerItemId& operator=(const LoggerItemId& other);
		bool operator==(const LoggerItemId &another) const;
		bool operator!=(const LoggerItemId &another) const;

		std::string toString() const;
		std::string toJsonString() const;
};

class LoggerItem {
	public:
		LoggerItemId id;
		std::string packet;
		time_t received;
		int errCode;

		LOGGER_MEASUREMENT_HDR *measurement;
		// std::string errDescription;
		LoggerItem();
		LoggerItem(time_t t);
		LoggerItem(const LoggerItem &value);
		LoggerItem(const void *buffer, size_t size);
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
		LOGGER_PACKET_TYPE put(size_t &retSize, const void *buffer, size_t size);
		void putRaw(size_t &retSize, const void *buffer, size_t size);

		/**
		 * Put collection of strings
		 */
		LOGGER_PACKET_TYPE put(const std::vector<std::string> values);

		bool completed() const;
		bool get(std::map<uint8_t, double> &t) const;

		std::string toString() const;
		std::string toJsonString() const;
		std::string toTableString(
			const LoggerItemId &id,
			const time_t &t
		) const;
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

		std::string toString() const;
		std::string toJsonString() const;
		std::string toTableString() const;
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

		bool add(const LoggerItem &value);
		// helper functions
		/**
		 * Put char buffer
		 */
		LOGGER_PACKET_TYPE put(size_t &retSize, const void *buffer, size_t size);
		/**
		 * Put collection of strings
		 */
		LOGGER_PACKET_TYPE put(const std::vector<std::string> values);

		std::string toString() const;
		std::string toJsonString() const;
		std::string toTableString() const;
};

std::string LOGGER_PACKET_TYPE_2_string(const LOGGER_PACKET_TYPE &value);

LOGGER_PACKET_TYPE LOGGER_PACKET_TYPE_2_string(const std::string &value);

std::string LOGGER_MEASUREMENT_HDR_2_string(const LOGGER_MEASUREMENT_HDR &value);
std::string LOGGER_MEASUREMENT_HDR_2_json(const LOGGER_MEASUREMENT_HDR &value);
std::string LOGGER_DATA_TEMPERATURE_RAW_2_json(const LOGGER_DATA_TEMPERATURE_RAW *value);
std::string LOGGER_PACKET_FIRST_HDR_2_string(const LOGGER_PACKET_FIRST_HDR &value);
std::string LOGGER_PACKET_FIRST_HDR_2_json(const LOGGER_PACKET_FIRST_HDR &value);
std::string LOGGER_PACKET_SECOND_HDR_2_json(const LOGGER_PACKET_SECOND_HDR &value);

/** hexadecimal data represented string to binary */
std::string hex2binString(const char *hexChars, size_t size);
/** binary data to hexadecimal represented data string to binary */
std::string bin2hexString(const char *binChars, size_t size);
std::string bin2hexString(const std::string &value);

const char *strerror_logger_huffman(int errCode);

#endif
