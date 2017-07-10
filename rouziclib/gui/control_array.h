extern int8_t get_state_checkbox_array(int8_t *array, int len);
extern int ctrl_array_checkbox_fullarg(raster_t fb, zoom_t zc, mouse_t mouse, vector_font_t *font, double drawing_thickness, int8_t *array, int count, char **label, col_t *col, int col_count, rect_t box, xy_t pos_inc);
extern int ctrl_array_checkbox_with_all_fullarg(raster_t fb, zoom_t zc, mouse_t mouse, vector_font_t *font, double drawing_thickness, int8_t *array, int count, char *all_label, col_t all_col, char **label, col_t *col, int col_count, rect_t box, xy_t pos_inc);
extern int ctrl_array_radio_fullarg(raster_t fb, zoom_t zc, mouse_t mouse, vector_font_t *font, double drawing_thickness, int *sel, int count, char **label, col_t *col, int col_count, rect_t box, xy_t pos_inc);
extern int ctrl_checkbox_subknob_fullarg(raster_t fb, zoom_t zc, mouse_t mouse, vector_font_t *font, double drawing_thickness, double *v_orig, double step, double ctrl_num, double subknob_num, knob_t knob, rect_t topbox, col_t colour);
extern int ctrl_array_knob_fullarg(raster_t fb, zoom_t zc, mouse_t mouse, vector_font_t *font, double drawing_thickness, double *knob_value, knob_t *knob_data, int count, col_t *col, int col_count, rect_t box, xy_t pos_inc);

#define ctrl_array_checkbox(array, count, label, col, col_count, box, pos_inc)	ctrl_array_checkbox_fullarg(fb, zc, mouse, font, drawing_thickness, array, count, label, col, col_count, box, pos_inc)
#define ctrl_array_checkbox_with_all(array, count, all_label, all_col, label, col, col_count, box, pos_inc)	ctrl_array_checkbox_with_all_fullarg(fb, zc, mouse, font, drawing_thickness, array, count, all_label, all_col, label, col, col_count, box, pos_inc)
#define ctrl_array_radio(sel, count, label, col, col_count, box, pos_inc)	ctrl_array_radio_fullarg(fb, zc, mouse, font, drawing_thickness, sel, count, label, col, col_count, box, pos_inc)
#define ctrl_checkbox_subknob(v_orig, step, ctrl_num, subknob_num, knob, topbox, colour)	ctrl_checkbox_subknob_fullarg(fb, zc, mouse, font, drawing_thickness, v_orig, step, ctrl_num, subknob_num, knob, topbox, colour)
#define ctrl_array_knob(knob_value, knob_data, count, col, col_count, box, pos_inc)	ctrl_array_knob_fullarg(fb, zc, mouse, font, drawing_thickness, knob_value, knob_data, count, col, col_count, box, pos_inc)
