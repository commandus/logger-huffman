#include <string.h>

#include "logger-huffman.h"
#include "errlist.h"

/**
 * @param retHdr,return header
 * @param buffer data
 * @param size buffer size
 */
int exractMeasurementHeader(
	LOGGER_MEASUREMENT_HDR **retHdr,
	const void *buffer,
	size_t bufferSize
)
{
	if (!retHdr)
		return ERR_LOGGER_HUFFMAN_PARAM_INVALID;
	if (!buffer)
		return ERR_LOGGER_HUFFMAN_PARAM_INVALID;
	if (bufferSize < sizeof(LOGGER_MEASUREMENT_HDR))
		return ERR_LOGGER_HUFFMAN_INVALID_PACKET;
	*retHdr = (LOGGER_MEASUREMENT_HDR*) buffer;
	
	return 0;
}
