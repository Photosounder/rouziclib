enum		// custom unicode colours
{
	sc_start=0xE000,
	sc_white,
	sc_black,
	sc_red,
	sc_green,
	sc_blue,
	sc_baby_azure,
	sc_amber,
	sc_azure,
	sc_grey,

	sc_end
};

extern int draw_vector_char(raster_t fb, vector_font_t *font, uint32_t c, xy_t p, xy_t off, double scale, col_t colour, double line_thick, const int mode, const int bidi);
extern int draw_vector_char_lookahead(raster_t fb, vector_font_t *font, uint32_t c, char *string, xy_t p, xy_t off, double scale, col_t colour, double line_thick, const int mode, const int bidi);
extern void draw_string_full(raster_t fb, vector_font_t *font, char *string, xy_t p, xy_t off, double scale, col_t colour, double intensity, double line_thick, const int mode, int32_t len, double glyph_limit, double line_limit, const int bidi, const int recur, text_param_t *tp);
extern void draw_string_len(raster_t fb, vector_font_t *font, char *string, xy_t p, double scale, col_t colour, double intensity, double line_thick, const int mode, int32_t len, text_param_t *tp);
extern void draw_string(raster_t fb, vector_font_t *font, char *string, xy_t p, double scale, col_t colour, double intensity, double line_thick, const int mode, text_param_t *tp);
extern void print_to_screen_fullarg(raster_t fb, zoom_t zc, vector_font_t *font, xy_t pos, double scale, col_t colour, double intensity, const int32_t mode, double line_thick, const char *format, ...);

#define print_to_screen(pos, scale, colour, intensity, mode, format, ...)	print_to_screen_fullarg(fb, zc, font, pos, scale, colour, intensity, mode, drawing_thickness, format, __VA_ARGS__)
