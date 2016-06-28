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

extern int draw_vector_char(vector_font_t *font, uint32_t c, xy_t p, xy_t off, double scale, lrgb_t colour, double intensity, const int mode);
extern void draw_string_nolimit(vector_font_t *font, uint8_t *string, xy_t p, double scale, lrgb_t colour, double intensity, const int mode, int32_t len);
extern void draw_string(vector_font_t *font, uint8_t *string, xy_t p, double scale, lrgb_t colour, double intensity, const int mode);
