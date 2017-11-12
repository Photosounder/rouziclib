void alloc_enough(void **buffer, int needed_count, int *alloc_count, int size_elem, double inc_ratio)	// increases a buffer's size to accomodate for the requested count if necessary
{
	int newsize;
	void *p;

	if (needed_count >= *alloc_count)
	{
		newsize = ceil((double) needed_count * inc_ratio);

		p = realloc(*buffer, newsize * size_elem);
		if (p == NULL)
		{
			fprintf_rl(stderr, "realloc(*buffer=0x%08x, size=%d) failed.\n", *buffer, newsize * size_elem);
			return ;
		}
		else
			*buffer = p;

		memset(&((uint8_t *)(*buffer))[*alloc_count * size_elem], 0, (newsize-*alloc_count) * size_elem);

		*alloc_count = newsize;

		if (*buffer==NULL)
			fprintf_rl(stderr, "realloc failed in alloc_enough() for *alloc_count = %d\n", *alloc_count);
	}
}

void free_2d(void **ptr, const int count)
{
	int i;

	if (ptr==NULL)
		return ;

	for (i=0; i < count; i++)
		free(ptr[i]);

	free(ptr);
}

void *copy_alloc(void *b0, size_t size)		// makes an allocated copy of a buffer
{
	void *b1;

	if (b0==NULL || size <= 0)
		return NULL;

	b1 = malloc(size);
	memcpy(b1, b0, size);

	return b1;
}
