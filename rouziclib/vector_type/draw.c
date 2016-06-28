int draw_vector_char(vector_font_t *font, uint32_t c, xy_t p, xy_t off, double scale, lrgb_t colour, double intensity, const int mode)
{
	letter_t *l;
	double fixoff, wc1, wc2;
	unicode_data_t ucd;
	int found = 0;

	if (c==0x0129)
		c=c;

	l = get_letter(font, c);
	if (l)
		if (l->obj)
		{
			found = 1;
			fixoff = 0.;
			if ((mode&4)==MONOSPACE)
				fixoff = 0.5 * (4. - l->width);
			fixoff -= l->bl;

			draw_vobj(l->obj, p.x + off.x + fixoff*scale, p.y + off.y, scale, 0., colour);
		}

	if (found==0)
	{
		ucd = get_unicode_data(c);

		if (ucd.upper_map)
			found |= draw_vector_char(font, ucd.upper_map, p, off, scale*LOWERCASESCALE, colour, intensity, mode);
	}

	if (found==0)
	{
		ucd = get_unicode_data(c);
		if (ucd.decomp_type == decomp_canonical && ucd.combo1)		// if we have a valid combo for this character
		{
			wc1 = letter_width(font, off.x, ucd.combo1, scale, mode);
			wc2 = letter_width(font, off.x, ucd.combo2, scale, mode);

			found |= draw_vector_char(font, ucd.combo1, p, off, scale, colour, intensity, mode);
			off.x += (wc1 - wc2) * 0.5;
			found |= draw_vector_char(font, ucd.combo2, p, off, scale, colour, intensity, mode);
		}
	}

	return found;
}

void draw_string_nolimit(vector_font_t *font, uint8_t *string, xy_t p, double scale, lrgb_t colour, double intensity, const int mode, int32_t len)
{
	uint32_t i, c;
	double w;
	xy_t off;
	uint8_t *rom;

	if ((mode&3)!=ALIG_LEFT)
	{
		w = calc_strlen_len(font, string, scale, mode, len);

		if ((mode&3)==ALIG_CENTRE)
			p.x -= w*0.5;

		if ((mode&3)==ALIG_RIGHT)
			p.x -= w;
	}

	colour.r = (double)colour.r * intensity + 0.5;
	colour.g = (double)colour.g * intensity + 0.5;
	colour.b = (double)colour.b * intensity + 0.5;

	if (len < 0)
		len = strlen(string);

	off.x = 0.;
	off.y = 0.;
	for (i=0; i<len; i++)
	{
		c = utf8_to_unicode32(&string[i], &i);

		if (c > sc_start && c < sc_end)
		{
			switch (c)
			{
				case sc_white:		colour = white;					break;
				case sc_black:		colour = black;					break;
				case sc_red:		colour = red;					break;
				case sc_green:		colour = make_colour_lin(0., 0.5, 0., 1.);	break;
				case sc_blue:		colour = blue;					break;
				case sc_baby_azure:	colour = make_colour_lin(0.1, 0.3, 1., 1.);	break;
				case sc_amber:		colour = make_colour_lin(1., 0.55, 0., 1.);	break;
				case sc_azure:		colour = make_colour_lin(0., 0.45, 1., 1.);	break;
				case sc_grey:		colour = make_colour_lin(0.18, 0.18, 0.18, 1.);	break;
				default:		colour = grey;
			}

			colour.r = (double)colour.r * intensity + 0.5;
			colour.g = (double)colour.g * intensity + 0.5;
			colour.b = (double)colour.b * intensity + 0.5;
		}
		else
		{
			switch (c)
			{
				case '\n':
					off.x = 0.;
					off.y -= LINEVSPACING * scale;
					break;

				default:
					draw_vector_char(font, c, p, off, scale, colour, intensity, mode);

					off.x += letter_width(font, off.x, c, scale, mode);
			}
		}
	}
}

void draw_string(vector_font_t *font, uint8_t *string, xy_t p, double scale, lrgb_t colour, double intensity, const int mode)
{
	double w=0., total_scale = scale*scrscale;

	intensity *= intensity_scaling(total_scale, 1.);

	if (total_scale >= 0.25)
		draw_string_nolimit(font, string, p, scale, colour, intensity, mode, -1);	// write the characters
	else if (total_scale >= 0.01)								// if it's too small
	{											// use a line instead
		if ((mode&3)!=ALIG_LEFT)
		{
			w = calc_strlen(font, string, scale, mode);

			if ((mode&3)==ALIG_CENTRE)
				p.x -= w*0.5;

			if ((mode&3)==ALIG_RIGHT)
				p.x -= w;
		}
		draw_line_thin(fb, sc_x(p.x), sc_y(p.y), sc_x(p.x+w), sc_y(p.y), drawing_thickness, colour, blend_add, intensity*3.);
	}
}
