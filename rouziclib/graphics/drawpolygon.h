extern void draw_polygon_dq(xy_t *p, int p_count, double radius, frgb_t colour, double intensity);
extern rect_t get_bounding_box_for_polygon(xy_t *p, int p_count);
extern int get_dq_bounding_box_for_polygon(xy_t *p, int p_count, xy_t rad, recti_t *bbi);
extern void draw_polygon(xy_t *p, int p_count, double radius, col_t colour, double intensity);
extern void draw_polygon_wc(xy_t *p, int p_count, double radius, col_t colour, double intensity);
extern void draw_triangle(triangle_t tri, double radius, col_t colour, double intensity);
extern void draw_triangle_wc(triangle_t tri, double radius, col_t colour, double intensity);
