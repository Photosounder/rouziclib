#ifndef LBD
#define LBD	15		// Linear Bit Depth (per channel), 12 <= LBD <= 15
#endif
#define ONE	(1<<LBD)	// Value of 1.0 in the linear pixel format (2^LBD)
#define ONEF	((double) ONE)

#ifndef GAUSSLIMIT
#define GAUSSLIMIT	0.0002	// limit of intensity for drawing lines in the [0, 1) range
#endif
// solves e^-x² = GAUSSLIMIT for x, giving 2.92 (the necessary Gaussian radius) for GAUSSLIMIT of 0.0002 with a radius of 1
#define GAUSSRAD(intensity, radius)	(sqrt(log(intensity / GAUSSLIMIT))*radius)

typedef struct
{
#ifdef __APPLE__
	uint32_t a:8;
	uint32_t r:8;
	uint32_t g:8;
	uint32_t b:8;
#else
	uint32_t b:8;
	uint32_t g:8;
	uint32_t r:8;
	uint32_t a:8;
#endif
} srgb_t;			// sRGB

typedef struct
{
	uint16_t r, g, b, a;	// in 1.LBD format (as it goes up to a fp value of 1.0)
} lrgb_t;			// linear RGB format
