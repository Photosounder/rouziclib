enum
{
	FULLCIRCLE,
	HOLLOWCIRCLE,
};

extern void draw_circle_lrgb(const int circlemode, xy_t pos, double circrad, double radius, lrgb_t colour, const blend_func_t bf, double intensity);
extern void draw_circle_dq(const int circlemode, xy_t pos, double circrad, double radius, frgb_t colour, double intensity);
extern void draw_circle(const int circlemode, xy_t pos, double circrad, double radius, col_t colour, const blend_func_t bf, double intensity);
extern void draw_circle_with_lines(xy_t pos, double circrad, double radius, col_t colour, const blend_func_t bf, double intensity);
extern void draw_rect(rect_t r, double radius, col_t colour, const blend_func_t bf, double intensity);
extern void draw_rect_chamfer(rect_t r, double radius, col_t colour, const blend_func_t bf, double intensity, double chamfer);
extern void draw_line_dashed(xy_t p1, xy_t p2, double dash_period, double dash_ratio, double phase, double radius, col_t colour, const blend_func_t bf, double intensity);
extern int32_t get_dist_to_roundrect(int32_t lx1, int32_t ly1, int32_t lx2, int32_t ly2, int32_t corner, int32_t ixf, int32_t iyf);
extern void draw_roundrect(rect_t box, double corner, double radius, lrgb_t colour, const blend_func_t bf, double intensity);
extern void draw_roundrect_frame(rect_t box1, rect_t box2, double corner1, double corner2, double radius, lrgb_t colour, const blend_func_t bf, double intensity);
extern void draw_point_lrgb(xy_t pos, double radius, lrgb_t colour, const blend_func_t bf, double intensity);
extern void draw_point_frgb(xy_t pos, double radius, frgb_t colour, const blend_func_fl_t bf, double intensity);
extern void draw_point_dq(xy_t pos, double radius, frgb_t colour, double intensity);
extern void draw_point(xy_t pos, double radius, col_t colour, const blend_func_t bf, double intensity);
extern void draw_point_on_row(xy_t pos, double radius, lrgb_t colour, const blend_func_t bf, double intensity);
extern void draw_triangle_thin(triangle_t tr, double drawing_thickness, col_t col, const blend_func_t bf, double intensity);
extern void draw_mousecursor(xy_t pos);
