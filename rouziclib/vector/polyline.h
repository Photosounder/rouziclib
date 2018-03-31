typedef struct
{
	xy_t *pv;
	xyi_t *line;
	ctrl_drag_state_t *ds;
	int pv_count, pv_alloc;
	int line_count, line_alloc;
} polyline_edit_t;

extern int ctrl_polyline_fullarg(framebuffer_t fb, zoom_t zc, mouse_t mouse, vector_font_t *font, double drawing_thickness, polyline_edit_t *pl, rect_t box, xy_t offset, double sm);
extern void free_polyline(polyline_edit_t *pl);
extern char *sprint_polyline(polyline_edit_t *pl);
extern void round_polyline_pv(polyline_edit_t *pl, xy_t offset, double sm);

#define ctrl_polyline(pl, box, offset, sm)	ctrl_polyline_fullarg(fb, zc, mouse, font, drawing_thickness, pl, box, offset, sm)
