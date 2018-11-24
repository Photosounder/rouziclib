extern char *make_string_copy(const char *orig);
extern void strcpy_then_free(char *dest, char *src);
extern char *replace_char(char *str, char find, char replace);
extern char *string_tolower(char *str);
extern char *sprintf_realloc(char **string, int *alloc_count, const int append, const char *format, ...);
extern char *vsprintf_alloc(const char *format, va_list args);
