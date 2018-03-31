// in gui/controls_struct.h:
// knob_t

// in gui/inputprocessing_struct.h
// ctrl_button_state_t, ctrl_knob_state_t, ctrl_drag_state_t, ctrl_id_t

extern int ctrl_button_invis_fullarg(zoom_t zc, rect_t box, ctrl_button_state_t *butt_state_ptr);
extern int ctrl_button_chamf_fullarg(framebuffer_t fb, zoom_t zc, mouse_t mouse, vector_font_t *font, double drawing_thickness, uint8_t *name, rect_t box, col_t colour);
extern int ctrl_checkbox_fullarg(framebuffer_t fb, zoom_t zc, mouse_t mouse, vector_font_t *font, double drawing_thickness, int8_t *state, uint8_t *name, rect_t box, col_t colour);
extern int ctrl_radio_fullarg(framebuffer_t fb, zoom_t zc, mouse_t mouse, vector_font_t *font, double drawing_thickness, int8_t state, uint8_t *name, rect_t box, col_t colour);
extern knob_t make_knob(char *main_label, double default_value, const knob_func_t func, double min, double max, char *fmt_str);
extern int ctrl_knob_fullarg(framebuffer_t fb, zoom_t zc, mouse_t mouse, vector_font_t *font, double drawing_thickness, double *v_orig, knob_t knob, rect_t box, col_t colour);
extern ctrl_drag_state_t make_drag_state(xy_t pos, xy_t freedom);
extern int ctrl_draggable_fullarg(framebuffer_t fb, zoom_t zc, mouse_t mouse, vector_font_t *font, double drawing_thickness, ctrl_drag_state_t *state, xy_t dim);
extern int ctrl_resizing_rect_fullarg(framebuffer_t fb, zoom_t zc, mouse_t mouse, vector_font_t *font, double drawing_thickness, ctrl_resize_rect_t *data, rect_t *box);

#define ctrl_button_invis(box)			ctrl_button_invis_fullarg(zc, box, NULL)
#define ctrl_button_chamf(name, box, colour)	ctrl_button_chamf_fullarg(fb, zc, mouse, font, drawing_thickness, name, box, colour)
#define ctrl_checkbox(state, name, box, colour)	ctrl_checkbox_fullarg(fb, zc, mouse, font, drawing_thickness, state, name, box, colour)
#define ctrl_radio(state, name, box, colour)	ctrl_radio_fullarg(fb, zc, mouse, font, drawing_thickness, state, name, box, colour)
#define ctrl_knob(v_orig, knob, box, colour)	ctrl_knob_fullarg(fb, zc, mouse, font, drawing_thickness, v_orig, knob, box, colour)
#define ctrl_draggable(state, dim)		ctrl_draggable_fullarg(fb, zc, mouse, font, drawing_thickness, state, dim)
#define ctrl_resizing_rect(state, box)		ctrl_resizing_rect_fullarg(fb, zc, mouse, font, drawing_thickness, state, box)
