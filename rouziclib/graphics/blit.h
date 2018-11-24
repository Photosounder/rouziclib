enum
{	// alignments
	A_CEN,
	A_LEF,
	A_RIG,
	A_TOP,
	A_BOT,
};

enum
{	// interpolation and filtering modes
	NO_FILTER,
	LINEAR_FILTER,

	NEAREST_INTERP,
	LINEAR_INTERP,
	GAUSSIAN_INTERP,
};

typedef struct
{
	double top, knee, c0, c1;
	double (*func)();
} interp_param_t;

extern int sprite_offsets_old(int32_t fbw, int32_t fbh, int32_t spw, int32_t sph, int32_t *pos_x, int32_t *pos_y, int32_t *offset_x, int32_t *offset_y, int32_t *start_x, int32_t *start_y, int32_t *stop_x, int32_t *stop_y, int hmode, int vmode);
extern int sprite_offsets(framebuffer_t fb, raster_t r, xyi_t *pos, xyi_t *offset, xyi_t *start, xyi_t *stop, int hmode, int vmode);
extern int calc_blit_bounds(xyi_t in_dim, xyi_t out_dim, xyi_t offset, recti_t *bounds);
extern void blit_sprite(framebuffer_t fb, raster_t r, xyi_t pos, const blend_func_t bf, int hmode, int vmode);
extern void blit_layout(framebuffer_t fb, raster_t r);
extern void blit_scale_lrgb(framebuffer_t fb, raster_t r, xy_t pscale, xy_t pos, int interp);
extern void blit_scale_cl(framebuffer_t *fb, raster_t *r, xy_t pscale, xy_t pos, int interp);
extern void blit_scale(framebuffer_t *fb, raster_t *r, xy_t pscale, xy_t pos, int interp);
extern void blit_in_rect(framebuffer_t *fb, raster_t *raster, rect_t r, int keep_aspect_ratio, int interp);
extern void blit_scale_photo_cl(framebuffer_t fb, raster_t r, xy_t pscale, xy_t pos, int interp, xy_t pc, double distortion, double gain);
extern void blit_photo_in_rect(framebuffer_t fb, raster_t raster, rect_t r, int keep_aspect_ratio, int interp, xy_t pc, double distortion, double gain);

extern double blit_scale_func_linear(double x, double unit_inv, interp_param_t p);
extern double blit_scale_func_modlin(double x, double unit_inv, interp_param_t p);
extern interp_param_t calc_interp_param_modlin(double n);
extern double calc_flattop_slope(double n);
