#ifndef LOGGER_HUFFMAN_IMPL_H_
#define LOGGER_HUFFMAN_IMPL_H_ 1

#include <string>

#include "logger-huffman.h"

class LoggerPacket {
	const char *buffer;
	size_t size;
	public:
		int errCode;
		// std::string errDescription;
		LoggerPacket();
		LoggerPacket(const void *buffer, size_t size);
		virtual ~LoggerPacket();
		LOGGER_PACKET_TYPE setBinary(const void *buffer, size_t size);
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
