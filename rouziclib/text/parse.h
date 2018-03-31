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
extern char **arrayise_text(char *text, int *linecount);
extern char *strstr_i (char *fullstr, char *substr);
extern char *bstrchr(const char *s, int c, int l);
extern char *bstrstr(const char *s1, int l1, const char *s2, int l2);
extern int compare_varlen_word_to_fixlen_word(const char *var, size_t varlen, const char *fix);
