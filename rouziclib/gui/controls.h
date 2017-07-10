// in gui/controls_struct.h:
// knob_t

extern int ctrl_button_chamf_fullarg(raster_t fb, zoom_t zc, mouse_t mouse, vector_font_t *font, double drawing_thickness, uint8_t *name, rect_t box, col_t colour);
extern int ctrl_checkbox_fullarg(raster_t fb, zoom_t zc, mouse_t mouse, vector_font_t *font, double drawing_thickness, int8_t *state, uint8_t *name, rect_t box, col_t colour);
extern int ctrl_radio_fullarg(raster_t fb, zoom_t zc, mouse_t mouse, vector_font_t *font, double drawing_thickness, int8_t state, uint8_t *name, rect_t box, col_t colour);
extern knob_t make_knob(char *main_label, double default_value, const knob_func_t func, double min, double max, int valfmt);
extern int ctrl_knob_fullarg(raster_t fb, zoom_t zc, mouse_t mouse, vector_font_t *font, double drawing_thickness, double *v_orig, knob_t knob, rect_t box, col_t colour);
extern ctrl_drag_state_t make_drag_state(xy_t pos, xy_t freedom);
extern int ctrl_draggable_fullarg(raster_t fb, zoom_t zc, mouse_t mouse, vector_font_t *font, double drawing_thickness, ctrl_drag_state_t *state, uint8_t *name, xy_t dim, col_t colour);

#define ctrl_button_chamf(name, box, colour)	ctrl_button_chamf_fullarg(fb, zc, mouse, font, drawing_thickness, name, box, colour)
#define ctrl_checkbox(state, name, box, colour)	ctrl_checkbox_fullarg(fb, zc, mouse, font, drawing_thickness, state, name, box, colour)
#define ctrl_radio(state, name, box, colour)	ctrl_radio_fullarg(fb, zc, mouse, font, drawing_thickness, state, name, box, colour)
#define ctrl_knob(v_orig, knob, box, colour)	ctrl_knob_fullarg(fb, zc, mouse, font, drawing_thickness, v_orig, knob, box, colour)
#define ctrl_draggable(state, name, dim, colour)	ctrl_draggable_fullarg(fb, zc, mouse, font, drawing_thickness, state, name, dim, colour)
