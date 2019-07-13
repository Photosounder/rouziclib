#ifdef _WIN32
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

#ifdef __APPLE__
#include <mach/mach_time.h>
#endif

double get_time_hr()	// High-resolution timing
{
	static double tick_dur = 0.;

	// Find the tick duration in seconds only once
	if (tick_dur==0.)
	{
		#ifdef _WIN32
		LARGE_INTEGER rate;
		QueryPerformanceFrequency(&rate);
		tick_dur = 1. / (double) rate.QuadPart;

		#elif __APPLE__
		mach_timebase_info_data_t rate_nsec;
		mach_timebase_info(&rate_nsec);
		tick_dur = 1e-9 * (double) rate_nsec.numer / (double) rate_nsec.denom;

		#else
		struct timespec rate;
		clock_getres(CLOCKID, &rate);
		tick_dur = 1e-9 * (double) rate.tv_nsec;

		#endif
	}

	#ifdef _WIN32
	LARGE_INTEGER now;
	QueryPerformanceCounter(&now);
	return (double) now.QuadPart * tick_dur;

	#elif __APPLE__
	return (double) mach_absolute_time() * tick_dur;

	#else
	struct timespec now;
	clock_gettime(CLOCKID, &now);
	return (double) ((uint64_t) time.tv_sec*1000000000LL + time.tv_nsec) * tick_dur;
	#endif
}

// the caller should give a pointer to the old time value for it to be replaced with the new value, and the difference is returned
int32_t get_time_diff(uint32_t *t)
{
	uint32_t now, diff;

	now = get_time_ms();
	diff = now - *t;
	*t = now;

	return diff;
}

double get_time_diff_hr(double *t)
{
	double now, diff;

	now = get_time_hr();
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
	struct tm ts={0};

	if (string==NULL)
		return 0;

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

#ifdef _WIN32
typedef NTSTATUS (NTAPI *NtDelayExecution_func)(BOOLEAN Alertable, PLARGE_INTEGER DelayInterval);
NtDelayExecution_func NtDelayExecution;
typedef NTSTATUS (NTAPI *ZwSetTimerResolution_func)(IN ULONG RequestedResolution, IN BOOLEAN Set, OUT PULONG ActualResolution);
ZwSetTimerResolution_func ZwSetTimerResolution;
#endif

void sleep_hr(double t)
{
	#ifdef _WIN32
	static int init=1;

	if (init)
	{
		NtDelayExecution = (NtDelayExecution_func) GetProcAddress(GetModuleHandle("ntdll.dll"), "NtDelayExecution");
		ZwSetTimerResolution = (ZwSetTimerResolution_func) GetProcAddress(GetModuleHandle("ntdll.dll"), "ZwSetTimerResolution");
		ULONG actualResolution;
		ZwSetTimerResolution(1, TRUE, &actualResolution);
		init = 0;
	}

	LARGE_INTEGER interval;
	interval.QuadPart = -1e7 * t;
	NtDelayExecution(FALSE, &interval);

	#else
	struct timespec t;

	t.tv_sec  = t;
	t.tv_nsec = (t - (double) t.tv_sec) * 1e9;

	nanosleep(&t, NULL);
	#endif
}
