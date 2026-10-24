const _Thread_local blend_func_t vector_font_blending_mode = blend_add;

// Glyph coverage cache: an outline glyph at a given size, antialiasing radius and position within the pixel (rounded to 1/64 px) is drawn once into a coverage map,
// which keeps the coverage draw_contours_lrgb() hands to the blending function for each pixel, and is then blended from that map, giving the same pixels as drawing it each time
#define GLYPH_CACHE_SLOTS	4096		// power of 2, the cache starts over when 3/4 full
#define GLYPH_CACHE_MAX_AREA	65536		// larger glyphs (when zoomed in) are drawn directly
#define GLYPH_CACHE_MAX_BYTES	(32<<20)	// the cache starts over beyond this
#ifndef CONTOURS_FAR
#define CONTOURS_FAR 3.25
#endif

typedef struct
{
	contours_t *c;		// key: the glyph's outline, its scale, the antialiasing radius and the position within the pixel in 64ths
	double scale, radius;
	int fx, fy;
	int x0, y0, w, h;	// the coverage map's offset from the glyph's whole pixel position, and its size
	uint16_t *cov;		// coverage (1.15 format) for each pixel, 0 where nothing is drawn
} glyph_cache_entry_t;

static _Thread_local glyph_cache_entry_t *glyph_cache = NULL;
static _Thread_local size_t glyph_cache_count = 0, glyph_cache_bytes = 0;

void glyph_cache_clear(void)
{
	// Forget every cached glyph, needed when a font is freed as its outlines' addresses could be reused
	if (glyph_cache)
		for (size_t i=0; i < GLYPH_CACHE_SLOTS; i++)
			free(glyph_cache[i].cov);
	free(glyph_cache);
	glyph_cache = NULL;
	glyph_cache_count = glyph_cache_bytes = 0;
}

static void blend_capture_coverage(lrgb_t *bg, lrgb_t fg, int32_t p)
{
	// Keeps the coverage handed to the blending function instead of blending
	bg->r = p;
}

static size_t glyph_cache_slot(contours_t *c, double scale, double radius, int fx, int fy)
{
	// Hash of the key
	uint64_t h = (uint64_t) (size_t) c * 0x9E3779B97F4A7C15ULL;
	h ^= double_as_u64(scale) + 0x632BE59BD9B4E019ULL + (h << 6) + (h >> 2);
	h ^= double_as_u64(radius) + 0x85EBCA77C2B2AE63ULL + (h << 6) + (h >> 2);
	h ^= (uint64_t) (fx * 64 + fy) * 0xC2B2AE3D27D4EB4FULL;
	return (h ^ (h >> 29)) & (GLYPH_CACHE_SLOTS-1);
}

