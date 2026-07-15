void draw_rect_full_dq(rect_t box, double radius, frgb_t colour, double intensity)
{
#ifndef RL_FREESTANDING
	float *df;
	double grad;
	xyi_t ip;
	rect_t fr, screen_box = rect(XY0, xy(fb->w-1, fb->h-1));
	recti_t bbi, fri;

	if (intensity==0.)
		return ;

	grad = GAUSSRAD_HQ * radius;		// erfr and gaussian can go up to x = ±4

	if (drawq_get_bounding_box(box, set_xy(grad), &bbi)==0)
			return ;

	box = sort_rect(box);

	// calculate the fill rectangle, which is the inner rectangle where sectors are just filled plainly
	fr.p0 = ceil_xy(add_xy(box.p0, set_xy(grad)));
	fr.p1 = floor_xy(sub_xy(box.p1, set_xy(grad)));

	fri.p0 = xy_to_xyi(max_xy(fr.p0, screen_box.p0));
	fri.p1 = xy_to_xyi(min_xy(fr.p1, screen_box.p1));
	fri = rshift_recti(fri, fb->sector_size);

	if (fri.p1.x <= fri.p0.x || fri.p1.y <= fri.p0.y)	// if there is no plain fill area
		fri = recti(xyi(-1,-1), xyi(-1,-1));

	// calculate the drawing parameters
	colour.r *= intensity;
	colour.g *= intensity;
	colour.b *= intensity;

	// store the drawing parameters in the main drawing queue
	df = drawq_add_to_main_queue(DQT_RECT_FULL);
	df[0] = box.p0.x;
	df[1] = box.p0.y;
	df[2] = box.p1.x;
	df[3] = box.p1.y;
	df[4] = 1./radius;
	df[5] = colour.r;
	df[6] = colour.g;
	df[7] = colour.b;

	// find the affected sectors
	for (ip.y=bbi.p0.y; ip.y<=bbi.p1.y; ip.y++)
		for (ip.x=bbi.p0.x; ip.x<=bbi.p1.x; ip.x++)
			if (check_point_within_box_int(ip, fri)!=1)			// if we're not inside the plain fill area
				drawq_add_sector_id(ip.y*fb->sector_w + ip.x);		// add sector reference

	if (fri.p0.x == -1)
		return ;

	//************PLAIN FILL AREA************

	df = drawq_add_to_main_queue(DQT_PLAIN_FILL);
	df[0] = colour.r;
	df[1] = colour.g;
	df[2] = colour.b;

	// find the affected sectors
	for (ip.y=fri.p0.y; ip.y<=fri.p1.y; ip.y++)
		for (ip.x=fri.p0.x; ip.x<=fri.p1.x; ip.x++)
			if (check_point_within_box_int(xyi(ip.x, ip.y), fri)==1)	// if we're inside the plain fill area
				drawq_add_sector_id(ip.y*fb->sector_w + ip.x);		// add sector reference
#endif	// RL_FREESTANDING
}

