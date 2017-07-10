void alloc_more(void **buffer, int add_count, int *current_count, int *alloc_count, int size_elem, double inc_ratio)	// doubles a buffer's size if necessary
{
	int newsize;

	(*current_count) += add_count;

	if (*current_count >= *alloc_count)
	{
		//newsize = MAXN(1, *alloc_count * 2);
		newsize = ceil((double) *current_count * inc_ratio);

		*buffer = realloc(*buffer, newsize * size_elem);
		memset(&((uint8_t *)(*buffer))[*alloc_count * size_elem], 0, (newsize-*alloc_count) * size_elem);

		*alloc_count = newsize;

		if (*buffer==NULL)
			fprintf_rl(stderr, "realloc failed in alloc_more() for *alloc_count = %d\n", *alloc_count);
	}
}
