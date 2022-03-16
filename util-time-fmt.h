#ifndef UTIL_TIME_FMT_H_
#define UTIL_TIME_FMT_H_	1

#include <string>
#include <inttypes.h>

#ifdef _MSC_VER
// #define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdint.h> // portable: uint64_t   MSVC: __int64 
#endif

/**
 * Parse NULL-terminated timstamp string
 * @return  Unix epoch time (seconds) or 2015-11-25T23:59:11
 */
time_t string2time(const char *v, size_t size, bool localtime);

time_t logger2time
(
	uint8_t year2000,
	uint8_t month,
	uint8_t date,
	uint8_t hours,
	uint8_t minutes,
	uint8_t seconds,
	bool isLocaltime
);

/**
 * Format date and time
 * @return timestamp string
 */
std::string time2string(time_t val, bool localtime);

#endif