void draw_rect_full_lrgb(rect_t box, double radius, lrgb_t colour, const blend_func_t bf, double intensity)
{
	double grad;
	xyi_t ip, pf, d0, d1, gv;
	rect_t gr, fr;
	recti_t gri, fri, rfp, screen_box = recti(XYI0, xyi(fb->w-1, fb->h-1));
	int32_t fbi, p, iradf, ratio;
	const int32_t fp=16;
	const double fpratio = (double) (1<<fp);

	ratio = 32768. * intensity + 0.5;
	if (ratio == 0)
		return;

	// Shrink box to fit in a maximum size and check if it's entirely off-screen
	grad = GAUSSRAD_HQ * radius;		// erfr and gaussian can go up to x = ±3
	box = rect_intersection(box, rect_add_margin(recti_to_rect(screen_box), set_xy(grad+1.)));
	if (isnan(box.p0.x))
		return;

	rfp = rect_to_recti_fixedpoint(box, fpratio);
	iradf = roundaway(1./radius * fpratio);

	// calculate the gauss rectangle, which is the outer rectangle that contains all the pixels
	gr.p0 = ceil_xy(sub_xy(box.p0, set_xy(grad)));
	gr.p1 = floor_xy(add_xy(box.p1, set_xy(grad)));

	gri.p0 = max_xyi(xy_to_xyi(gr.p0), screen_box.p0);
	gri.p1 = min_xyi(xy_to_xyi(gr.p1), screen_box.p1);

	// calculate the fill rectangle, which is the inner rectangle where pixels are just filled plainly
	fr.p0 = ceil_xy(add_xy(box.p0, set_xy(grad)));
	fr.p1 = floor_xy(sub_xy(box.p1, set_xy(grad)));

	fri.p0 = max_xyi(xy_to_xyi(fr.p0), screen_box.p0);
	fri.p1 = min_xyi(xy_to_xyi(fr.p1), screen_box.p1);

	if (fri.p1.x <= fri.p0.x || fri.p1.y <= fri.p0.y)	// if there is no plain fill area
	{
		for (ip.y=gri.p0.y; ip.y<=gri.p1.y; ip.y++)
		{
			pf.y = ip.y << fp;
			d0.y = (int64_t) (pf.y - rfp.p0.y) * iradf >> fp;
			d1.y = (int64_t) (pf.y - rfp.p1.y) * iradf >> fp;
			gv.y = (fperfr_d0(d0.y) - fperfr_d0(d1.y)) >> 15;

			for (ip.x=gri.p0.x; ip.x<=gri.p1.x; ip.x++)
			{
				fbi = ip.y*fb->w + ip.x;

				pf.x = ip.x << fp;
				d0.x = (int64_t) (pf.x - rfp.p0.x) * iradf >> fp;
				d1.x = (int64_t) (pf.x - rfp.p1.x) * iradf >> fp;
				gv.x = (fperfr_d0(d0.x) - fperfr_d0(d1.x)) >> 15;
				p = gv.x * gv.y >> 15;
				p = p * ratio >> 15;

				bf(&fb->r.l[fbi], colour, p);
			}
		}
	}
	else
	{
		// first Y section
		for (ip.y=gri.p0.y; ip.y < fri.p0.y; ip.y++)
		{
			pf.y = ip.y << fp;
			d0.y = (int64_t) (pf.y - rfp.p0.y) * iradf >> fp;
			gv.y = fperfr_d0(d0.y) >> 15;

			for (ip.x=gri.p0.x; ip.x < fri.p0.x; ip.x++)	// first X section
			{
				pf.x = ip.x << fp;
				d0.x = (int64_t) (pf.x - rfp.p0.x) * iradf >> fp;
				gv.x = fperfr_d0(d0.x) >> 15;
				p = gv.x * gv.y >> 15;
				p = p * ratio >> 15;

				bf(&fb->r.l[ip.y*fb->w + ip.x], colour, p);
			}

			p = gv.y * ratio >> 15;
			for (; ip.x <= fri.p1.x; ip.x++)		// mid X section
				bf(&fb->r.l[ip.y*fb->w + ip.x], colour, p);

			for (; ip.x <= gri.p1.x; ip.x++)		// last X section
			{
				pf.x = ip.x << fp;
				d1.x = (int64_t) (pf.x - rfp.p1.x) * iradf >> fp;
				gv.x = fperfr_d0(-d1.x) >> 15;
				p = gv.x * gv.y >> 15;
				p = p * ratio >> 15;

				bf(&fb->r.l[ip.y*fb->w + ip.x], colour, p);
			}
		}

		// mid Y section
		for (; ip.y <= fri.p1.y; ip.y++)
		{
			for (ip.x=gri.p0.x; ip.x < fri.p0.x; ip.x++)	// first X section
			{
				pf.x = ip.x << fp;
				d0.x = (int64_t) (pf.x - rfp.p0.x) * iradf >> fp;
				gv.x = fperfr_d0(d0.x) >> 15;
				p = gv.x * ratio >> 15;

				bf(&fb->r.l[ip.y*fb->w + ip.x], colour, p);
			}

			for (; ip.x <= fri.p1.x; ip.x++)		// mid X section
				bf(&fb->r.l[ip.y*fb->w + ip.x], colour, ratio);

			for (; ip.x <= gri.p1.x; ip.x++)		// last X section
			{
				pf.x = ip.x << fp;
				d1.x = (int64_t) (pf.x - rfp.p1.x) * iradf >> fp;
				gv.x = fperfr_d0(-d1.x) >> 15;
				p = gv.x * ratio >> 15;

				bf(&fb->r.l[ip.y*fb->w + ip.x], colour, p);
			}
		}

		// last Y section
		for (; ip.y <= gri.p1.y; ip.y++)
		{
			pf.y = ip.y << fp;
			d1.y = (int64_t) (pf.y - rfp.p1.y) * iradf >> fp;
			gv.y = fperfr_d0(-d1.y) >> 15;

			// this block of code is the same as in the first Y section
			for (ip.x=gri.p0.x; ip.x < fri.p0.x; ip.x++)	// first X section
			{
				pf.x = ip.x << fp;
				d0.x = (int64_t) (pf.x - rfp.p0.x) * iradf >> fp;
				gv.x = fperfr_d0(d0.x) >> 15;
				p = gv.x * gv.y >> 15;
				p = p * ratio >> 15;

				bf(&fb->r.l[ip.y*fb->w + ip.x], colour, p);
			}

			p = gv.y * ratio >> 15;
			for (; ip.x <= fri.p1.x; ip.x++)		// mid X section
				bf(&fb->r.l[ip.y*fb->w + ip.x], colour, p);

			for (; ip.x <= gri.p1.x; ip.x++)		// last X section
			{
				pf.x = ip.x << fp;
				d1.x = (int64_t) (pf.x - rfp.p1.x) * iradf >> fp;
				gv.x = fperfr_d0(-d1.x) >> 15;
				p = gv.x * gv.y >> 15;
				p = p * ratio >> 15;

				bf(&fb->r.l[ip.y*fb->w + ip.x], colour, p);
			}
		}
	}
}

