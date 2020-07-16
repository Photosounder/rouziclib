void swap_double(double *a, double *b)
{
	double c = *a;

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