static void draw_glyph_outline(contours_t *c, xy_t pos, double scale, double line_thick, col_t colour)
{
	static _Thread_local xy_t *p=NULL;
	static _Thread_local lrgb_t *scratch_l=NULL;
	static _Thread_local size_t p_as=0, scratch_as=0;
	int i, p_count = c->cont_start[c->cont_count];
	double radius, margin;
	rect_t ext;

	if (fb->discard || p_count < 3)
		return;

	// The glyph's extent around its position, its outline placed the way draw_vobj_tri() places the mesh (y going down), plus the antialiasing's reach, as draw_contours() would use them
	radius = drawing_focus_adjust(focus_rlg, line_thick, NULL, 0);
	margin = CONTOURS_FAR * radius;
	ext.p0 = ext.p1 = neg_y(mul_xy(c->p[0], set_xy(scale)));
	for (i=1; i < p_count; i++)
	{
		xy_t q = neg_y(mul_xy(c->p[i], set_xy(scale)));
		ext.p0 = min_xy(ext.p0, q);
		ext.p1 = max_xy(ext.p1, q);
	}
	ext = rect_add_margin(ext, set_xy(margin));

	// Nothing to do off screen
	if (pos.x + ext.p1.x < 0. || pos.y + ext.p1.y < 0. || pos.x + ext.p0.x > fb->w || pos.y + ext.p0.y > fb->h)
		return;

	// Large glyphs are drawn directly at their exact position
	if ((ext.p1.x - ext.p0.x + 4.) * (ext.p1.y - ext.p0.y + 4.) > GLYPH_CACHE_MAX_AREA)
	{
		alloc_enough(&p, p_count, &p_as, sizeof(xy_t), 1.5);
		for (i=0; i < p_count; i++)
			p[i] = add_xy(neg_y(mul_xy(c->p[i], set_xy(scale))), pos);
		draw_contours_lrgb(p, c->cont_start, c->cont_count, radius, col_to_lrgb(colour), blend_add, 1.);
		return;
	}

	// Position split into whole pixels and 64ths of a pixel
	double ox = floor(pos.x), oy = floor(pos.y);
	int fx = nearbyint((pos.x - ox) * 64.), fy = nearbyint((pos.y - oy) * 64.);
	if (fx == 64) { ox += 1.; fx = 0; }
	if (fy == 64) { oy += 1.; fy = 0; }
	xy_t sub = xy(fx / 64., fy / 64.);

	// Look the glyph up, starting over when the cache is too full
	if (glyph_cache == NULL || glyph_cache_count >= GLYPH_CACHE_SLOTS*3/4 || glyph_cache_bytes >= GLYPH_CACHE_MAX_BYTES)
	{
		glyph_cache_clear();
		glyph_cache = calloc(GLYPH_CACHE_SLOTS, sizeof(glyph_cache_entry_t));
	}
	size_t slot = glyph_cache_slot(c, scale, radius, fx, fy);
	glyph_cache_entry_t *e;
	while (1)
	{
		e = &glyph_cache[slot];
		if (e->c == NULL || (e->c == c && e->scale == scale && e->radius == radius && e->fx == fx && e->fy == fy))
			break;
		slot = (slot + 1) & (GLYPH_CACHE_SLOTS-1);
	}

	// A new glyph is drawn into a scratch framebuffer large enough to hold every pixel draw_contours_lrgb() visits, keeping the coverage of each
	if (e->c == NULL)
	{
		e->c = c;
		e->scale = scale;
		e->radius = radius;
		e->fx = fx;
		e->fy = fy;
		e->x0 = floor(sub.x + ext.p0.x) - 1;
		e->y0 = floor(sub.y + ext.p0.y) - 1;
		e->w = (int) ceil(sub.x + ext.p1.x) - e->x0 + 2;
		e->h = (int) ceil(sub.y + ext.p1.y) - e->y0 + 2;
		size_t n = (size_t) e->w * e->h;

		alloc_enough(&scratch_l, n, &scratch_as, sizeof(lrgb_t), 1.5);
		memset(scratch_l, 0, n * sizeof(lrgb_t));
		alloc_enough(&p, p_count, &p_as, sizeof(xy_t), 1.5);
		for (i=0; i < p_count; i++)
			p[i] = add_xy(neg_y(mul_xy(c->p[i], set_xy(scale))), xy(sub.x - e->x0, sub.y - e->y0));

		framebuffer_t scratch = *fb, *fb_saved = fb;
		scratch.r.l = scratch_l;
		scratch.r.dim = xyi(e->w, e->h);
		scratch.w = e->w;
		scratch.h = e->h;
		fb = &scratch;
		draw_contours_lrgb(p, c->cont_start, c->cont_count, radius, col_to_lrgb(colour), blend_capture_coverage, 1.);
		fb = fb_saved;

		e->cov = malloc(n * sizeof(uint16_t));
		for (size_t k=0; k < n; k++)
			e->cov[k] = scratch_l[k].r;
		glyph_cache_count++;
		glyph_cache_bytes += n * sizeof(uint16_t);
	}

	// Blend the coverage map on screen as draw_contours() would have
	lrgb_t lc = col_to_lrgb(colour);
	int bx = (int) ox + e->x0, by = (int) oy + e->y0;
	int ix0 = MAXN(0, -bx), ix1 = MINN(e->w, fb->w - bx), iy0 = MAXN(0, -by), iy1 = MINN(e->h, fb->h - by);
	for (int iy=iy0; iy < iy1; iy++)
	{
		uint16_t *cov = &e->cov[iy * e->w];
		lrgb_t *row = &fb->r.l[(size_t) (by + iy) * fb->w + bx];
		for (int ix=ix0; ix < ix1; ix++)
			if (cov[ix])
				blend_add(&row[ix], lc, cov[ix]);
	}
}

