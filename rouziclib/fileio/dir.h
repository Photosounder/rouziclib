// in fileio/dir_struct.h:
// DIR_CHAR, fs_file_t, fs_dir_t

extern int dir_count(char *path, int *subdir_count, int *subfile_count);
extern void load_dir(char *path, fs_dir_t *dir);
extern void load_dir_depth(char *path, fs_dir_t *dir, int max_depth);
extern void print_dir_depth(fs_dir_t *dir, int current_depth);
extern char *sprint_dir_depth_fullarg(fs_dir_t *dir, int current_depth, char **string, int *alloc_count);
extern void free_dir(fs_dir_t *dir);
extern void export_subfiles_to_file(FILE *file, fs_dir_t *dir, const int indent);
extern int export_subfiles_to_path(char *path, fs_dir_t *dir);
extern void export_whole_dir_flat_to_file(FILE *file, fs_dir_t *dir, const int show_dirs, const int path_start);
extern int export_whole_dir_flat_to_path(char *path, fs_dir_t *dir, const int show_dirs, const int remove_path);
extern int dirent_test(char *path);
extern int64_t get_volume_free_space(char *path);
extern double get_volume_free_space_gb(char *path);

#define sprint_dir_depth(dir, current_depth)	sprint_dir_depth_fullarg(dir, current_depth, NULL, NULL)
