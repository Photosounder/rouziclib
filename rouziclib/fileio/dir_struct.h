#ifdef _WIN32
#include "../libraries/dirent.h"
#define DIR_CHAR '\\'
#else
#include <dirent.h>
#define DIR_CHAR '/'
#endif

typedef struct fs_file_n
{
	char *name;
	struct fs_dir_n *parent;
} fs_file_t;

typedef struct fs_dir_n
{
	char *name, *path;
	int subdir_count, subfile_count;
	struct fs_dir_n *subdir, *parent;
	struct fs_file_n *subfile;
} fs_dir_t;
