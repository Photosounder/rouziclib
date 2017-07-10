int draw_vector_char(raster_t fb, vector_font_t *font, uint32_t c, xy_t p, xy_t off, double scale, col_t colour, double line_thick, const int mode, const int bidi)
{
	letter_t *l;
	double fixoff, wc1, wc2;
	unicode_data_t ucd;
	int found = 0;

	process_one_glyph(font, get_letter_index(font, c));

	l = get_letter(font, c);
	if (l)
	if (l->obj)
	{
		found = 1;
		fixoff = 0.;
		if ((mode&12)==MONOSPACE || ((mode&12)==MONODIGITS && c>='0' && c<='9'))
			fixoff = 0.5 * (4. - l->width);

		if (bidi == -2)
			fixoff -= l->br;
		else
			fixoff -= l->bl;

		draw_vobj(fb, l->obj, xy(p.x + off.x + fixoff*scale, p.y + off.y), scale, 0., line_thick, colour);
	}

	// Alias
	if (found==0 && l)
		if (l->alias)
			found |= draw_vector_char(fb, font, l->alias, p, off, scale, colour, line_thick, mode, bidi);

	// Combo
	if (found==0)
	{
		ucd = get_unicode_data(c);
		if (/*ucd.decomp_type == decomp_canonical &&*/ ucd.combo1)		// if we have a valid combo for this character
		{
			wc1 = glyph_width(font, off.x, ucd.combo1, scale, mode);

			found |= draw_vector_char(fb, font, ucd.combo1, p, off, scale, colour, line_thick, mode, bidi);

			if (ucd.uccat == uccat_Ll)	// if character is lowercase
			{
				wc2 = ((glyph_width(font, off.x, ucd.combo2, 1., mode) - LETTERSPACING) * LOWERCASESCALE + LETTERSPACING) * scale;
				scale *= LOWERCASESCALE;
			}
			else
				wc2 = glyph_width(font, off.x, ucd.combo2, scale, mode);

			if (bidi == -2)
				off.x -= (wc1 - wc2) * 0.5;
			else
				off.x += (wc1 - wc2) * 0.5;
			found |= draw_vector_char(fb, font, ucd.combo2, p, off, scale, colour, line_thick, mode, bidi);
		}
	}

	// Upper case
	if (found==0)
		if (ucd.upper_map)
			found |= draw_vector_char(fb, font, ucd.upper_map, p, off, scale*LOWERCASESCALE, colour, line_thick, mode, bidi);

	// Decomposed CJK
	if (found==0)
		found |= draw_cjkdec_glyph(fb, font, c, add_xy(p, off), scale, colour, line_thick, mode);

	return found;
}

int draw_vector_char_lookahead(raster_t fb, vector_font_t *font, uint32_t c, char *string, xy_t p, xy_t off, double scale, col_t colour, double line_thick, const int mode, const int bidi)
{
	letter_t *l;
	double fixoff, wc1, wc2;
	unicode_data_t ucd1, ucd2;
	int i=0, ir=0, found = 0;
	uint32_t cn;
	xy_t noff=off;
	double scale_mod=1., offb=0., offt=0.;
	letter_t *ldom;
	int lowerscale_dom, bidi1, onscreen=1;
	rect_t bound_box;

	bound_box = make_rect_off( add_xy(p, off), set_xy(24. * scale), xy(0.5, 0.5) );
	if (check_box_box_intersection(fb.window_dl, bound_box)==0)
		onscreen = 0;

	if (onscreen)
	{
		ucd1 = get_unicode_data(c);
		bidi1 = bidicat_direction(ucd1.bidicat);
		wc1 = glyph_width(font, off.x, c, scale, mode);

		draw_vector_char(fb, font, c, p, off, scale, colour, line_thick, mode, bidi);

		if (ucd1.uccat == uccat_Ll)	// if character is lowercase
			scale_mod = LOWERCASESCALE;
	}

	do
	{
		cn = utf8_to_unicode32(&string[i], &i);		// lookup next character
		ucd2 = get_unicode_data(cn);

		if (ucd2.bidicat==bidicat_NSM)			// non-spacing (combining) mark
		{
			ir = i+1;

			if (onscreen)
			{
				ldom = get_dominant_letter(font, cn, &lowerscale_dom);

				if (ucd1.uccat == uccat_Ll)	// if character is lowercase
					wc2 = ((glyph_width(font, off.x, cn, 1., mode) - LETTERSPACING) * LOWERCASESCALE + LETTERSPACING) * scale;
				else
					wc2 = glyph_width(font, off.x, cn, scale, mode);

				noff.x = off.x + (wc1 - wc2) * 0.5 * (bidi == -2 ? -1. : 1.);
				noff.y = off.y;

				if (ldom)
				{
					if (ldom->bb > 3. && ldom->bt > 6.)
						noff.y -= offt * scale*scale_mod;

					if (ldom->bt < 3. && ldom->bb < 0.)
						noff.y -= offb * scale*scale_mod;
				}

				draw_vector_char(fb, font, cn, p, noff, scale*scale_mod, colour, line_thick, mode, bidi);

				if (ldom)
				{
					if (ldom->bb > 3. && ldom->bt > 6.)
						offt += ldom->bt - 6.;

					if (ldom->bt < 3. && ldom->bb < 0.)
						offb += ldom->bb;
				}
			}
		}

		i++;
	}
	while (ucd2.bidicat==bidicat_NSM);

	return ir;
}

