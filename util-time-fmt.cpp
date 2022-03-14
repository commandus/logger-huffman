#include "util-time-fmt.h"

#include <time.h>
#include <cstdlib>
#include <cstring>
#include <sstream>
#include <iomanip>

#ifdef _MSC_VER
// #include "strptime.h"
#else
#include <sys/time.h>
#define TMSIZE sizeof(struct tm)
#define localtime_s(tm, time) memmove(tm, localtime(time), TMSIZE)
#endif

const static char *dateformat0 = "%FT%T";
const static char *dateformat1 = "%FT%T%Z";


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
	strftime(dt, sizeof(dt), dateformat0, ptm);
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

time_t logger2time
(
	uint8_t year2000,
	uint8_t month,
	uint8_t date,
	uint8_t hours,
	uint8_t minutes,
	uint8_t seconds,
	bool isLocaltime
)
{
	struct tm m;
	m.tm_sec = seconds;			/* Seconds.	[0-60] (1 leap second) */
  	m.tm_min = minutes;			/* Minutes.	[0-59] */
  	m.tm_hour = hours;			/* Hours.	[0-23] */
  	m.tm_mday = date;			/* Day.		[1-31] */
  	m.tm_mon = month;			/* Month.	[0-11] */
  	m.tm_year = year2000 + 100;	/* Year	- 1900.  */
  	m.tm_wday = 0;				/* Day of week.	[0-6] */
  	m.tm_yday = 0;			/* Days in year.[0-365]	*/
  	m.tm_isdst = 0;			/* DST.		[-1/0/1]*/

	time_t r = mktime(&m);

	if (!isLocaltime)
		r -= timezone;
	return r;
}
