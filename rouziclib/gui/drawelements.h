extern void draw_titled_roundrect_frame(raster_t fb, xy_t pos, double radius, xy_t c, xy_t space, lrgb_t colour, const blend_func_t bf);
extern void draw_label_fullarg(raster_t fb, zoom_t zc, vector_font_t *font, double drawing_thickness, uint8_t *label, rect_t box, col_t colour, const int mode);
extern void display_dialog_enclosing_frame_fullarg(raster_t fb, vector_font_t *font, double drawing_thickness, rect_t box_os, double scale, char *label, col_t colour);

#define draw_label(label, box, colour, mode)	draw_label_fullarg(fb, zc, font, drawing_thickness, label, box, colour, mode)
#define display_dialog_enclosing_frame(box_os, scale, label, colour)	display_dialog_enclosing_frame_fullarg(fb, font, drawing_thickness, box_os, scale, label, colour)
