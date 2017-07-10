// in graphics/graphics_struct.h:
// srgb_t, lrgb_t, frgb_t, raster_t

#ifndef LBD			// for lrgb_t
#define LBD	15		// Linear Bit Depth (per channel), 12 <= LBD <= 15
#endif
#define ONE	(1<<LBD)	// Value of 1.0 in the linear pixel format (2^LBD)
#define ONEF	((double) ONE)

#ifndef GAUSSLIMIT
#define GAUSSLIMIT	0.0002	// limit of intensity for drawing lines in the [0, 1) range (0.0002 == 0.66/255 in sRGB)
#endif

#ifndef MIN_DRAW_THICKNESS
#define MIN_DRAW_THICKNESS	0.8
#endif

#ifndef GAUSSRAD_HQ
#define GAUSSRAD_HQ	4.
#endif

// solves e^-x² = GAUSSLIMIT for x, giving 2.92 (the necessary Gaussian radius) for GAUSSLIMIT of 0.0002 with a radius of 1
#define GAUSSRAD gaussrad	// this is much faster

extern double gaussrad(double intensity, double radius);
extern raster_t make_raster_l(lrgb_t *l, int32_t w, int32_t h);
extern raster_t make_raster_f(frgb_t *f, int32_t w, int32_t h);
extern void free_raster(raster_t *r);
extern double intensity_scaling(double scale, double scale_limit);
extern void thickness_limit(double *thickness, double *brightness, double limit);
extern void screen_blank(raster_t fb);
