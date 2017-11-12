extern char *make_string_copy(const char *orig);
extern char *replace_char(char *str, char find, char replace);
extern char *string_tolower(char *str);
extern char *sprintf_realloc(char **string, int *alloc_count, const int append, const char *format, ...);
