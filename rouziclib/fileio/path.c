char *remove_name_from_path(char *dirpath, char *fullpath)	// removes the file or dir name after DIR_CHAR and DIR_CHAR itself
{
	char *p;
	int len;

	if (fullpath==NULL)
	{
		fprintf_rl(stderr, "fullpath is NULL in remove_name_from_path()\n");
		return NULL;
	}

	p = strrchr(fullpath, DIR_CHAR);	// look for DIR_CHAR
	if (p == NULL)
	{
		if (dirpath)
			dirpath[0] = '\0';
		return NULL;
	}

	len = p - fullpath;

	if (dirpath==NULL)
		dirpath = calloc(len + 1, sizeof(char));

	memcpy(dirpath, fullpath, len);
	dirpath[len] = 0;

	return dirpath;
}

char *get_filename_from_path(char *fullpath)	// returns a pointer to the filename in the path
{
	char *p;

	if (fullpath==NULL)
	{
		fprintf_rl(stderr, "fullpath is NULL in get_filename_from_path()\n");
		return NULL;
	}

	p = strrchr(fullpath, DIR_CHAR);	// look for DIR_CHAR
	if (p == NULL)
		return NULL;
	else
		return &p[1];
}

char *append_name_to_path(char *dest, char *path, char *name)	// appends name to path properly regardless of how path is ended
{
	int path_len, name_len, path_has_dirchar=0;

	path_len = strlen(path);
	name_len = strlen(name);

	if (path_len > 0)
		if (path[path_len-1] == DIR_CHAR)
			path_has_dirchar = 1;

	if (dest==NULL)
		dest = calloc(path_len + name_len + 2, sizeof(char));
	
	if (dest==path)		// in-place appending
	{
		if (path_has_dirchar)
			sprintf(&path[path_len], "%s", name);
		else
			sprintf(&path[path_len], "%c%s", DIR_CHAR, name);
	}
	else
	{
		if (path_has_dirchar)
			sprintf(dest, "%s%s", path, name);
		else
			sprintf(dest, "%s%c%s", path, DIR_CHAR, name);
	}

	return dest;
}

char *extract_file_extension(char *path, char *ext)
{
	int len, i, j;

	len = strlen(path);
	i = len - 1;				// start from the end of the string

	if (i <= 0)
		return ;

	while (path[i] != '.' && i > 0)		// decrement until a . or the beginning
		i--;

	if (path[i] == '.')
	{
		if (ext==NULL)
			ext = calloc(len-(i+1)+1, sizeof(char));

		strcpy(ext, &path[i+1]);	// copy extension
	}
	else
	{
		if (ext==NULL)
			ext = calloc(1, sizeof(char));

		ext[0] = '\0';
	}

	string_tolower(ext);

	return ext;
}

#ifdef __APPLE__
#include <glob.h>
#endif

char *make_appdata_path(const char *dirname, const char *filename, const int make_subdir)
{
	char *path=NULL;

	#ifdef _WIN32
	wchar_t origpath_w[PATH_MAX];
	char origpath[PATH_MAX*4];

	SHGetFolderPathW(NULL, 0x001a /*CSIDL_APPDATA*/, NULL, 0, origpath_w);
	//SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, origpath_w);
	wchar_to_utf8(origpath_w, origpath);
	#endif

	#ifdef __APPLE__
	glob_t globbuf;
	char *origpath=NULL;

	if (glob("~/Library/Application Support/", GLOB_TILDE | GLOB_MARK, NULL, &globbuf)==0)	// globbuf.gl_pathv[0] is the path
		origpath = globbuf.gl_pathv[0];
	else
	{
		globfree(&globbuf);
		return NULL;
	}
	#endif

	path = calloc (strlen(origpath) + strlen(dirname)+1 + (filename ? strlen(filename) : 0) + 1, sizeof(char));

	append_name_to_path(path, origpath, dirname);

	if (make_subdir)
		create_dir(path);

	if (filename)
		append_name_to_path(path, path, filename);

	#ifdef __APPLE__
	globfree(&globbuf);
	#endif

	return path;
}