int draw_vector_char(vector_font_t *font, uint32_t c, xy_t p, xy_t off, double scale, col_t col_thin, col_t col_poly, double line_thick, const int mode, const int bidi)
{
	letter_t *l;
	double fixoff, wc1, wc2;
	unicode_data_t ucd;
	int found = 0;

	if (c == 0)
		return found;

	// Algorithmic substitution
	if (bidi == -2)
		c = substitute_rtl_punctuation(c);

	process_one_glyph(font, get_letter_index(font, c));

	l = get_letter(font, c);
	if (l)
	if (l->obj || l->tri_mesh.count)
	{
		found = 1;
		fixoff = 0.;
		if ((mode&12)==MONOSPACE || ((mode&12)==MONODIGITS && c>='0' && c<='9'))
			fixoff = 0.5 * (4. - l->width);

		if (bidi == -2)
			fixoff -= l->br;
		else
			fixoff -= l->bl;

		xy_t pos = xy(p.x + off.x + fixoff*scale, p.y + off.y);

		if (l->obj)
			if (fb->use_dqnq)
				draw_vobj_dqnq(l->obj, pos, scale, 0., line_thick, col_thin);
			else
				draw_vobj(l->obj, pos, scale, 0., line_thick, col_thin);
		else
			if (l->outline.cont_count && font->use_outline && fb->use_drawq == 0 && fb->r.use_frgb == 0)
				draw_glyph_outline(&l->outline, pos, scale, line_thick, col_poly);
			else if (l->polynomial_grid.cell && font->use_mesh == 0)
				draw_polynomial_grid(&l->polynomial_grid, pos, scale, 0., col_poly, vector_font_blending_mode);
			else
				draw_vobj_tri(l->tri_mesh, pos, scale, 0., line_thick, col_poly);
	}

	// Alias
	if (found==0 && l)
		if (l->alias)
			found |= draw_vector_char(font, l->alias, p, off, scale, col_thin, col_poly, line_thick, mode, bidi);

	// Combo
	if (found==0)
	{
		ucd = get_unicode_data(c);
		if (/*ucd.decomp_type == decomp_canonical &&*/ ucd.combo1)		// if we have a valid combo for this character
		{
			wc1 = glyph_width(font, off.x, ucd.combo1, scale, mode);

			found |= draw_vector_char(font, ucd.combo1, p, off, scale, col_thin, col_poly, line_thick, mode, bidi);

			if (ucd.uccat == uccat_Ll)	// if character is lowercase
			{
				wc2 = ((glyph_width(font, off.x, ucd.combo2, 1., mode) - font->letter_spacing) * LOWERCASESCALE + font->letter_spacing) * scale;
				scale *= LOWERCASESCALE;
			}
			else
				wc2 = glyph_width(font, off.x, ucd.combo2, scale, mode);

			if (bidi == -2)
				off.x -= (wc1 - wc2) * 0.5;
			else
				off.x += (wc1 - wc2) * 0.5;
			found |= draw_vector_char(font, ucd.combo2, p, off, scale, col_thin, col_poly, line_thick, mode, bidi);
		}
	}

	// Upper case
	if (found==0)
		if (ucd.upper_map)
			found |= draw_vector_char(font, ucd.upper_map, p, off, scale*LOWERCASESCALE, col_thin, col_poly, line_thick, mode, bidi);

	// Decomposed CJK
	//if (found==0)
	//	found |= draw_cjkdec_glyph(font, c, add_xy(p, off), scale, colour, line_thick, mode);

	return found;
}

