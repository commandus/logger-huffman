#include "util-time-fmt.h"

#include <time.h>
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
