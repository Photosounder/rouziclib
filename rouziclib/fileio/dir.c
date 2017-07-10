int dir_count(char *path, int *subdir_count, int *subfile_count)
{
	DIR *dirp;
	struct dirent *entry;

	*subdir_count = 0;
	*subfile_count = 0;

	dirp = opendir(path);
	if (dirp == NULL)
		return -1;

	while ((entry = readdir(dirp)))
	{
		if (entry->d_type == DT_DIR)
			(*subdir_count)++;

		if (entry->d_type == DT_REG)
			(*subfile_count)++;
	}

	closedir(dirp);

	return *subdir_count + *subfile_count;
}

void load_dir(char *path, fs_dir_t *dir)
{
	DIR *dirp;
	struct dirent *entry;
	int i_dir=0, i_file=0;

	if (dir->subdir)  free (dir->subdir);	dir->subdir = NULL;
	if (dir->subfile) free (dir->subfile);	dir->subfile = NULL;

	if (dir_count(path, &dir->subdir_count, &dir->subfile_count) < 0)
		return ;

	dir->subdir = calloc(dir->subdir_count, sizeof(fs_dir_t));
	dir->subfile = calloc(dir->subfile_count, sizeof(fs_file_t));

	dirp = opendir(path);
	if (dirp == NULL)
		return ;

	dir->path = calloc(strlen(path)+1, sizeof(char));
	strcpy(dir->path, path);

	while ((entry = readdir(dirp)))
	{
		if (entry->d_type == DT_DIR && i_dir < dir->subdir_count)
		{
			dir->subdir[i_dir].name = calloc(strlen(entry->d_name)+1, sizeof(char));
			strcpy(dir->subdir[i_dir].name, entry->d_name);
			dir->subdir[i_dir].parent = dir;
			i_dir++;
		}

		if (entry->d_type == DT_REG && i_file < dir->subfile_count)
		{
			dir->subfile[i_file].name = calloc(strlen(entry->d_name)+1, sizeof(char));
			strcpy(dir->subfile[i_file].name, entry->d_name);
			dir->subfile[i_file].parent = dir;
			i_file++;
		}
	}

	closedir(dirp);
}

void load_dir_depth(char *path, fs_dir_t *dir, int max_depth)	// a max_depth of -1 means infinite
{
	int i;
	char subdir_path[PATH_MAX*4];

	load_dir(path, dir);

	if (max_depth)
	for (i=0; i<dir->subdir_count; i++)
	{
		if (strcmp(dir->subdir[i].name, ".") && strcmp(dir->subdir[i].name, ".."))
		{
			//fprintf_rl(stdout, "[%s]\n", dir->subdir[i]);
			sprintf(subdir_path, "%s%c%s", path, DIR_CHAR, dir->subdir[i].name);
			load_dir_depth(subdir_path, &dir->subdir[i], max_depth-1);
		}
	}
}

void print_dir_depth(fs_dir_t *dir, int current_depth)
{
	int i, j;

	for (i=0; i<dir->subdir_count; i++)
	{
		if (strcmp(dir->subdir[i].name, ".") && strcmp(dir->subdir[i].name, ".."))
		{
			for (j=0; j<current_depth; j++)
				fprintf_rl(stdout, "    ");
			fprintf_rl(stdout, "[%s]\n", dir->subdir[i].name);

			print_dir_depth(&dir->subdir[i], current_depth+1);
		}
	}

	for (i=0; i<dir->subfile_count; i++)
	{
		for (j=0; j<current_depth; j++)
			fprintf_rl(stdout, "    ");
		fprintf_rl(stdout, "%s\n", dir->subfile[i].name);
	}
}

void free_dir(fs_dir_t *dir)
{
	int i;

	for (i=0; i<dir->subdir_count; i++)
	{
		if (strcmp(dir->subdir[i].name, ".") && strcmp(dir->subdir[i].name, ".."))
		{
			free_dir(&dir->subdir[i]);
		}
		else if (dir->subdir[i].name)
			free (dir->subdir[i].name);
	}

	for (i=0; i<dir->subfile_count; i++)
		if (dir->subfile[i].name)
			free (dir->subfile[i].name);

	if (dir->name)  free (dir->name);	dir->name = NULL;
	if (dir->subdir)  free (dir->subdir);	dir->subdir = NULL;
	if (dir->subfile) free (dir->subfile);	dir->subfile = NULL;
}

int dirent_test(char *path)
{
	int i;
	fs_dir_t dir;

	memset(&dir, 0, sizeof(fs_dir_t));

	load_dir_depth(path, &dir, 2);
	print_dir_depth(&dir, 0);
	free_dir(&dir);

	return 0;
}
