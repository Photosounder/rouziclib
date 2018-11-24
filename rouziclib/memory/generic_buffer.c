char *bufprintf(buffer_t *s, const char *format, ...)	// similar in the way it's used to fprintf except for a buffer_t
{
	va_list args;
	int len1;

	if (s==NULL)
		return NULL;

	va_start(args, format);
	len1 = vsnprintf(NULL, 0, format, args);	// gets the printed length without actually printing
	va_end(args);

	alloc_enough(&s->buf, s->len + len1 + 1, &s->as, sizeof(char), s->as==0 ? 1. : 1.5);

	va_start(args, format);
	vsnprintf(&s->buf[s->len], s->as - s->len, format, args);
	va_end(args);

	s->len += len1;

	return s->buf;
}

char *bufnprintf(buffer_t *s, size_t n, const char *format, ...)	// similar in the way it's used to fprintf except for a buffer_t and only n chars like snprintf
{
	va_list args;
	int len1;

	if (s==NULL)
		return NULL;

	va_start(args, format);
	len1 = vsnprintf(NULL, 0, format, args);	// gets the printed length without actually printing
	va_end(args);
	len1 = MINN(n, len1);				// limit the length to add to n

	alloc_enough(&s->buf, s->len + len1 + 1, &s->as, sizeof(char), s->as==0 ? 1. : 1.5);

	va_start(args, format);
	vsnprintf(&s->buf[s->len], len1+1, format, args);
	va_end(args);

	s->len += len1;

	return s->buf;
}

char *bufwrite(buffer_t *s, uint8_t *ptr, size_t size)	// similar in the way it's used to fwrite except for a buffer_t
{
	if (s==NULL)
		return NULL;

	if (ptr)
	{
		alloc_enough(&s->buf, size + s->len+1, &s->as, sizeof(char), 1.5);
		memcpy(&s->buf[s->len], ptr, size);
		s->len += size;
	}

	return s->buf;
}

void free_buf(buffer_t *s)
{
	free(s->buf);
	memset(s, 0, sizeof(buffer_t));
}

void bufprint_gmtime(buffer_t *s, time_t t)
{
	char datestamp[32];
	strftime(datestamp, 32, "%Y-%m-%d %H.%M.%S", gmtime(&t));

	bufprintf(s, "%s", datestamp);
}
