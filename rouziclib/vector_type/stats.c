double letter_width(vector_font_t *font, double pos, uint32_t c, double scale, const int mode)	// includes inter-letter spacing
{
	double tabx = 0., wc1, wc2;
	letter_t *l;
	unicode_data_t ucd;

	if (c > 0x10FFFF)
		return 0.;

	if (c=='\t')
	{
		while (tabx < pos + LETTERSPACING*scale)
			tabx += 8.*(4.+LETTERSPACING) * scale;

		return tabx - pos;
	}

	l = get_letter(font, c);
	if (l)
	if (l->obj)
		if ((mode&4)==PROPORTIONAL)
			return (l->width + LETTERSPACING) * scale;
		else
			return (4. + LETTERSPACING) * scale;

	l = get_uppercase_letter(font, c);
	if (l)
	if (l->obj)
		if ((mode&4)==PROPORTIONAL)
			return (l->width*LOWERCASESCALE + LETTERSPACING) * scale;
		else
			return (4. + LETTERSPACING) * scale;

	ucd = get_unicode_data(c);
	if (ucd.decomp_type == decomp_canonical && ucd.combo1)		// if we have a valid combo for this character
	{
		wc1 = letter_width(font, pos, ucd.combo1, scale, mode);
		wc2 = letter_width(font, pos, ucd.combo2, scale, mode);

		return wc1;
	}

	return 0.;
}

double calc_strlen_len(vector_font_t *font, uint8_t *string, double scale, const int mode, int32_t len)
{
	int32_t i;
	double w = 0.;

	if (len < 0)
		len = strlen(string);

	for (i=0; i<len; i++)
		w += letter_width(font, w, utf8_to_unicode32(&string[i], &i), scale, mode);

	w -= LETTERSPACING * scale;	// removes the end space

	return w;
}

double calc_strlen(vector_font_t *font, uint8_t *string, double scale, const int mode)
{
	return calc_strlen_len(font, string, scale, mode, -1);
}

word_stats_t make_word_stats(vector_font_t *font, uint8_t *string, const int mode)
{
	int i, iw, prev_was_space=1, len = strlen(string);
	uint32_t c;
	word_stats_t ws;

	ws.full_length = calc_strlen(font, string, 1., mode);

	// count spaces (and therefore words)
	for (ws.word_count=1, i=0; i<strlen(string); i++)
		if (string[i]==' ')
			ws.word_count++;

	ws.word_length = calloc(ws.word_count, sizeof(double));
	ws.word_start = calloc(ws.word_count, sizeof(int));
	ws.word_end = calloc(ws.word_count, sizeof(int));

	ws.aver_word_length = (ws.full_length - (double) (ws.word_count-1)*(letter_width(font, 0., ' ', 1., mode)+LETTERSPACING)) / (double) ws.word_count;

	// find individual word length and the final length sum of all the lines
	ws.max_word_length = 0.;
	for (iw=0, i=0; i<len; i++)
	{
		c = utf8_to_unicode32(&string[i], &i);

		if (c==' ')
		{
			ws.word_length[iw] -= LETTERSPACING;	// removes the end space
			ws.max_word_length = MAXN(ws.max_word_length, ws.word_length[iw]);
			iw++;
			prev_was_space = 1;
		}
		else
		{
			ws.word_length[iw] += letter_width(font, 0., c, 1., mode);
			if (i == len-1)		// if we've reached the end
			{
				ws.word_length[iw] -= LETTERSPACING;	// removes the end space
				ws.max_word_length = MAXN(ws.max_word_length, ws.word_length[iw]);
			}

			ws.word_end[iw] = i;

			if (prev_was_space)
				ws.word_start[iw] = i;
			prev_was_space = 0;
		}
	}

	return ws;
}

void free_word_stats(word_stats_t ws)
{
	free (ws.word_length);
	free (ws.word_start);
	free (ws.word_end);
}

// gives to correct word length to use for adding to the length of the line starting at iw_start
double get_word_length(vector_font_t *font, uint8_t *string, word_stats_t ws, const int mode, int iw_start, int iw)
{
	const double space_width = letter_width(font, 0., ' ', 1., mode);
	double width;

	width = ws.word_length[iw];

	if (iw>iw_start)
		width += space_width + LETTERSPACING;

	return width;
}
