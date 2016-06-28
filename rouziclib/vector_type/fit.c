// finds a line that fits within thresh and returns the end word index and the actual line width
double find_line_for_thresh(vector_font_t *font, uint8_t *string, word_stats_t ws, const int mode, double thresh, int iw_start, int *iw_end)
{
	double line_length=0., ret=0.;
	int iw;

	for (iw=iw_start; line_length <= thresh && iw < ws.word_count; iw++)
	{
		line_length += get_word_length(font, string, ws, mode, iw_start, iw);

		if (line_length <= thresh)
		{
			*iw_end = iw;
			ret = line_length;
		}
	}

	return ret;
}

// finds the number of lines that the string will be broken down to for a certain thresh
int find_line_count_for_thresh(vector_font_t *font, uint8_t *string, word_stats_t ws, const int mode, double thresh, double *maxwidth)
{
	int iw, iw_end=0, line_count = 1;
	double width;

	if (maxwidth)
		*maxwidth = 0.;

	for (iw=0; iw < ws.word_count; )
	{
		width = find_line_for_thresh(font, string, ws, mode, thresh, iw, &iw_end);	// changes iw_end to the corrent ending word
		if (maxwidth)
			*maxwidth = MAXN(*maxwidth, width);
		iw = iw_end + 1;

		if (iw < ws.word_count)		// if there are still words left after this line
			line_count++;
	}

	return line_count;
}

int search_thresh(vector_font_t *font, search_thresh_t *st, double maxwidth, int nlines, double prec)
{
	double step;

	if (st->n > nlines)		// if we found a new minimum
	{
		st->min = st->thresh;
		st->min_n = st->n;
	}
	else				// or we found a new maximum
	{
		st->max = maxwidth;
		st->max_n = st->n;
	}

	step = 0.5 * (st->max - st->min);
	step = MINN(step, 7.5);		// cap by an arbitrary limit
	if (step < prec)
		return 1;		// stop the search

	if (st->n > nlines)		// if we found a new minimum
		st->thresh += step;
	else				// or we found a new maximum
		st->thresh -= step;

	return 0;			// keep the search going
}

// find best threshold for fitting a string into nlines
double find_string_width_for_nlines(vector_font_t *font, uint8_t *string, word_stats_t ws, int *nlines, const int mode, double *lower_bound)	// doesn't handle \n or \t
{
	const double space_width = letter_width(font, 0., ' ', 1., mode);
	double maxwidth, total_end_length;
	search_thresh_t st;

	if (*nlines==1)
		return ws.full_length;

	if (ws.word_count==1)
		return ws.full_length;

	total_end_length = ws.full_length - (space_width+LETTERSPACING)*(*nlines-1);	// remove the widths of the spaces that will be turned into line breaks

	*lower_bound = total_end_length / (double) *nlines;	// the threshold couldn't possibly be lower than this
	*lower_bound = MAXN(*lower_bound, ws.max_word_length);	// or this

	memset(&st, 0, sizeof(search_thresh_t));
	st.min = *lower_bound;
	st.max = total_end_length;
	st.thresh = *lower_bound + 0.5*ws.aver_word_length;

	while (1)
	{
		st.n = find_line_count_for_thresh(font, string, ws, mode, st.thresh, &maxwidth);

		if (search_thresh(font, &st, maxwidth, *nlines, 0.5))
			break;
	}

	*nlines = st.max_n;
	return st.max;
}

