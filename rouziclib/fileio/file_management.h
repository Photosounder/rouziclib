extern int create_symlink(const char *oldname, const char *newname, const int is_dir);
extern int create_dir(const char *path);
extern int move_file(const char *path, const char *newpath);
extern int copy_file(const char *path, const char *newpath, const int overwrite);
extern int remove_file(const char *path);
extern int remove_empty_dir(const char *path);
extern int remove_every_file(const char *path);
extern int remove_dir(const char *path);
extern void system_open(const char *path);
