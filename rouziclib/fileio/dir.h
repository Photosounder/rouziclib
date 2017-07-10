// in fileio/dir_struct.h:
// DIR_CHAR, fs_file_t, fs_dir_t

extern int dir_count(char *path, int *subdir_count, int *subfile_count);
extern void load_dir(char *path, fs_dir_t *dir);
extern void load_dir_depth(char *path, fs_dir_t *dir, int max_depth);
extern void print_dir_depth(fs_dir_t *dir, int current_depth);
extern void free_dir(fs_dir_t *dir);
extern int dirent_test(char *path);
