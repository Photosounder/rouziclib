extern float eval_polygon_at_pos(xy_t *p, int p_count, double radius, xy_t pos);
extern void draw_polygon_lrgb(xy_t *p, int p_count, double radius, lrgb_t colour, const blend_func_t bf, double intensity);
extern void draw_polygon_dq(xy_t *p, int p_count, double radius, frgb_t colour, const int bf, double intensity);
extern rect_t get_bounding_box_for_polygon(xy_t *p, int p_count);
extern int get_dq_bounding_box_for_polygon(xy_t *p, int p_count, xy_t rad, recti_t *bbi);
extern void draw_polygon(xy_t *p, int p_count, double radius, col_t colour, const blend_func_t bf, double intensity);
extern void draw_polygon_wc(xy_t *p, int p_count, double radius, col_t colour, const blend_func_t bf, double intensity);
extern void draw_triangle(triangle_t tri, double radius, col_t colour, const blend_func_t bf, double intensity);
extern void draw_triangle_wc(triangle_t tri, double radius, col_t colour, const blend_func_t bf, double intensity);
