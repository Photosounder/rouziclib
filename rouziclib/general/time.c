#ifdef _WIN32
//#include <mmsystem.h>
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
