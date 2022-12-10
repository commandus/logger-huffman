#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "util-time-fmt.h"

#include <ctime>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <iomanip>

#ifdef _MSC_VER
#include "strptime.h"
int timezone = 0;
#else
#include <sys/time.h>
#define TMSIZE sizeof(struct tm)
#define localtime_s(tm, time) memmove(tm, localtime(time), TMSIZE)
#endif

const static char *dateformat0 = "%FT%T";
const static char *dateformat1 = "%FT%T%Z";

std::string time2unixepochstring(time_t val)
{
	std::stringstream ss;
	ss << val;
	return ss.str();
}

/**
 * Format date and time
 * @return timestamp string
 */
std::string time2string(
	time_t val,
	bool isLocaltime
)
{
	if (!val)
		val = time(NULL);
	struct tm tm;
	struct tm *ptm;
	if (isLocaltime) {
		localtime_s(&tm, &val);
		ptm = &tm;
	} else {
		// gmtime_s(&tm, &val);
		ptm = gmtime(&val);
	}
	char dt[64];
	strftime(dt, sizeof(dt), dateformat1, ptm);
	return std::string(dt);
}

/**
 * Parse NULL-terminated timstamp string
 * @return  Unix epoch time (seconds) or 2015-11-25T23:59:11
 */
time_t string2time(
	const char *v,
	size_t size,
	bool isLocaltime
)
{
	struct tm tmd;
	memset(&tmd, 0, sizeof(struct tm));

	time_t r;
	if ((strptime(v, dateformat0, &tmd) == NULL)
		&& (strptime(v, dateformat1, &tmd) == NULL)) 
	{
		r = strtol(v, NULL, 0);
	} else {
		r = mktime(&tmd);
		if (isLocaltime)
			r -= timezone;
	}
	return r;
}

int timevalSubtract (
    struct timeval *retVal,
    struct timeval *start,
    struct timeval *finish)
{
    /* Perform the carry for the later subtraction by updating finish. */
    if (start->tv_usec < finish->tv_usec) {
        int nsec = (finish->tv_usec - start->tv_usec) / 1000000 + 1;
        finish->tv_usec -= 1000000 * nsec;
        finish->tv_sec += nsec;
    }
    if (start->tv_usec - finish->tv_usec > 1000000) {
        int nsec = (start->tv_usec - finish->tv_usec) / 1000000;
        finish->tv_usec += 1000000 * nsec;
        finish->tv_sec -= nsec;
    }

    /* Compute the time remaining to wait. tv_usec is certainly positive. */
    retVal->tv_sec = start->tv_sec - finish->tv_sec;
    retVal->tv_usec = start->tv_usec - finish->tv_usec;

    /* Return 1 if retVal is negative. */
    return start->tv_sec < finish->tv_sec;
}
