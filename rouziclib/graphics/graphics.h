// in graphics/graphics_struct.h:
// srgb_t, lrgb_t, frgb_t, raster_t

#ifndef LBD			// for lrgb_t
#define LBD	15		// Linear Bit Depth (per channel), 12 <= LBD <= 15
#endif
#define ONE	(1<<LBD)	// Value of 1.0 in the linear pixel format (2^LBD)
#define ONEF	((double) ONE)

#if LBD == 15
#define LBD_TO_Q15(x)	(x)
#define Q15_TO_LBD(x)	(x)
#else
#define LBD_TO_Q15(x)	((x) << (15-LBD))
#define Q15_TO_LBD(x)	((x) >> (15-LBD))
#endif

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
extern size_t get_raster_mode_elem_size(const int mode);
extern raster_t make_raster(void *data, const xyi_t dim, xyi_t maxdim, const int mode);
extern raster_t make_raster_empty();
extern raster_t copy_raster(raster_t r0);
extern void **get_raster_buffer_for_mode_ptr(raster_t *r, const int mode);
extern void *get_raster_buffer_for_mode(raster_t r, const int mode);
extern void **get_raster_buffer_ptr(raster_t *r);
extern void *get_raster_buffer(raster_t *r);
extern int get_raster_mode(raster_t r);
extern srgb_t get_raster_pixel_in_srgb(raster_t r, const int index);
extern void free_raster(raster_t *r);
extern void cl_unref_raster(raster_t *r);
extern framebuffer_t init_framebuffer(xyi_t dim, xyi_t maxdim, const int mode);
extern void init_tls_fb(xyi_t dim);
extern double intensity_scaling(double scale, double scale_limit);
extern void thickness_limit(double *thickness, double *brightness, double limit);
extern void screen_blank(framebuffer_t fb);
extern void draw_gain(framebuffer_t fb, double gain);
extern void draw_luma_compression(framebuffer_t fb, double factor);
