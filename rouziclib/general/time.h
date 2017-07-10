#include <time.h>

extern uint32_t get_time_ms();
extern int32_t get_time_diff(uint32_t *t);
extern double convert_time_to_jd(time_t t);
extern double get_time_day_fraction(time_t t, int gmt);
