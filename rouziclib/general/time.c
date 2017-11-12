#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
	#include <mmsystem.h>
	#pragma comment(lib, "winmm.lib")

	uint32_t get_time_ms()
	{
		return timeGetTime();
	}

#else
	#include <sys/types.h> 
	#include <sys/time.h>
	
	uint32_t get_time_ms()
	{
		struct timeval now;
	
		gettimeofday(&now, NULL);
	
		return now.tv_sec * 1000 + now.tv_usec / 1000;
	}
#endif

// the caller should give a pointer to the old time value for it to be replaced with the new value, and the difference is returned
int32_t get_time_diff(uint32_t *t)
{
	uint32_t now, diff;

	now = get_time_ms();
	diff = now - *t;
	*t = now;

	return diff;
}

double convert_time_to_jd(time_t t)
{
	// reference time is January 1st, 2017 at 00:00:00 UTC, date of the latest leap second (see https://hpiers.obspm.fr/iers/bul/bulc/Leap_Second.dat)
	const double ref_jd = 2400000.5 + 57754.0;	// Julian date for the reference time
	const time_t ref_ut = 1483228800;		// Unix time for the reference time

	#ifdef _WIN32
	if (t >= 32535200000)
		t %= 32535200000;	// Windows difftime() has a year 3001 bug
	#endif

	double dts = difftime(t, ref_ut);		// time in seconds since reference

	return ref_jd + dts / 86400.;			// Julian date for t
}

double get_time_day_fraction(time_t t, int gmt)
{
	struct tm *ts;

	#ifdef _WIN32
	if (t >= 32535200000)
		t %= 32535200000;	// Windows difftime() has a year 3001 bug
	#endif

	if (gmt)
		ts = gmtime(&t);
	else
		ts = localtime(&t);
	return (ts->tm_hour + (ts->tm_min + ts->tm_sec/60.)/60.) / 24.;
}

time_t parse_date_time_string(const char *string)	// expected format is "YYYY-MM-DD hh.mm.ss"
{
	struct tm ts;

	memset(&ts, 0, sizeof(ts));

	sscanf(string, "%d-%d-%d %d.%d.%d", &ts.tm_year, &ts.tm_mon, &ts.tm_mday, &ts.tm_hour, &ts.tm_min, &ts.tm_sec);

	ts.tm_year -= 1900;
	ts.tm_mon -= 1;

	return timegm(&ts);
}

void sleep_ms(int ms)
{
	#ifdef _WIN32
	Sleep(ms);
	#else
	struct timespec t;

	t.tv_sec  = ms / 1000;
	t.tv_nsec = (ms - t.tv_sec) * 1000000;

	nanosleep(&t, NULL);
	#endif
}
