#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

uint32_t get_time_ms()
{
	return timeGetTime();
}
#endif

#ifdef __APPLE__
#include <sys/types.h> 

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
	// reference time is July 1st, 2015 at 00:00:00 UTC, date of the latest leap second (see https://hpiers.obspm.fr/iers/bul/bulc/Leap_Second.dat)
	const double ref_jd = 2400000.5 + 57204.0;	// Julian date for the reference time
	const time_t ref_ut = 1435708800;		// Unix time for the reference time

	double dts = difftime(t, ref_ut);		// time in seconds since reference

	return ref_jd + dts / 86400.;			// Julian date for t
}
