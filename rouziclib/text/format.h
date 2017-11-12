#define strend(x)	&x[strlen(x)]

#ifndef fprintf_rl
#define fprintf_rl fprintf
#endif

enum
{
	VALFMT_DEFAULT,
	VALFMT_3F,
	VALFMT_PCT_2F,
	VALFMT_COUNT
};

extern char *sprint_large_num_simple(char *string, double number);
extern char *sprint_fractional_12(char *string, double v);
extern char *sprint_compile_date(char *string, const char *location);
extern void print_valfmt(char *str, int str_size, double v, const int valfmt);
extern void fprint_indent(FILE *file, char *indent, int ind_lvl, char *string);
extern char *sprint_duration(char *string, double sec);
