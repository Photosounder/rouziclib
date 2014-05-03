typedef struct
{
	int32_t init;	// TODO remove
	int32_t i, lut_size;
	double lut_range, lut_range_inv;
	double mul, add;
	double *lut;
	int32_t *lutint;	// same as lut except in fixed point format
	uint8_t *lutb;		// for values from 0 to 255 (like sRGB colours)
} lut_t;
