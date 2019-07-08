#ifdef WIN32
#define snprintf _snprintf	// Microsoft is stupid, like seriously
#endif

extern char *skip_string(const char *string, const char *skipstring);
extern char *skip_whitespace(const char *string);
extern int string_get_field(char *string, char *delim, int n, char *field);
extern char *string_parse_fractional_12(const char *string, double *v);
extern double doztof(const char *string);
extern xy_t doztof_xy(const char *str_x, const char *str_y);
extern char *remove_after_char(char *string, char c);
extern int strlen_until_after_char(char *string, char c);
extern char *remove_after_char_copy(char *string, char c);
extern int get_string_linecount(char *text, int len);
extern int string_find_start_nth_line(char *text, int len, int n);
extern char **arrayise_text(char *text, int *linecount);
extern char *strstr_i (char *fullstr, char *substr);
#ifdef _WIN32
extern void *memmem(const uint8_t *l, size_t l_len, const uint8_t *s, size_t s_len);
#endif
extern int compare_varlen_word_to_fixlen_word(const char *var, size_t varlen, const char *fix);
extern char *find_pattern_in_string(const char *str, const char *pat);
extern char *find_date_time_in_string(const char *str);
extern double parse_timestamp(const char *ts);