void draw_rect_full_dqnq(rect_t box, double radius, frgb_t colour, double intensity)
{
#ifndef RL_FREESTANDING
	const enum dqnq_type type = DQNQT_RECT_FULL;

	// Get pointer to data buffer
	uint8_t *p = (uint8_t *) dqnq_new_entry(type);

	// Write arguments to buffer
	write_LE32(&p, float_as_u32(box.p0.x));
	write_LE32(&p, float_as_u32(box.p0.y));
	write_LE32(&p, float_as_u32(box.p1.x));
	write_LE32(&p, float_as_u32(box.p1.y));
	write_LE32(&p, float_as_u32(radius));
	write_LE32(&p, float_as_u32(colour.r * intensity));
	write_LE32(&p, float_as_u32(colour.g * intensity));
	write_LE32(&p, float_as_u32(colour.b * intensity));

	dqnq_finish_entry(type);
#endif	// RL_FREESTANDING
}

void draw_rect_full(rect_t box, double radius, col_t colour, const blend_func_t bf, double intensity)
{
	if (fb->discard)
		return;

	radius = drawing_focus_adjust(focus_rlg, radius, NULL, 0);	// adjusts the focus

	if (fb->use_drawq)
		if (fb->use_dqnq)
			draw_rect_full_dqnq(box, radius, col_to_frgb(colour), intensity);
		else
			draw_rect_full_dq(box, radius, col_to_frgb(colour), intensity);
	else
		draw_rect_full_lrgb(box, radius, col_to_lrgb(colour), bf, intensity);
}

