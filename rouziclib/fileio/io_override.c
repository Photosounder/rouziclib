_Thread_local fread_func_t fread_override = (fread_func_t) fread;
_Thread_local fwrite_func_t fwrite_override = (fwrite_func_t) fwrite;
_Thread_local fprintf_func_t fprintf_override = (fprintf_func_t) fprintf;
_Thread_local fopen_func_t fopen_override = (fopen_func_t) fopen_utf8;
_Thread_local fclose_func_t fclose_override = (fclose_func_t) fclose;

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

void io_override_set_FILE()
{
	// Set the function points to those of the FILE type
	fread_override  = (fread_func_t)  fread;
	fwrite_override = (fwrite_func_t) fwrite;
	fprintf_override = (fprintf_func_t) fprintf;
	fopen_override  = (fopen_func_t)  fopen_utf8;
	fclose_override = (fclose_func_t) fclose;
}

void io_override_set_buffer()
{
	// Set the function points to those of the generic buffer type
	fread_override  = fread_buffer;
	fwrite_override = fwrite_buffer;
	fprintf_override = (fprintf_func_t) bufprintf;
	fopen_override  = fopen_buffer;
	fclose_override = fclose_buffer;
}
