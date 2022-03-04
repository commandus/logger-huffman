#ifndef LOGGER_HUFFMAN_IMPL_H_
#define LOGGER_HUFFMAN_IMPL_H_ 1

#include <string>
#include <vector>

#include "logger-huffman.h"

// следующий пакет typ 4b- plain 49- delta 4d- huffman
class LoggerItemId {
	public:
		uint8_t kosa;							// идентификатор косы (номер, дата)
		uint8_t measure;						// мл. Байт номера замера, lsb used (или addr_used?)
		uint8_t packet;							// packet number
		LoggerItemId();
		LoggerItemId(
			uint8_t kosa,						// идентификатор косы (номер, дата)
			uint8_t measure,						// мл. Байт номера замера, lsb used (или addr_used?)
			uint8_t packet						// packet number
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
			int8_t packet						// packet number
		);
		bool operator==(const LoggerItemId &another);
		bool operator!=(const LoggerItemId &another);
};

class LoggerItem {
	public:
		LoggerItemId id;
		std::string packet;
		int errCode;

		LOGGER_MEASUREMENT_HDR *measurement;
		// std::string errDescription;
		LoggerItem();
		LoggerItem(const LoggerItem &value);
		LoggerItem(const void *buffer, size_t size);
		virtual ~LoggerItem();

		LoggerItem& operator=(const LoggerItem& other);
		bool operator==(const LoggerItem &another);
		bool operator!=(const LoggerItem &another);

		LOGGER_PACKET_TYPE set(size_t &retSize, const void *buffer, size_t size);

		std::string toString() const;
		std::string toJsonString() const;
};

class LoggerCollection {
	public:
		std::vector <LoggerItem> items;
		int errCode;
		// std::string errDescription;
		LoggerCollection();
		virtual ~LoggerCollection();
		LOGGER_PACKET_TYPE put(size_t &retSize, const void *buffer, size_t size);
		std::string toString() const;
		std::string toJsonString() const;
};

std::string LOGGER_PACKET_TYPE_2_string(const LOGGER_PACKET_TYPE &value);

LOGGER_PACKET_TYPE LOGGER_PACKET_TYPE_2_string(const std::string &value);

std::string LOGGER_MEASUREMENT_HDR_2_string(const LOGGER_MEASUREMENT_HDR &value);
std::string LOGGER_PACKET_FIRST_HDR_2_string(const LOGGER_PACKET_FIRST_HDR &value);

std::string hex2binString(const char *hexChars, size_t size);
std::string bin2hexString(const char *binChars, size_t size);

const char *strerror_logger_huffman(int errCode);

#endif