void draw_black_rect_dq(rect_t box, double radius, double intensity)
{
#ifndef RL_FREESTANDING
	float *df;
	double grad;
	xyi_t ip;
	recti_t bbi;

	grad = GAUSSRAD_HQ * radius;		// erfr and gaussian can go up to x = ±4

	if (drawq_get_bounding_box(box, set_xy(grad), &bbi)==0)
			return ;

	box = sort_rect(box);

	// Store the drawing parameters in the main drawing queue
	df = drawq_add_to_main_queue(DQT_RECT_BLACK);
	df[0] = box.p0.x;
	df[1] = box.p0.y;
	df[2] = box.p1.x;
	df[3] = box.p1.y;
	df[4] = 1./radius;
	df[5] = intensity;

	// Find the affected sectors
	for (ip.y=bbi.p0.y; ip.y<=bbi.p1.y; ip.y++)
		for (ip.x=bbi.p0.x; ip.x<=bbi.p1.x; ip.x++)
		{
			int32_t sector_id = ip.y*fb->sector_w + ip.x;
			if (fb->sector_count[sector_id] > 0 && fb->pending_bracket[sector_id] == 0)	// if the sector contains something at the current bracket level
				drawq_add_sector_id(sector_id);	// add sector reference
		}

	// TODO clear lists in obscured sectors, don't add to empty sectors
#endif	// RL_FREESTANDING
}

void draw_black_rect_dqnq(rect_t box, double radius, double intensity)
{
#ifndef RL_FREESTANDING
	const enum dqnq_type type = DQNQT_RECT_BLACK;

	// Get pointer to data buffer
	uint8_t *p = (uint8_t *) dqnq_new_entry(type);

	// Write arguments to buffer
	write_LE32(&p, float_as_u32(box.p0.x));
	write_LE32(&p, float_as_u32(box.p0.y));
	write_LE32(&p, float_as_u32(box.p1.x));
	write_LE32(&p, float_as_u32(box.p1.y));
	write_LE32(&p, float_as_u32(radius));
	write_LE32(&p, float_as_u32(intensity));

	dqnq_finish_entry(type);
#endif	// RL_FREESTANDING
}

void draw_black_rect(rect_t box, double radius, double intensity)
{
	if (fb->discard)
		return;

	radius = drawing_focus_adjust(focus_rlg, radius, NULL, 0);	// adjusts the focus

	if (fb->use_drawq)
		if (fb->use_dqnq)
			draw_black_rect_dqnq(box, radius, intensity);
		else
			draw_black_rect_dq(box, radius, intensity);
	else
		draw_rect_full_lrgb(box, radius, col_to_lrgb(make_grey(0.)), blend_alphablendfg, intensity);
}

void draw_black_rect_inverted_dq(rect_t box, double radius, double intensity)
{
#ifndef RL_FREESTANDING
	float *df;
	double grad;
	xyi_t ip;
	recti_t bbi;

	grad = GAUSSRAD_HQ * radius;		// erfr and gaussian can go up to x = ±4

	drawq_get_inner_box(box, set_xy(grad), &bbi);

	box = sort_rect(box);

	// Store the drawing parameters in the main drawing queue
	df = drawq_add_to_main_queue(DQT_RECT_BLACK_INV);
	df[0] = box.p0.x;
	df[1] = box.p0.y;
	df[2] = box.p1.x;
	df[3] = box.p1.y;
	df[4] = 1./radius;
	df[5] = intensity;

	// Find the affected sectors (all sectors outside the inner box)
	rect_t screen_box = rect(XY0, xy(fb->w-1, fb->h-1));
	for (ip.y=0; ip.y <= fb->h-1 >> fb->sector_size; ip.y++)
		for (ip.x=0; ip.x <= fb->w-1 >> fb->sector_size; ip.x++)
			if (ip.x <= bbi.p0.x || ip.y <= bbi.p0.y || ip.x >= bbi.p1.x || ip.y >= bbi.p1.y)
			{
				int32_t sector_id = ip.y*fb->sector_w + ip.x;
				if (fb->sector_count[sector_id] > 0 && fb->pending_bracket[sector_id] == 0)	// if the sector contains something at the current bracket level
					drawq_add_sector_id(sector_id);	// add sector reference
			}
#endif	// RL_FREESTANDING
}