int draw_vector_char_lookahead(vector_font_t *font, uint32_t c, const char *string, xy_t p, xy_t *off, double scale, col_t col_thin, col_t col_poly, double line_thick, const int mode, const int bidi)
{
	letter_t *l;
	double fixoff, wc1, wc2;
	unicode_data_t ucd1, ucd2;
	size_t i=0;
	int ir=0, found = 0;
	uint32_t cn;
	xy_t noff=*off;
	double scale_mod=1., offb=0., offt=0.;
	letter_t *ldom;
	int lowerscale_dom, bidi1, onscreen=1;
	rect_t bound_box;

	bound_box = make_rect_off( add_xy(p, *off), set_xy(24. * scale), xy(0.5, 0.5) );
	if (check_box_box_intersection(fb->window_dl, bound_box)==0)
		onscreen = 0;

	if (onscreen)
	{
		ucd1 = get_unicode_data(c);
		//bidi1 = bidicat_direction(ucd1.bidicat);
		wc1 = glyph_width(font, off->x, c, scale, mode);

		draw_vector_char(font, c, p, *off, scale, col_thin, col_poly, line_thick, mode, bidi);

		if (ucd1.uccat == uccat_Ll)	// if character is lowercase
			scale_mod = LOWERCASESCALE;
	}

	// Check for insert rect sequence
	if (c >= cp_ins_start && c < cp_ins_end)	// if c is a custom spacing character and cn is a variation selector that indicates its index
	{
		ir = parse_insert_rect_charseq(p, off, scale, bidi, c, string);
		return ir;
	}

	do	// loop through all following non-spacing (combining) marks
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
					wc2 = ((glyph_width(font, off->x, cn, 1., mode) - font->letter_spacing) * LOWERCASESCALE + font->letter_spacing) * scale;
				else
					wc2 = glyph_width(font, off->x, cn, scale, mode);

				noff.x = off->x + (wc1 - wc2) * 0.5 * (bidi == -2 ? -1. : 1.);
				noff.y = off->y;

				if (ldom)
				{
					if (ldom->bb > 3. && ldom->bt > 6.)
						noff.y -= offt * scale*scale_mod;

					if (ldom->bt < 3. && ldom->bb < 0.)
						noff.y -= offb * scale*scale_mod;
				}

				draw_vector_char(font, cn, p, noff, scale*scale_mod, col_thin, col_poly, line_thick, mode, bidi);

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

void check_closest_cursor(xy_t off, double scale, xy_t expected_pos, double *closest_deltapos, int isa, int *curpos)
{
	if (fabs(off.y - expected_pos.y) <= font->line_vspacing*scale*0.5)
		if (fabs(off.x - expected_pos.x) < *closest_deltapos)
		{
			*closest_deltapos = fabs(off.x - expected_pos.x);
			*curpos = isa;
		}
}

col_t text_sel_col={0};

void cursor_processing(vector_font_t *font, const char *string, uint32_t c, xy_t p, xy_t off, double scale, xy_t expected_pos, int is, int curpos, int recur, const int mode, int bidi, int bidi_change, double line_thick, double *closest_deltapos)
{
	static int32_t sel0=0, sel1=0, newline=0;
	static xy_t p0, p1;
	rect_t box;
	int isa = is + string - cur_textedit->string;
	double wc1;

	if (is0_col(text_sel_col))
		text_sel_col = make_colour(0.0033, 0.028, 0.1, 1.);

	if (recur==0 /*&& is==0*/)		// if it's the first run of this function for this string in this loop FIXME 'is' doesn't get to be 0 if the string starts with colours
	{
		sel0 = cur_textedit->sel0;
		sel1 = cur_textedit->sel1;
		minmax_i32(&sel0, &sel1);
		newline = 0;
	}

	if (recur > 0 && is==0)
		newline = 1;

	if (newline)
	{
		newline = 0;
		p0 = add_xy(p, off);
	}

	if (sel0 < sel1)		// if there's a selection to display
	{
		if (sel0==isa)
			p0 = add_xy(p, off);

		if (sel1==isa || (c=='\n' && isa > sel0))
		{
			p1 = add_xy(p, off);

			box.p0 = add_xy(p0, xy(-font->letter_spacing*0.5*scale * (bidi == -2 ? -1. : 1.), 2.*scale));
			box.p1 = add_xy(p1, xy(-font->letter_spacing*0.5*scale * (bidi == -2 ? -1. : 1.), -8.*scale));
		//	draw_rect_full(box, line_thick, text_sel_col, 1.);

			if (c=='\n')
				newline = 1;

			if (sel1==isa)
				sel0 = sel1 = 0;
		}
	}

	if (isa >= sel0 && isa <= sel1 && sel0 < sel1 && bidi_change==0)		// display selection one character at a time
	{
		wc1 = glyph_width(font, off.x, c, scale, mode);
		box.p0 = add_xy(add_xy(p, off), xy(-font->letter_spacing*0.5*scale * (bidi == -2 ? -1. : 1.), 2.*scale));
		box.p1 = add_xy(box.p0, xy(wc1 * (bidi == -2 ? -1. : 1.), -10.*scale));
		draw_rect_full(box, line_thick, text_sel_col, cur_blend, 1.);
	}

	if (cur_textedit->click_on==0 && is==curpos)		// if the cursor is on the current character
	{
		if (cur_textedit->read_only==0)
			draw_textedit_cursor(add_xy(p, off), scale, bidi, bidi_change, line_thick);
		cur_textedit->cur_screen_pos = div_xy(off, set_xy(scale));
	}

	if (cur_textedit->click_on)
		check_closest_cursor(off, scale, expected_pos, &closest_deltapos[0], isa, &cur_textedit->curpos);

	// find the position in the line above and below
	expected_pos = mad_xy(cur_textedit->cur_screen_pos_prev, set_xy(scale), xy(0., font->line_vspacing * -scale));
	expected_pos = mad_xy(xy(0.5*font->letter_spacing * (bidi == -2 ? -1. : 1.), 3.), set_xy(scale), expected_pos);
	//expected_pos.y = MAXN(0., expected_pos.y);
	check_closest_cursor(off, scale, expected_pos, &closest_deltapos[1], isa, &cur_textedit->curpos_up);

	expected_pos = mad_xy(cur_textedit->cur_screen_pos_prev, set_xy(scale), xy(0., font->line_vspacing * scale));
	expected_pos = mad_xy(xy(0.5*font->letter_spacing * (bidi == -2 ? -1. : 1.), 3.), set_xy(scale), expected_pos);
	check_closest_cursor(off, scale, expected_pos, &closest_deltapos[2], isa, &cur_textedit->curpos_down);
}

void draw_string_full(vector_font_t *font, const char *string, xy_t p, xy_t off, double scale, col_t colour, double intensity, double line_thick, const int mode, int32_t len, double glyph_limit, double line_limit, const int bidi, const int recur, text_param_t *tp)
{
	size_t i, is;
	uint32_t c, co;
	double w=0., base_off=0.;
	xy_t off_ls=XY0;
	int drawline=0;
	unicode_data_t ucd;
	int c_bidi, len_sec, con_prev=0, use_textedit=0, curpos, bidi_change=0;
	col_t col_thin, col_poly;
	xy_t expected_pos=XY0;
	static _Thread_local double closest_deltapos[3];

	if (recur==0)		// if it's the top recursion of the function for this string
		for (i=0; i<3; i++)
			closest_deltapos[i] = FLT_MAX;

	if (font==NULL)
		return ;

	if (scale < line_limit)
		return ;

	if (scale < glyph_limit)
		drawline = 1;

	if (((mode&3)!=ALIG_LEFT && bidi!=-2) || ((mode&3)!=ALIG_RIGHT && bidi==-2))
	{
		//w = calc_strwidth_len(font, string, scale, mode, len);
		w = calc_strwidth_firstline(font, string, scale, mode, len, NULL);

		if ((mode&3)==ALIG_CENTRE)
			if (bidi==-2)
				base_off += w*0.5;
			else
				base_off -= w*0.5;

		if ((mode&3)==ALIG_LEFT && bidi==-2)
			base_off += w;

		if ((mode&3)==ALIG_RIGHT && bidi!=-2)
			base_off -= w;
	}

	if (bidi==1)
	{
		if (w==0.)
			w = calc_strwidth_firstline(font, string, scale, mode, len, NULL);
		base_off -= w;
	}

	off.x += base_off;
	off_ls.x = off.x;

	// Apply colour intensities
	double scaled_intensity = intensity * intensity_scaling(scale, 2.);
	col_poly = colour_mul(colour, intensity);
	col_thin = colour_mul(colour, scaled_intensity);

	if (len < 0)
		len = strlen(string);

	if (cur_textedit)
		if (cur_textedit->string)
			if (string >= cur_textedit->string && string <= &cur_textedit->string[strlen(cur_textedit->string)])	// if the string belongs to text being edited
			{
				use_textedit = 1;
				curpos = &cur_textedit->string[cur_textedit->curpos] - string;

				if (curpos==0 && cur_textedit->click_on==0 && recur==0 && len==0)	// draw cursor if the string is empty
					draw_textedit_cursor(add_xy(p, off), scale, bidi, bidi_change, line_thick);

				if (cur_textedit->click_on)
				{
					expected_pos = mad_xy(xy(0.5*font->letter_spacing * (bidi == -2 ? -1. : 1.), 3.), set_xy(scale), sub_xy(cur_textedit->click, p));
					expected_pos.y = MAXN(0., expected_pos.y);
					if (recur==0)
						cur_textedit->curpos = strlen(cur_textedit->string);
				}
			}

	for (i=0; i<len; i++)
	{
		is = i;							// save i at start (it might get incremented right below)
		co = utf8_to_unicode32(&string[i], &i);			// get (original) codepoint and increment i
		if (co == 0xFFFFFFFF)
			break;

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

			col_poly = colour_mul(colour, intensity);
			col_thin = colour_mul(colour, scaled_intensity);
		}
		else						// regular Unicode characters
		{
			// draw the line for the last word it represents
			if (drawline && recur==0)
			if (c==' ' || c=='\t' || c=='\n')	// if c is whitespace char
			if (equal_xy(off, off_ls)==0)
				draw_line_thin(add_xy(add_xy(p, off_ls), xy(0., -2.5*scale)), add_xy(add_xy(p, off), xy(-font->letter_spacing*scale, -2.5*scale) ), line_thick, col_thin, cur_blend, 3.);

			c_bidi = bidicat_direction(ucd.bidicat);

			bidi_change = (bidi!=-2 && c_bidi==-2) || (bidi==-2 && c_bidi>0);

			if (use_textedit)
				cursor_processing(font, string, c, p, off, scale, expected_pos, is, curpos, recur, mode, bidi, bidi_change, line_thick, closest_deltapos);

			if (bidi_change)
			{
				len_sec = find_len_bidi_section(&string[is], len-is, c_bidi);
				draw_string_full(font, &string[is], p, off, scale, colour, intensity, line_thick, mode, len_sec, glyph_limit, line_limit, c_bidi, recur+1, tp);
				off.x += (calc_strwidth_len(font, &string[is], scale, mode, len_sec) + font->letter_spacing * scale) * (bidi == -2 ? -1. : 1.);
				is += len_sec;
				i = is-1;
			}
			else switch (c)
			{
				case '\n':
					off.x = base_off;
					off.y += font->line_vspacing * scale;
					break;

				default:
					if (drawline==0)
						i += draw_vector_char_lookahead(font, c, &string[i+1], p, &off, scale, col_thin, col_poly, line_thick, mode, bidi);

					off.x += letter_width(font, off.x, c, scale, mode) * (bidi == -2 ? -1. : 1.);
			}

			if (use_textedit && i+1==len)	// if we're reaching the end of the string
				cursor_processing(font, string, 0, p, off, scale, expected_pos, i+1, curpos, recur, mode, bidi, bidi_change, line_thick, closest_deltapos);

			if (drawline)
			if (c==' ' || c=='\t' || c=='\n')	// if c is whitespace char
				off_ls = off;			// fix position in RTL
		}
	}

	if (drawline && recur==0)
	if (equal_xy(off, off_ls)==0)
		draw_line_thin(add_xy(add_xy(p, off_ls), xy(0., -2.5*scale)), add_xy(add_xy(p, off), xy(-font->letter_spacing*scale, -2.5*scale) ), line_thick, col_thin, cur_blend, 3.);
}

void draw_string_len(vector_font_t *font, const char *string, xy_t p, double scale, col_t colour, double intensity, double line_thick, const int mode, int32_t len, text_param_t *tp)
{
	draw_string_full(font, string, p, XY0, scale, colour, intensity, line_thick, mode, len, 0.2, 0.01, 2, 0, tp);
}

void draw_string(vector_font_t *font, const char *string, xy_t p, double scale, col_t colour, double intensity, double line_thick, const int mode, text_param_t *tp)
{
	draw_string_len(font, string, p, scale, colour, intensity, line_thick, mode, -1, tp);
}

void print_to_screen(xy_t pos, double scale, col_t colour, double intensity, const int32_t mode, const char *format, ...)
{
	va_list args;
	static buffer_t string={0};

	va_start(args, format);
	clear_buf(&string);
	vbufprintf(&string, format, args);
	va_end(args);

	draw_string(font, string.buf, sc_xy(pos), scale*zc.scrscale, colour, intensity, drawing_thickness, mode, NULL);
}
