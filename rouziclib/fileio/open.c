#ifdef _WIN32
FILE *fopen_utf8(const char *filename, const char *mode)
{
	wchar_t wfilename[MAX_PATH], wmode[8];
	FILE *file;

	if (MultiByteToWideChar(CP_UTF8, 0, filename, -1, wfilename, MAX_PATH)==0)
		return NULL;

	if (MultiByteToWideChar(CP_UTF8, 0, mode, -1, wmode, 8)==0)
		return NULL;

	file = _wfopen(wfilename, wmode);
	
	return file;
}
#else
FILE *fopen_utf8(const char *filename, const char *mode)
{
	return fopen(filename, mode);
}
#endif

uint8_t *load_raw_file(const char *path, int32_t *size)	// path is UTF-8
{
	FILE *in_file;
	uint8_t *data;
	int fsize;
	
	in_file = fopen_utf8(path, "rb");

	if (in_file==NULL)
	{
		fprintf_rl(stderr, "File '%s' not found.\n", path);
		return NULL;
	}

	fseek(in_file, 0, SEEK_END);
	fsize = ftell(in_file);

	data = calloc (fsize+1, sizeof(uint8_t));
	rewind (in_file);
	fread(data, 1, fsize, in_file);

	fclose(in_file);

	if (size)
		*size = fsize;

	return data;	
}

int32_t count_linebreaks(FILE *file)
{
	int32_t i=0;
	uint8_t b;

	while (fread(&b, 1, 1, file))
		if (b=='\n')
			i++;

	rewind(file);

	return i;
}
