_Thread_local fread_func_t fread_override = (fread_func_t) fread;
_Thread_local fwrite_func_t fwrite_override = (fwrite_func_t) fwrite;
_Thread_local fprintf_func_t fprintf_override = (fprintf_func_t) fprintf;
_Thread_local fopen_func_t fopen_override = (fopen_func_t) fopen_utf8;
_Thread_local fclose_func_t fclose_override = (fclose_func_t) fclose;
_Thread_local rewind_func_t rewind_override = (rewind_func_t) rewind;
_Thread_local fseek_func_t fseek_override = (fseek_func_t) fseek;
_Thread_local ftell_func_t ftell_override = (ftell_func_t) ftell;

size_t fread_buffer(void *ptr, size_t size, size_t nmemb, void *stream)
{
	buffer_t *s = stream;
	size_t readable_bytes = MINN(s->len - s->read_pos, size*nmemb);

	memcpy(ptr, &s->buf[s->read_pos], readable_bytes);
	s->read_pos += readable_bytes;

	return readable_bytes;
}

size_t fwrite_buffer(const void *ptr, size_t size, size_t nmemb, void *stream)
{
	bufwrite((buffer_t *) stream, (uint8_t *) ptr, size*nmemb);
	return nmemb;
}

void *fopen_buffer(const char *path, const char *mode)	// only works for "rb" and "wb" modes
{
	buffer_t *s;

	if (strcmp(mode, "rb") != 0 && strcmp(mode, "wb") != 0)
	{
		fprintf_rl(stderr, "Error calling fopen_buffer() with mode \"%s\" instead of \"rb\" or \"wb\"\n", mode);
		return NULL;
	}

	// Alloc struct
	s = calloc(1, sizeof(buffer_t));

	// Either load file or flag write mode
	if (strcmp(mode, "rb") == 0)
		*s = buf_load_raw_file(path);
	else
		s->write_dest = make_string_copy(path);

	return s;
}

int fclose_buffer(void *stream)
{
	int ret = 0;
	buffer_t *s = (buffer_t *) stream;

	// Save the buffer to file if in write mode
	if (s->write_dest)
	{
		ret = buf_save_raw_file(s, s->write_dest, "wb") == 0;
		free(s->write_dest);
	}

	free_buf(s);
	free(stream);

	return ret;
}

void rewind_buffer(void *stream)
{
	buffer_t *s = (buffer_t *) stream;
	s->read_pos = 0;
}

int fseek_buffer(void *stream, long int offset, int whence)
{
	buffer_t *s = (buffer_t *) stream;

	switch (whence)
	{
			case SEEK_SET: s->read_pos = offset;
		break;	case SEEK_CUR: s->read_pos += offset;
		break;	case SEEK_END: s->read_pos = s->len + offset;
	}

	if (s->read_pos < 0 || s->read_pos > s->len)
	{
		s->read_pos = 0;
		return -1;
	}

	return 0;
}

long int ftell_buffer(void *stream)
{
	return s->read_pos;
}

void io_override_set_FILE()
{
	// Set the function points to those of the FILE type
	fread_override  = (fread_func_t)  fread;
	fwrite_override = (fwrite_func_t) fwrite;
	fprintf_override = (fprintf_func_t) fprintf;
	fopen_override  = (fopen_func_t)  fopen_utf8;
	fclose_override = (fclose_func_t) fclose;
	rewind_override = (rewind_func_t) rewind;
	fseek_override = (fseek_func_t) fseek;
	ftell_override = (ftell_func_t) ftell;
}

void io_override_set_buffer()
{
	// Set the function points to those of the generic buffer type
	fread_override  = fread_buffer;
	fwrite_override = fwrite_buffer;
	fprintf_override = (fprintf_func_t) bufprintf;
	fopen_override  = fopen_buffer;
	fclose_override = fclose_buffer;
	rewind_override = rewind_buffer;
	fseek_override = fseek_buffer;
	ftell_override = ftell_buffer;
}
