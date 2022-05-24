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

/**
 * Format date and time
 * @return timestamp string
 */
std::string time2string(time_t val, bool localtime);
std::string time2unixepochstring(time_t val);

/**
 * @param retVal return time difference
 * @param start start time
 * @param finish finish time
 * @return
 * @see https://www.gnu.org/software/libc/manual/html_node/Calculating-Elapsed-Time.html
 */
int timevalSubtract(
    struct timeval *retVal,
    struct timeval *start,
    struct timeval *finish
);

#endif
