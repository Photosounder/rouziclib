extern void draw_line_thin_lrgb(framebuffer_t fb, xy_t p1, xy_t p2, double radius, lrgb_t colour, const blend_func_t bf, double intensity);
extern void draw_line_thin_frgb(framebuffer_t fb, xy_t p1, xy_t p2, double radius, frgb_t colour, const blend_func_fl_t bf, double intensity);
extern void draw_line_thin_dq(framebuffer_t fb, xy_t p1, xy_t p2, double radius, frgb_t colour, const int bf, double intensity, int quality);
extern void draw_line_thin(framebuffer_t fb, xy_t p1, xy_t p2, double radius, col_t colour, const blend_func_t bf, double intensity);
extern void draw_line_thin_rectclip(framebuffer_t fb, xy_t p1, xy_t p2, xy_t b1, xy_t b2, double radius, col_t colour, const blend_func_t bf, double intensity);
extern void draw_line_thin_short(framebuffer_t fb, xy_t p1, xy_t p2, double u1, double u2, double radius, col_t colour, const blend_func_t bf, double intensity);
