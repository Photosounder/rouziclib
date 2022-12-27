void swap_float(float *a, float *b)
{
	float c = *a;

	*a = *b;
	*b = c;
}

void swap_double(double *a, double *b)
{
	double c = *a;

	*a = *b;
	*b = c;
}

void swap_xy_xy(xy_t *a, xy_t *b)
{
	xy_t c = *a;

	*a = *b;
	*b = c;
}

void swap_i32(int32_t *a, int32_t *b)
{
	int32_t c = *a;

	*a = *b;
	*b = c;
}

void swap_int(int *a, int *b)
{
	int c = *a;

	*a = *b;
	*b = c;
}

void swap_ptr(void **a, void **b)
{
	void *c = *a;

	*a = *b;
	*b = c;
}

void swap_char(char *a, char *b)
{
	char c = *a;

	*a = *b;
	*b = c;
}

void swap_byte(uint8_t *a, uint8_t *b)
{
	uint8_t c = *a;

	*a = *b;
	*b = c;
}

void swap_mem(void *a, void *b, size_t size)
{
	size_t i;

	// Swap 4 bytes at a time
	for (i=0; i <= size-4; i+=4)
		swap_i32((int32_t *) a+i, (int32_t *) b+i);

	// Return if there aren't any trailing bytes
	if (i == size)
		return;

	// Swap remaining bytes
	for (; i < size; i++)
		swap_byte((uint8_t *) a+i, (uint8_t *) b+i);
}