void cursor_processing(raster_t fb, vector_font_t *font, xy_t p, xy_t off, double scale, xy_t expected_pos, int is, int curpos, int bidi, int bidi_change, double line_thick)
{
	if (is==curpos)		// if the cursor is on the current character
	{
		draw_textedit_cursor(fb, add_xy(p, off), scale, bidi, bidi_change, line_thick);
		cur_textedit->cur_screen_pos = div_xy(off, set_xy(scale));
		fprintf_rl(stdout, "cur_screen_pos: %g | %g\n", cur_textedit->cur_screen_pos.x, cur_textedit->cur_screen_pos.y);
		fprintf_rl(stdout, "expected_pos: %g | %g\n", expected_pos.x, expected_pos.y);
		fprintf_rl(stdout, "cur_vert_mov: %d\n", cur_textedit->cur_vert_mov);
	}
}

void draw_string_full(raster_t fb, vector_font_t *font, char *string, xy_t p, xy_t off, double scale, col_t colour, double intensity, double line_thick, const int mode, int32_t len, double glyph_limit, double line_limit, const int bidi, const int recur, text_param_t *tp)
{
	uint32_t i, is, c, co;
	double w=0.;
	xy_t off_ls=XY0;
	int drawline=0;
	unicode_data_t ucd;
	int c_bidi, len_sec, con_prev=0, use_textedit=0, curpos, bidi_change=0;
	col_t colm;
	xy_t expected_pos, closest_pos;
	double closest_deltapos=FLT_MAX;
	int closest_is=0, newcursor_curline=0;

	if (font==NULL)
		return ;

	if (scale < line_limit)
		return ;

	if (scale < glyph_limit)
		drawline = 1;

	if (recur==0)
		intensity *= intensity_scaling(scale, 1.);

	if (((mode&3)!=ALIG_LEFT && bidi!=-2) || ((mode&3)!=ALIG_RIGHT && bidi==-2))
	{
		//w = calc_strwidth_len(font, string, scale, mode, len);
		w = calc_strwidth_firstline(font, string, scale, mode, len, NULL);

		if ((mode&3)==ALIG_CENTRE)
			if (bidi==-2)
				p.x += w*0.5;
			else
				p.x -= w*0.5;

		if ((mode&3)==ALIG_LEFT && bidi==-2)
			p.x += w;

		if ((mode&3)==ALIG_RIGHT && bidi!=-2)
			p.x -= w;
	}

	if (bidi==1)
	{
		if (w==0.)
			w = calc_strwidth_firstline(font, string, scale, mode, len, NULL);
		p.x -= w;
	}

	colm = colour;
	if (drawline==0)
		colm = colour_mul(colm, intensity);

	if (len < 0)
		len = strlen(string);

	if (cur_textedit)
		if (cur_textedit->string)
			if (string >= cur_textedit->string && string <= &cur_textedit->string[strlen(cur_textedit->string)])	// if the string belongs to text being edited
			{
				use_textedit = 1;
				curpos = &cur_textedit->string[cur_textedit->curpos] - string;

				if (curpos==0)
					draw_textedit_cursor(fb, add_xy(p, off), scale, bidi, bidi_change, line_thick);
			}

	if (use_textedit)
		if (cur_textedit->cur_vert_mov)
			expected_pos = add_xy( off , xy(0., LINEVSPACING * scale * cur_textedit->cur_vert_mov) );

	for (i=0; i<len; i++)
	{
		is = i;							// save i at start (it might get incremented right below)
		co = utf8_to_unicode32(&string[i], &i);			// get (original) codepoint and increment i
		c = get_arabic_form(co, &string[i+1], len-(i+1), con_prev);	// substitute for Arabic form if needed
		ucd = get_unicode_data(c);
		if (ucd.bidicat!=bidicat_NSM)				// if character that is not a combining mark
			con_prev = unicode_arabic_can_connect(co, 1);	// if the current character connects with the next (in Arabic)

		if (c > sc_start && c < sc_end)		// custom colour-setting Unicode characters
		{
			switch (c)
			{
				case sc_white:		colour = make_colour(1., 1., 1., 1.);		break;
				case sc_black:		colour = make_colour(0., 0., 0., 1.);		break;
				case sc_red:		colour = make_colour(1., 0., 0., 1.);		break;
				case sc_green:		colour = make_colour(0., 0.5, 0., 1.);		break;
				case sc_blue:		colour = make_colour(0., 0., 1., 1.);		break;
				case sc_baby_azure:	colour = make_colour(0.1, 0.3, 1., 1.);		break;
				case sc_amber:		colour = make_colour(1., 0.55, 0., 1.);		break;
				case sc_azure:		colour = make_colour(0., 0.45, 1., 1.);		break;
				case sc_grey:		colour = make_colour(0.184, 0.184, 0.184, 1.);	break;
				default:		colour = make_colour(0.184, 0.184, 0.184, 1.);
			}

			colm = colour_mul(colour, intensity);
		}
		else						// regular Unicode characters
		{
			// draw the line for the last word it represents
			if (drawline && recur==0)
			if (c==' ' || c=='\t' || c=='\n')	// if c is whitespace char
			if (equal_xy(off, off_ls)==0)
				draw_line_thin(fb, add_xy(add_xy(p, off_ls), xy(0., -2.5*scale)), add_xy(add_xy(p, off), xy(-LETTERSPACING*scale, -2.5*scale) ), line_thick, colour, blend_add, intensity*3.);

			c_bidi = bidicat_direction(ucd.bidicat);

			bidi_change = (bidi!=-2 && c_bidi==-2) || (bidi==-2 && c_bidi>0);

			if (use_textedit)
				cursor_processing(fb, font, p, off, scale, expected_pos, is, curpos, bidi, bidi_change, line_thick);

			if (use_textedit && newcursor_curline)
				if (fabs(off.x - expected_pos.x) < closest_deltapos)
				{
					closest_deltapos = fabs(off.x - expected_pos.x);
					cur_textedit->curpos = is + string - cur_textedit->string;
				}

			if (bidi_change)
			{
				len_sec = find_len_bidi_section(&string[is], len-is, c_bidi);
				draw_string_full(fb, font, &string[is], p, off, scale, colour, intensity, line_thick, mode, len_sec, glyph_limit, line_limit, c_bidi, 1, tp);
				off.x += (calc_strwidth_len(font, &string[is], scale, mode, len_sec) + LETTERSPACING * scale) * (bidi == -2 ? -1. : 1.);
				is += len_sec;
				i = is-1;
			}
			else switch (c)
			{
				case '\n':
					off.x = 0.;
					off.y += LINEVSPACING * scale;

					if (use_textedit)
					if (cur_textedit->cur_vert_mov)
					{
						newcursor_curline = 0;
						if (fabs(off.y - expected_pos.y) < 0.1)
							newcursor_curline = 1;

						if (newcursor_curline)	// if the previous line was the cursor's new line
						{
							cur_textedit->cur_vert_mov = 0;
							//expected_pos = XY0;
						}
					}
					break;

				default:
					if (drawline==0)
						i += draw_vector_char_lookahead(fb, font, c, &string[i+1], p, off, scale, colm, line_thick, mode, bidi);

					off.x += letter_width(font, off.x, c, scale, mode) * (bidi == -2 ? -1. : 1.);
			}

			if (use_textedit && i+1==curpos && curpos==len)	// if we're reaching the end of the string and the cursor is there 
				cursor_processing(fb, font, p, off, scale, expected_pos, i+1, curpos, bidi, bidi_change, line_thick);

			if (drawline)
			if (c==' ' || c=='\t' || c=='\n')	// if c is whitespace char
				off_ls = off;			// fix position in RTL
		}
	}

	if (drawline && recur==0)
	if (equal_xy(off, off_ls)==0)
		draw_line_thin(fb, add_xy(add_xy(p, off_ls), xy(0., -2.5*scale)), add_xy(add_xy(p, off), xy(-LETTERSPACING*scale, -2.5*scale) ), line_thick, colour, blend_add, intensity*3.);
}

void draw_string_len(raster_t fb, vector_font_t *font, char *string, xy_t p, double scale, col_t colour, double intensity, double line_thick, const int mode, int32_t len, text_param_t *tp)
{
	draw_string_full(fb, font, string, p, XY0, scale, colour, intensity, line_thick, mode, len, 0.2, 0.01, 2, 0, tp);
}

void draw_string(raster_t fb, vector_font_t *font, char *string, xy_t p, double scale, col_t colour, double intensity, double line_thick, const int mode, text_param_t *tp)
{
	draw_string_len(fb, font, string, p, scale, colour, intensity, line_thick, mode, -1, tp);
}

void print_to_screen_fullarg(raster_t fb, zoom_t zc, vector_font_t *font, xy_t pos, double scale, col_t colour, double intensity, const int32_t mode, double line_thick, const char *format, ...)
{
	va_list args;
	char string[4096];
	
	va_start (args, format);
	//vsnprintf (string, LOGLINEMAX, format, args);		// print new text to a
	vsprintf (string, format, args);		// print new text to a
	va_end (args);

	draw_string(fb, font, string, sc_xy(pos), scale*zc.scrscale, colour, intensity, line_thick, mode, NULL);
}
