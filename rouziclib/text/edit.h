// in vector_type/vector_type_struct.h:
// textedit_t

extern void textedit_init(textedit_t *te);
extern void textedit_add(textedit_t *te, char *str, int32_t cmd, int32_t mod);
extern void test_textedit_add();
extern int ctrl_textedit_fullarg(raster_t fb, zoom_t zc, mouse_t mouse, vector_font_t *font, double drawing_thickness, textedit_t *te, rect_t box, col_t colour);
extern void draw_textedit_cursor(raster_t fb, xy_t offset, double scale, int bidi, int bidi_change, double drawing_thickness);

#define ctrl_textedit(te, box, colour)	ctrl_textedit_fullarg(fb, zc, mouse, font, drawing_thickness, te, box, colour)

extern textedit_t *cur_textedit;
