extern FILE *fopen_utf8(const char *filename, const char *mode);
extern uint8_t *load_raw_file(const char *path, size_t *size);
extern uint8_t *load_raw_file_dos_conv(const char *path, size_t *size);
extern int save_string_to_file(const char *path, const char *mode, char *string);
extern int32_t count_linebreaks(FILE *file);
extern int check_file_is_readable(char *path);

#ifdef _WIN32
#define utf8_to_wchar(utf8, wchar)	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wchar, sizeof(wchar))
#endif
