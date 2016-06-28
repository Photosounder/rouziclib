#ifndef LBD
#define LBD	15		// Linear Bit Depth (per channel), 12 <= LBD <= 15
#endif
#define ONE	(1<<LBD)	// Value of 1.0 in the linear pixel format (2^LBD)
#define ONEF	((double) ONE)

#ifndef GAUSSLIMIT
#define GAUSSLIMIT	0.0002	// limit of intensity for drawing lines in the [0, 1) range (0.0002 == 0.66/255 in sRGB)
#endif

#ifndef GAUSSRAD_HQ
#define GAUSSRAD_HQ	4.
#endif

// solves e^-x² = GAUSSLIMIT for x, giving 2.92 (the necessary Gaussian radius) for GAUSSLIMIT of 0.0002 with a radius of 1
//#define GAUSSRAD(intensity, radius)	(sqrt(log(intensity / GAUSSLIMIT))*radius)
#define GAUSSRAD gaussrad	// this is much faster

static double gaussrad(double intensity, double radius)
{
	static double last_intensity=0., last_radius=0., last_result=0.;

	if (last_intensity==intensity && last_radius==radius)
		return last_result;
	else
	{
		last_intensity = intensity;
		last_radius = radius;
		if (intensity > GAUSSLIMIT)
			last_result = (sqrt(log(intensity / GAUSSLIMIT))*radius);
		else
			last_result = 0.;
		return last_result;
	}
}

typedef struct
{
	uint32_t r:8;
	uint32_t g:8;
	uint32_t b:8;
	uint32_t a:8;
} srgb_t;			// sRGB

typedef struct
{
	uint16_t r, g, b, a;	// in 1.LBD format (as it goes up to a fp value of 1.0)
} lrgb_t;			// linear RGB format

typedef struct
{
	float r, g, b, a;
} frgb_t;			// linear RGB format

typedef struct
{
	int32_t w, h;
	lrgb_t *l;
	frgb_t *f;
	srgb_t *srgb;
	int use_frgb;
	int use_cl;

	#ifdef RL_OPENCL
	cl_mem clbuf, cl_srgb;
	uint64_t clbuf_da;	// device address for clbuf
	uint32_t gltex;		// ID of the GL texture for cl_srgb
	clctx_t clctx;		// contains the context and the command queue

	// Draw queue data
	int32_t *drawq_data, *sector_pos, *entry_list, *sector_list, *sector_count;
	cl_mem drawq_data_cl, sector_pos_cl, entry_list_cl;

	int drawq_size;		// number of floats/ints in the queue
	int list_alloc_size;	// allocation size of entry and sector lists
	int max_sector_count;
	int entry_list_end;	// end (size) of entry_list
	int sectors;		// number of subdivisions (and separate drawing queues) on the screen
	int sector_size;	// size of the sectors in powers of two. sector_size==6 means 64x64 sized sectors
	int sector_w;		// number of sector per row (for instance rows of 30 64x64 sectors for 1920x1080)
	#endif
} raster_t;

#include <stdlib.h>
static raster_t make_raster_l(lrgb_t *l, int32_t w, int32_t h)
{
	raster_t r;

	r.w = w;
	r.h = h;
	r.use_frgb = 0;

	if (l)
		r.l = l;
	else
		r.l = calloc(w*h, sizeof(lrgb_t));

	return r;
}

static raster_t make_raster_f(frgb_t *f, int32_t w, int32_t h)
{
	raster_t r;

	r.w = w;
	r.h = h;
	r.use_frgb = 1;

	if (f)
		r.f = f;
	else
		r.f = calloc(w*h, sizeof(frgb_t));

	return r;
}

static double intensity_scaling(double scale, double scale_limit)	// gives an intensity ratio that decreases if the scale of the thing to be drawn is below a scale threshold
{
	double ratio = 1.;

	if (scale < scale_limit)
		ratio = scale / scale_limit;

	return ratio;
}

static void thickness_limit(double *thickness, double *brightness, double limit)	// same except also limits thickness
{
	if (*thickness < limit)
	{
		*brightness *= *thickness / limit;
		*thickness = limit;
	}
}
