extern int dialog_enclosing_frame_fullarg(framebuffer_t fb, zoom_t zc, vector_font_t *font, double drawing_thickness, xy_t offset, double sm, rect_t main_frame, double margin, char *label, col_t colour);
extern xy_t fit_into_area(rect_t area, rect_t box, double margin, double *sm);
extern rect_t make_title_rect(rect_t main_frame, double title_pos, double title_height);
extern xy_t get_grid_position(xy_t orig, int index, xyi_t grid_count, xy_t step, int horizontal);
extern rect_t get_grid_cell_rect(xyi_t ip, xy_t origin, xy_t cell_dim);
extern xyi_t get_grid_index_at_point(xy_t p, xy_t origin, xy_t cell_dim);
extern recti_t get_grid_index_rect(rect_t r, xy_t origin, xy_t cell_dim);

#define dialog_enclosing_frame(offset, sm, main_frame, margin, label, colour)	dialog_enclosing_frame_fullarg(fb, zc, font, drawing_thickness, offset, sm, main_frame, margin, label, colour)

#define check_box_on_screen(box)	check_box_box_intersection(box, zc.corners_dl)
