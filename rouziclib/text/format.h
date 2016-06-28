#define strend(x)	&x[strlen(x)]

#ifndef fprintf_rl
#define fprintf_rl fprintf
#endif

extern char *sprint_large_num_simple(char *string, double number);
extern char *sprint_fractional_12(char *string, double v);