void draw_black_rect_inverted_dqnq(rect_t box, double radius, double intensity)
{
#ifndef RL_FREESTANDING
	const enum dqnq_type type = DQNQT_RECT_BLACK_INV;

	// Get pointer to data buffer
	uint8_t *p = (uint8_t *) dqnq_new_entry(type);

	// Write arguments to buffer
	write_LE32(&p, float_as_u32(box.p0.x));
	write_LE32(&p, float_as_u32(box.p0.y));
	write_LE32(&p, float_as_u32(box.p1.x));
	write_LE32(&p, float_as_u32(box.p1.y));
	write_LE32(&p, float_as_u32(radius));
	write_LE32(&p, float_as_u32(intensity));

	dqnq_finish_entry(type);
#endif	// RL_FREESTANDING
}

static int32_t draw_black_rect_inverted_axis_weight_direct(int32_t p, double p0, double p1, int32_t p0f, int32_t p1f, int32_t iradf, int hard_edge)
{
	int32_t pf, d0, d1, weight;
	const int32_t fp = 16;

	// Use an exact binary mask when no soft radius was requested
	if (hard_edge)
		return p >= p0 && p <= p1 ? 32768 : 0;

	// Evaluate the same fixed-point error-function rectangle profile as direct filled rectangles
	pf = p << fp;
	d0 = (int64_t) (pf - p0f) * iradf >> fp;
	d1 = (int64_t) (pf - p1f) * iradf >> fp;
	weight = (fperfr_d0(d0) - fperfr_d0(d1)) >> 15;
	return rangelimit(weight, 0, 32768);
}

static int draw_black_rect_inverted_weights_direct(rect_t box, double radius, int32_t **x_weightp, recti_t *rfpp, int32_t *iradfp, int *hard_edgep, int *x0p, int *x1p)
{
	int32_t *x_weight;
	int32_t iradf=0;
	int x, x0=fb->w, x1=-1, hard_edge;
	const int32_t fp = 16;
	const double fpratio = (double) (1 << fp);

	// Reject invalid rectangles before allocating temporary weights
	box = sort_rect(box);
	if (isnan(box.p0.x) || isnan(box.p0.y) || isnan(box.p1.x) || isnan(box.p1.y))
		return 0;

	// Allocate one row of horizontal mask weights
	x_weight = malloc((size_t) fb->w * sizeof(*x_weight));
	if (x_weight==NULL)
	{
		fprintf_rl(stderr, "Direct inverted rectangle could not allocate %d mask weights\n", fb->w);
		return 0;
	}

	// Prepare fixed-point bounds and the optional soft-edge reciprocal radius
	hard_edge = radius <= 0.;
	*rfpp = rect_to_recti_fixedpoint(box, fpratio);
	if (!hard_edge)
	{
		if (!isfinite(radius))
		{
			free(x_weight);
			return 0;
		}
		iradf = roundaway((1. / radius) * fpratio);
	}

	// Cache horizontal weights and their nonzero span for the common full mask
	for (x=0; x < fb->w; x++)
	{
		x_weight[x] = draw_black_rect_inverted_axis_weight_direct(x, box.p0.x, box.p1.x, rfpp->p0.x, rfpp->p1.x, iradf, hard_edge);
		if (x_weight[x])
		{
			x0 = MINN(x0, x);
			x1 = MAXN(x1, x);
		}
	}

	// Return the prepared mask state to the pixel-format-specific multiplier
	*x_weightp = x_weight;
	*iradfp = iradf;
	*hard_edgep = hard_edge;
	*x0p = x0;
	*x1p = x1;
	return 1;
}

