#include <time.h>

#ifdef _WIN32
#define timegm _mkgmtime
#endif

extern uint32_t get_time_ms();
extern double get_time_hr();
extern int32_t get_time_diff(uint32_t *t);
extern double get_time_diff_hr(double *t);
extern double convert_time_to_jd(time_t t);
extern double get_time_day_fraction(time_t t, int gmt);
extern time_t parse_date_time_string(const char *string);
extern void sleep_ms(int ms);
