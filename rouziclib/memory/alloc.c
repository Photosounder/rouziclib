inline size_t alloc_enough2(void **buffer, size_t needed_count, size_t alloc_count, size_t size_elem, double inc_ratio)	// increases a buffer's size to accomodate for the requested count if necessary
{
	size_t newsize;
	void *p;

	if (needed_count > alloc_count)
	{
		newsize = ceil((double) needed_count * inc_ratio);

		// Try realloc to the new larger size
		p = realloc(*buffer, newsize * size_elem);
		if (p == NULL)
		{
			fprintf_rl(stderr, "realloc(*buffer=%p, size=%zu) failed.\n", (void *) *buffer, newsize * size_elem);
			return alloc_count;
		}
		else
			*buffer = p;

		// Blank the new bytes
		memset(&((uint8_t *)(*buffer))[alloc_count * size_elem], 0, (newsize-alloc_count) * size_elem);

		alloc_count = newsize;

		if (*buffer==NULL)
			fprintf_rl(stderr, "realloc failed in alloc_enough() for alloc_count = %d\n", alloc_count);
	}
	
	return alloc_count;
}

void free_null(void **ptr)
{
	if (ptr)
	{
		free(*ptr);
		*ptr = NULL;
	}
}

void **calloc_2d(const size_t ptr_count, const size_t size_buffers, const size_t size_elem)
{
	size_t i;
	uint8_t **array;

	array = calloc(ptr_count, sizeof(uint8_t *));
	if (array==NULL)
		return NULL;

	for (i=0; i < ptr_count; i++)
		array[i] = calloc(size_buffers, size_elem);

	return array;
}

void **memcpy_2d(void **dst, void **src, const size_t ptr_count, const size_t size_buffers)
{
	size_t i;

	if (dst==NULL || src==NULL)
		return NULL;

	for (i=0; i < ptr_count; i++)
		memcpy(dst[i], src[i], size_buffers);

	return dst;
}

void **copy_2d(void **ptr, const size_t ptr_count, const size_t size_buffers)
{
	size_t i;
	void **array;

	array = calloc(ptr_count, sizeof(void *));
	if (array==NULL)
		return NULL;

	for (i=0; i < ptr_count; i++)
		array[i] = copy_alloc(ptr[i], size_buffers);

	return array;
}

void **memset_2d(void **ptr, const int word, const size_t size, const size_t count)
{
	size_t i;

	for (i=0; i < count; i++)
		memset(ptr[i], word, size);

	return ptr;
}

void free_2d(void **ptr, const size_t count)
{
	size_t i;

	if (ptr==NULL)
		return ;

	for (i=0; i < count; i++)
		free(ptr[i]);

	free(ptr);
}

void free_null_2d(void ***ptr, const size_t count)
{
	if (ptr)
	{
		free_2d(*ptr, count);
		*ptr = NULL;
	}
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

size_t next_aligned_offset(size_t offset, const size_t size_elem)
{
	if (offset & (size_elem-1))			// if the offset isn't aligned
		return (offset | (size_elem-1)) + 1;	// align it to the next size_elem-aligned offset
	else
		return offset;
}