static void draw_black_rect_inverted_lrgb(rect_t box, double radius, double intensity)
{
	int32_t *x_weight, y_weight, coverage, mask, iradf;
	int x, y, ic, x0, x1, hard_edge, ratio;
	recti_t rfp;
	lrgb_t *row;
	uint16_t *channel;

	// Prepare the separable rectangle mask shared by all rows
	box = sort_rect(box);
	if (!draw_black_rect_inverted_weights_direct(box, radius, &x_weight, &rfp, &iradf, &hard_edge, &x0, &x1))
		return;
	ratio = rangelimit(intensity, 0., 1.) * 32768. + 0.5;

	// Multiply every LRGB and alpha channel by the inverted-black rectangle mask
	for (y=0; y < fb->h; y++)
	{
		row = &fb->r.l[(size_t) y * fb->w];
		y_weight = draw_black_rect_inverted_axis_weight_direct(y, box.p0.y, box.p1.y, rfp.p0.y, rfp.p1.y, iradf, hard_edge);

		if (ratio==32768 && (y_weight==0 || x1 < x0))
		{
			memset(row, 0, (size_t) fb->w * sizeof(*row));
			continue;
		}
		if (ratio==32768)
		{
			memset(row, 0, (size_t) x0 * sizeof(*row));
			memset(&row[x1+1], 0, (size_t) (fb->w-x1-1) * sizeof(*row));
		}

		for (x = ratio==32768 ? x0 : 0; x < (ratio==32768 ? x1+1 : fb->w); x++)
		{
			coverage = (int64_t) x_weight[x] * y_weight >> 15;
			mask = ((int64_t) coverage * ratio >> 15) + (32768 - ratio);
			if (mask==32768)
				continue;

			channel = (uint16_t *) &row[x];
			for (ic=0; ic < 4; ic++)
				channel[ic] = ((uint32_t) channel[ic] * mask + 16384) >> 15;
		}
	}

	free(x_weight);
}

static void draw_black_rect_inverted_frgb(rect_t box, double radius, double intensity)
{
	int32_t *x_weight, y_weight, iradf;
	int x, y, ic, x0, x1, hard_edge, ratio;
	float coverage, mask, mask_intensity, *channel;
	recti_t rfp;
	frgb_t *row;

	// Prepare the separable rectangle mask shared by all rows
	box = sort_rect(box);
	if (!draw_black_rect_inverted_weights_direct(box, radius, &x_weight, &rfp, &iradf, &hard_edge, &x0, &x1))
		return;
	ratio = rangelimit(intensity, 0., 1.) * 32768. + 0.5;
	mask_intensity = (float) ratio * (1.f / 32768.f);

	// Multiply every floating-point colour and alpha channel by the rectangle mask
	for (y=0; y < fb->h; y++)
	{
		row = &fb->r.f[(size_t) y * fb->w];
		y_weight = draw_black_rect_inverted_axis_weight_direct(y, box.p0.y, box.p1.y, rfp.p0.y, rfp.p1.y, iradf, hard_edge);

		if (ratio==32768 && (y_weight==0 || x1 < x0))
		{
			memset(row, 0, (size_t) fb->w * sizeof(*row));
			continue;
		}
		if (ratio==32768)
		{
			memset(row, 0, (size_t) x0 * sizeof(*row));
			memset(&row[x1+1], 0, (size_t) (fb->w-x1-1) * sizeof(*row));
		}

		for (x = ratio==32768 ? x0 : 0; x < (ratio==32768 ? x1+1 : fb->w); x++)
		{
			coverage = (float) ((int64_t) x_weight[x] * y_weight >> 15) * (1.f / 32768.f);
			mask = coverage * mask_intensity + (1.f - mask_intensity);
			if (mask==1.f)
				continue;

			channel = (float *) &row[x];
			for (ic=0; ic < 4; ic++)
				channel[ic] *= mask;
		}
	}

	free(x_weight);
}

void draw_black_rect_inverted(rect_t box, double radius, double intensity)
{
	if (fb->discard)
		return;

	radius = drawing_focus_adjust(focus_rlg, radius, NULL, 0);	// adjusts the focus

	if (fb->use_drawq)
		if (fb->use_dqnq)
			draw_black_rect_inverted_dqnq(box, radius, intensity);
		else
			draw_black_rect_inverted_dq(box, radius, intensity);
	else if (fb->r.use_frgb)
		draw_black_rect_inverted_frgb(box, radius, intensity);
	else
		draw_black_rect_inverted_lrgb(box, radius, intensity);
}
