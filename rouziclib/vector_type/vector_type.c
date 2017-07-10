#include "make_font.c"
#include "draw.c"
#include "stats.c"
#include "fit.c"
#include "cjk.c"

int32_t get_letter_index(vector_font_t *font, uint32_t c)
{
	if (c > 0x10FFFF)
		return -1;

	if (font==NULL)
		return -1;

	return font->codepoint_letter_lut[c];
}

letter_t *get_letter(vector_font_t *font, uint32_t c)
{
	int index;

	index = get_letter_index(font, c);

	if (index >= 0)
		return &font->l[index];

	return NULL;
}

vobj_t *get_letter_obj(vector_font_t *font, uint32_t c)
{
	letter_t *l = get_letter(font, c);
	if (l)
		return l->obj;
	return NULL;
}

char *get_letter_glyphdata(vector_font_t *font, uint32_t c)
{
	letter_t *l = get_letter(font, c);
	if (l)
		return l->glyphdata;
	return NULL;
}

letter_t *get_dominant_letter(vector_font_t *font, uint32_t c, int *lowerscale)
{
	uint32_t dom_c;
	letter_t *l;
	unicode_data_t ucd;

	if (c > 0x10FFFF)
		return NULL;

	l = get_letter(font, c);

	if (l)
	{
		process_one_glyph(font, get_letter_index(font, c));

		if (l->obj)
			return l;
	}

	// Alias
	if (l)
		if (l->alias)
			return get_dominant_letter(font, l->alias, lowerscale);

	ucd = get_unicode_data(c);

	// Combo
	if (/*ucd.decomp_type == decomp_canonical &&*/ ucd.combo1)
		return get_dominant_letter(font, ucd.combo1, lowerscale);

	// Upper case
	if (ucd.upper_map)
	{
		*lowerscale = 1;
		return get_dominant_letter(font, ucd.upper_map, lowerscale);
	}

	return NULL;
}
