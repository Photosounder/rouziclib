#include "make_font.c"
#include "draw.c"
#include "stats.c"
#include "fit.c"

letter_t *get_letter(vector_font_t *font, uint32_t c)
{
	int index;

	if (c > 0x10FFFF)
		return NULL;

	index = font->codepoint_letter_lut[c];

	if (index >= 0)
		return &font->l[index];

	return NULL;
}

letter_t *get_uppercase_letter(vector_font_t *font, uint32_t lower_c)
{
	uint32_t upper_c;

	if (lower_c > 0x10FFFF)
		return NULL;

	upper_c = get_unicode_data(lower_c).upper_map;

	if (upper_c)
		return get_letter(font, upper_c);

	return NULL;
}
