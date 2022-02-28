#ifndef LOGGER_HUFFMAN_IMPL_H_
#define LOGGER_HUFFMAN_IMPL_H_ 1

#include <string>
#include "logger-huffman.h"

std::string LOGGER_PACKET_TYPE_2_string(const LOGGER_PACKET_TYPE &value);
LOGGER_PACKET_TYPE LOGGER_PACKET_TYPE_2_string(const std::string &value);

std::string LOGGER_MEASUREMENT_HDR_2_string(const LOGGER_MEASUREMENT_HDR &value);

#endif
