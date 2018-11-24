extern FILE *fopen_utf8(const char *filename, const char *mode);
extern uint8_t *load_raw_file(const char *path, size_t *size);
extern uint8_t *load_raw_file_dos_conv(const char *path, size_t *size);
extern int save_raw_file(const char *path, const char *mode, uint8_t *data, size_t data_size);
extern int save_string_to_file(const char *path, const char *mode, char *string);
extern int32_t count_linebreaks(FILE *file);
extern int check_file_is_readable(char *path);
extern int check_dir_exists(char *path);
