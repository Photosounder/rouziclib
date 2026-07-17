extern frgb_t yuv_to_frgb(xyz_t yuv);
extern int buf_fmt_is_yuv420p(int buf_fmt);
extern frgb_t get_yuv420p_pixel_in_frgb(const raster_t *r, xyi_t p);
extern double frgb_to_yuv_y(frgb_t pv);
extern double frgb_to_yuv_u(frgb_t pv);
extern double frgb_to_yuv_v(frgb_t pv);
extern xyz_t frgb_to_yuv(frgb_t pv);
extern void frgb_2x2_to_yuv420(frgb_t *l0, frgb_t *l1, float *y, float *u, float *v);
