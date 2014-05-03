enum
{
	FULLCIRCLE,
	HOLLOWCIRCLE,
};

extern void draw_circle(const int circlemode, lrgb_t *fb, int32_t w, int32_t h, double x, double y, double circrad, double radius, lrgb_t colour, const int blendingmode, double intensity);
extern int32_t get_dist_to_roundrect(int32_t lx1, int32_t ly1, int32_t lx2, int32_t ly2, int32_t corner, int32_t ixf, int32_t iyf);
extern void draw_roundrect(lrgb_t *fb, int32_t w, int32_t h, double x1, double y1, double x2, double y2, double corner, double radius, lrgb_t colour, const int mode, double intensity);
extern void draw_roundrect_frame(lrgb_t *fb, int32_t w, int32_t h, double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4, double corner1, double corner2, double radius, lrgb_t colour, const int mode, double intensity);
extern void draw_polar_glow(lrgb_t *fb, int32_t w, int32_t h, double cx, double cy, lrgb_t col, double colmul, double scale, double rad, double gradr, double gradth, double angle, int32_t islog, int32_t riserf, double erfrad, double pixoffset);
extern void draw_gaussian_gradient(lrgb_t *fb, int32_t w, int32_t h, double cx, double cy, lrgb_t c0, lrgb_t c1, double gausrad, double gausoffx, double gausoffy);