// finds best width by testing different line counts
double find_best_string_width(vector_font_t *font, uint8_t *string, word_stats_t ws, const int mode, xy_t boxdim, int *nlines, double *scale_ratio)
{
	int i, prev_nlines, nl_start;
	double lower_bound=-1., thresh=0., prev_thresh, scale=0., prev_scale;

	if (ws.word_count==1)
	{
		*nlines = 1;
		*scale_ratio = MINN(boxdim.x / ws.full_length, boxdim.y / LINEVSPACING);	// find the scale needed for the text to fit the box with this many lines
		*scale_ratio = MINN(1., *scale_ratio);
		return ws.full_length;
	}

	if (boxdim.x!=0. && boxdim.y!=0.)
		nl_start = sqrt(ws.full_length / (boxdim.x*boxdim.y/LINEVSPACING)) * boxdim.y/LINEVSPACING;	// good starting point
	else
		nl_start = 1;

	nl_start = MAXN(1, MINN(nl_start, ws.word_count - 2));

	boxdim = abs_xy(boxdim);
	if (boxdim.x == 0.)
		boxdim.x = FLT_MAX;
	if (boxdim.y == 0.)
		boxdim.y = FLT_MAX;

	*scale_ratio = 1.;

	for (i=nl_start; lower_bound < thresh && i<=ws.word_count; i++)
	{
		prev_thresh = thresh;
		prev_scale = scale;
		prev_nlines = *nlines;
		*nlines = i;

		thresh = find_string_width_for_nlines(font, string, ws, nlines, mode, &lower_bound);

		if (thresh <= boxdim.x && *nlines*LINEVSPACING <= boxdim.y)		// if it fits without needing to lower the scale
			return thresh;

		scale = MINN(boxdim.x / thresh, boxdim.y / (*nlines*LINEVSPACING));	// find the scale needed for the text to fit the box with this many lines

		if (scale < prev_scale && *nlines > prev_nlines)			// if the previous attempt was the most suitable one
		{
			*nlines = prev_nlines;
			*scale_ratio = prev_scale;
			return prev_thresh;
		}
	}

	*scale_ratio = scale;
	return thresh;		// happens only if the longest line is a single word
}

void draw_string_maxwidth(vector_font_t *font, uint8_t *string, word_stats_t ws, xy_t box0, xy_t box1, double scale, lrgb_t colour, double intensity, const int mode, double maxwidth, int nlines)
{
	int i, iw, il, len, line_start=0, line_end=0, line_iw_start=0, line_iw_end=0, new_lw;
	const double space_width = letter_width(font, 0., ' ', 1., mode);
	double line_width;
	xy_t p;

	p.x = MINN(box0.x, box1.x);

	if ((mode&3)!=ALIG_LEFT)
	{
		if ((mode&3)==ALIG_CENTRE)
			p.x += (box1.x - box0.x) * 0.5;

		if ((mode&3)==ALIG_RIGHT)
			p.x += box1.x - box0.x;
	}

	p.y = 0.5*(box0.y+box1.y) - 3.*scale;				// puts the text right in the vertical middle of the box
	p.y += (double) (nlines-1) * 0.5 * LINEVSPACING * scale;	// shift it up depending on the number of lines

	for (iw=0; iw < ws.word_count; )
	{
		find_line_for_thresh(font, string, ws, mode, maxwidth, iw, &line_iw_end);	// changes line_iw_end to the corrent ending word

		line_start = ws.word_start[iw];
		line_end = ws.word_end[line_iw_end];
		len = 1 + line_end - line_start;
		draw_string_nolimit(font, &string[line_start], p, scale, colour, intensity, mode, len);

		iw = line_iw_end + 1;

		p.y -= LINEVSPACING * scale;	// move it down one line
	}
}

void draw_string_bestfit(vector_font_t *font, uint8_t *string, xy_t box0, xy_t box1, const double border, const double scale, lrgb_t colour, double intensity, const int mode)
{
	word_stats_t ws;
	double thresh, box_thresh, scale_ratio, total_scale;
	int nlines;
	xy_t boxdim, p;

	ws = make_word_stats(font, string, mode);

	box1 = sub_xy(box1, set_xy(border*scale));		// remove the border FIXME wrongly assumes correct order
	box0 = add_xy(box0, set_xy(border*scale));
	boxdim = abs_xy(div_xy(sub_xy(box1, box0), set_xy(scale)));

	thresh = find_best_string_width(font, string, ws, mode, boxdim, &nlines, &scale_ratio);
	thresh = double_add_ulp(thresh, 40);	// raises the threshold to deal with rounding errors, as much as 4 ULP have been found to be necessary, using more for safety

	box_thresh = fabs(box1.x - box0.x) / (scale*scale_ratio);				// thresh based on the calculated scale and the box
	if (box_thresh > thresh)								// if it's larger than thresh
	{
		thresh = box_thresh;								// box_thresh allows to fill the lines fully
		nlines = find_line_count_for_thresh(font, string, ws, mode, thresh, NULL);	// then the number of lines must be recounted
	}

	total_scale = scale*scale_ratio*scrscale;
	intensity *= intensity_scaling(total_scale, 1.);

	draw_string_maxwidth(font, string, ws, box0, box1, scale*scale_ratio, colour, intensity, mode, thresh, nlines);

	free_word_stats(ws);
}
