#define strend(x)	&x[strlen(x)]

#ifndef fprintf_rl
#define fprintf_rl fprintf
#endif

#define VALFMT_DEFAULT	"%.2f"
#define VALFMT_NEAR_INT	"%.0f"

extern char *sprint_large_num_simple(char *string, double number);
extern char *sprint_fractional_12(char *string, double v);
extern char *sprint_compile_date(char *string, const char *location);
extern void fprint_indent(FILE *file, char *indent, int ind_lvl, char *string);
extern char *sprint_duration(char *string, double sec);
extern char *text_to_multiline_c_literal(const char *text);
