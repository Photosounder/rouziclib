enum
{
	FULLCIRCLE,
	HOLLOWCIRCLE,
};

extern void draw_circle(const int circlemode, raster_t fb, double x, double y, double circrad, double radius, lrgb_t colour, const blend_func_t bf, double intensity);
extern void draw_rect(raster_t fb, xy_t p0, xy_t p1, double radius, lrgb_t colour, const blend_func_t bf, double intensity);
extern void draw_rect_chamfer(raster_t fb, xy_t p0, xy_t p1, double radius, lrgb_t colour, const blend_func_t bf, double intensity, double chamfer);
extern int32_t get_dist_to_roundrect(int32_t lx1, int32_t ly1, int32_t lx2, int32_t ly2, int32_t corner, int32_t ixf, int32_t iyf);
extern void draw_roundrect(raster_t fb, double x1, double y1, double x2, double y2, double corner, double radius, lrgb_t colour, const blend_func_t bf, double intensity);
extern void draw_roundrect_frame(raster_t fb, double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4, double corner1, double corner2, double radius, lrgb_t colour, const blend_func_t bf, double intensity);
extern void draw_polar_glow(raster_t fb, double cx, double cy, lrgb_t col, double colmul, double scale, double rad, double gradr, double gradth, double angle, int32_t islog, int32_t riserf, double erfrad, double pixoffset);
extern void draw_gaussian_gradient(raster_t fb, double cx, double cy, lrgb_t c0, lrgb_t c1, double gausrad, double gausoffx, double gausoffy, const blend_func_t bf);
extern void draw_point_lrgb(raster_t fb, double x, double y, double radius, lrgb_t colour, const blend_func_t bf, double intensity);
extern void draw_point_frgb(raster_t fb, double x, double y, double radius, frgb_t colour, const blend_func_fl_t bf, double intensity);
extern void draw_point_cl(raster_t fb, double x, double y, double radius, frgb_t colour, const blend_func_fl_t bf, double intensity);
extern void draw_point(raster_t fb, double x, double y, double radius, lrgb_t colour, const blend_func_t bf, double intensity);
extern void draw_point_xy(raster_t fb, xy_t p, double radius, lrgb_t colour, const blend_func_t bf, double intensity);
extern void draw_point_on_row(raster_t fb, double x, int32_t y, double radius, lrgb_t colour, const blend_func_t bf, double intensity);
