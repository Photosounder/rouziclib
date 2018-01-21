#ifndef _WIN32
#include <sys/stat.h>
#endif

int create_symlink(const char *oldname, const char *newname, const int is_dir)	// returns 0 if successful
{
	#ifdef _WIN32
	wchar_t oldname_wc[PATH_MAX*4], newname_wc[PATH_MAX*4];

	utf8_to_wchar(oldname, oldname_wc);
	utf8_to_wchar(newname, newname_wc);
	return CreateSymbolicLinkW(newname_wc, oldname_wc, is_dir) == 0;

	#else

	return symlink(oldname, newname);
	#endif
}

int create_dir(const char *path)	// returns 0 if successful
{
	#ifdef _WIN32
	wchar_t wpath[PATH_MAX*4];

	utf8_to_wchar(path, wpath);
	return CreateDirectoryW(wpath, NULL) == 0;

	#else

	return mkdir(path, 0755);
	#endif
}

int move_file(const char *path, const char *newpath)	// returns 0 if successful
{
	#ifdef _WIN32
	wchar_t wpath[PATH_MAX*4], wnewpath[PATH_MAX*4];

	utf8_to_wchar(path, wpath);
	utf8_to_wchar(newpath, wnewpath);
	return MoveFileW(wpath, wnewpath)==0;

	#else

	return rename(path, newpath);
	#endif
}

int remove_file(const char *path)	// returns 0 if successful
{
	#ifdef _WIN32
	wchar_t wpath[PATH_MAX*4];

	utf8_to_wchar(path, wpath);
	return _wremove(wpath);

	#else

	return remove(path);
	#endif
}

int remove_every_file(const char *path)		// returns 0 if successful on every single file
{
	int i, ret;
	fs_dir_t dir;
	char ffp[PATH_MAX*4];

	memset(&dir, 0, sizeof(fs_dir_t));

	load_dir_depth(path, &dir, 0);

	for (i=0; i<dir.subfile_count; i++)
		if (dir.subfile[i].name)
			ret |= remove_file(append_name_to_path(ffp, path, dir.subfile[i].name));

	free_dir(&dir);

	return ret;
}

void system_open(const char *path)
{
	#ifdef _WIN32
	wchar_t path_w[PATH_MAX];

	utf8_to_wchar(path, path_w);
	ShellExecuteW(NULL, L"open", path_w, NULL, NULL, SW_SHOWNORMAL);
	#endif

	#ifdef __APPLE__
	char command[PATH_MAX+5];

	sprintf(command, "open \"%s\"", path);
	system(command);

	/*NSURL *fileURL = [NSURL fileURLWithPath: [NSString stringWithUTF8String:path]];
	
	NSWorkspace *ws = [NSWorkspace sharedWorkspace];
	[ws openURL: fileURL];*/
	#endif
}
