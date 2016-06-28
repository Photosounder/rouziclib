#ifdef WIN32
#define snprintf _snprintf	// Microsoft is stupid, like seriously
#endif

extern char *skip_string(char *string, const char *skipstring);
extern char *skip_whitespace(char *string);
extern int string_get_field(char *string, char *delim, int n, char *field);
extern char *string_parse_fractional_12(char *string, double *v);
extern char *remove_after_char(char *string, char c);
extern int strlen_until_after_char(char *string, char c);
extern char *remove_after_char_copy(char *string, char c);
