extern char *remove_name_from_path(char *dirpath, char *fullpath);
extern char *get_filename_from_path(char *fullpath);
extern char *append_name_to_path(char *dest, char *path, char *name);
extern char *extract_file_extension(char *path, char *ext);
extern char *make_appdata_path(const char *dirname, const char *filename, const int make_subdir);
