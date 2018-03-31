enum
{
	gui_type_null,
	gui_type_none,
	gui_type_label,
	gui_type_button,
	gui_type_checkbox,
	gui_type_knob,
	gui_type_textedit,

	gui_type_count
};
// this enum must match the string array in make_gui.c
extern const char *layout_elem_type_name[];

typedef struct
{
	int type;
	xy_t pos, dim, pos_off;
	rect_t area;
	char *label;
	int label_as;
	col_t colour;
	void *data;
	ctrl_resize_rect_t resize_ctrl;
	int label_set, label_as_set, fmt_str_set;
	int implemented;
} layout_elem_t;

typedef struct
{
	int init, elem_as, edit_on, sel_id;
	layout_elem_t *elem;
	xy_t offset;
	double sm;
} gui_layout_t;

extern void free_layout_elem_data(layout_elem_t *elem);
extern void free_gui_layout(gui_layout_t *layout);
extern void make_gui_layout(gui_layout_t *layout, const char **src, const int linecount);
extern void sprint_gui_layout(gui_layout_t *layout, char **str, int *str_as);
extern int check_layout_id_validity(gui_layout_t *layout, const int id);
extern int ctrl_fromlayout_resizing_fullarg(framebuffer_t fb, zoom_t zc, mouse_t mouse, vector_font_t *font, double drawing_thickness, gui_layout_t *layout, const int id, const int phase);

extern void draw_label_fromlayout_fullarg(framebuffer_t fb, zoom_t zc, mouse_t mouse, vector_font_t *font, double drawing_thickness, gui_layout_t *layout, const int id, const int mode);
extern int ctrl_button_fromlayout_fullarg(framebuffer_t fb, zoom_t zc, mouse_t mouse, vector_font_t *font, double drawing_thickness, gui_layout_t *layout, const int id);
extern int ctrl_knob_fromlayout_fullarg(framebuffer_t fb, zoom_t zc, mouse_t mouse, vector_font_t *font, double drawing_thickness, double *v, gui_layout_t *layout, const int id);
extern int ctrl_textedit_fromlayout_fullarg(framebuffer_t fb, zoom_t zc, mouse_t mouse, vector_font_t *font, double drawing_thickness, gui_layout_t *layout, const int id);
extern void gui_layout_unimplemented_elems_fullarg(framebuffer_t fb, zoom_t zc, mouse_t mouse, vector_font_t *font, double drawing_thickness, gui_layout_t *layout);

#define ctrl_fromlayout_resizing(layout, id, phase)	ctrl_fromlayout_resizing_fullarg(fb, zc, mouse, font, drawing_thickness, layout, id, phase)
#define draw_label_fromlayout(layout, id, mode)	draw_label_fromlayout_fullarg(fb, zc, mouse, font, drawing_thickness, layout, id, mode)
#define ctrl_button_fromlayout(layout, id)	ctrl_button_fromlayout_fullarg(fb, zc, mouse, font, drawing_thickness, layout, id)
#define ctrl_checkbox_fromlayout(state, layout, id)	ctrl_checkbox_fromlayout_fullarg(fb, zc, mouse, font, drawing_thickness, state, layout, id)
#define ctrl_knob_fromlayout(v, layout, id)	ctrl_knob_fromlayout_fullarg(fb, zc, mouse, font, drawing_thickness, v, layout, id)
#define ctrl_textedit_fromlayout(layout, id)	ctrl_textedit_fromlayout_fullarg(fb, zc, mouse, font, drawing_thickness, layout, id)
#define gui_layout_unimplemented_elems(layout)	gui_layout_unimplemented_elems_fullarg(fb, zc, mouse, font, drawing_thickness, layout)

/* Available commands for layout source

elem		<array ID>
type		<name of type>
label		<label string>
label_alloc	<alloc size>
pos		<x y position of area>
dim		<x y dimensions of area>
off		<x y position offset for area>
knob		<min> <default> <max> <function (linear|log|recip)> (<format string>)

TODO colour
*/
