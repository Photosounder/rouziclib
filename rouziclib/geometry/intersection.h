extern void line_line_intersection(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4, double *x, double *y);
extern double point_line_distance(double x1, double y1, double x2, double y2, double x3, double y3);
extern void border_clip(double w, double h, double *x1, double *y1, double *x2, double *y2, double radius);
extern void line_rect_clip(xy_t *l1, xy_t *l2, xy_t b1, xy_t b2);
extern int check_point_within_box(xy_t p, xy_t box0, xy_t box1);
extern int plane_line_clip_far_z(xyz_t *p1, xyz_t *p2, double zplane);
