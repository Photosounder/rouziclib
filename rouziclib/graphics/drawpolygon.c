float expf_approx_for_acute_integral(float x)	// x = [-4.3 , -1.834], max error < 2.52e-6
{
	if (x < -2.93f)
		return (((0.00114811f*x + 0.02122094f)*x + 0.15357232f)*x + 0.5222234f)*x + 0.7142764f;
	else
		return (((0.00390672f*x + 0.05290861f)*x + 0.29126263f)*x + 0.79052235f)*x + 0.9120924f;
}

float erf_right_triangle_acute_integral(float x, float y)
{
	float x2 = x*x, y2 = y*y;
	float v = (((((-1.6663128e-05f*y2 + 5.104393e-06f)*x2 +
			0.0005496131f*y2 - 5.30433e-05f)*x2 +
			(0.0001584783f*y2 - 0.00741157237f)*y2 - 0.0018265954f)*x2 +
			(-0.003881739f*y2 + 0.0523013844f)*y2 + 0.04582956f)*x2 +
			((-0.00368418f*y2 + 0.03692744f)*y2 - 0.1996548f)*y2 - 0.50360028f)*x2 +
			((-0.0012717f*y2 - 0.0101518f)*y2 + 0.0109084f)*y2 - 1.836892f;
	return expf_approx_for_acute_integral(v) * x2 * y;
}

float fast_atan2pi_unitrange(float x)
{
	float x2 = x*x;
	return (((-0.00609f*x2 + 0.023075f)*x2 - 0.051014f)*x2 + 0.15901703f)*x;
}

float erf_fastf(float x)
{
	float y, xa;

	// Prepare x
	xa = fabsf(x);
	xa = MINN(xa, 4.45f);

	y = ((((-0.001133515f*xa + 0.00830125f)*xa - 0.003218f)*xa - 0.06928657f)*xa - 0.14108256f)*xa + 1.f;

	// (1 - poly(min(abs(x), 4.45))^c1) * sign(x)
	y *= y;
	y *= y;
	y *= y;
	y = 1.f - y;
	y = sign_multiplyf(y, x);

	return y;
}

float calc_right_triangle_pixel_weight(xyf_t rp)
{
	xyf_t rpa;
	int use_obtuse;
	float slope, acute, obtuse;

	rpa = abs_xyf(rp);
	use_obtuse = rpa.y > rpa.x;

	// If the edge is beyond 3 units, use the atan approximation
	if (rpa.x >= 3.f)
	{
		if (use_obtuse)
			return sign_multiplyf(0.25f - fast_atan2pi_unitrange(rpa.x/rpa.y), xor_float(rp.x, rp.y));
		else
			return fast_atan2pi_unitrange(rp.y/rp.x);
		//return fast_atan2pi_unitrange(rpa.y * sse_rcpf_newton1(rpa.x));
	}

	// Pick method
	if (use_obtuse)				// if we use the obtuse method
		swap_float(&rp.x, &rp.y);	// swap axes

	// Calculate slope
	slope = fabsf(rp.x) < 1e-5f ? 0.f : rp.y / rp.x;
	rp.x = rangelimitf(rp.x, -3.f, 3.f);

	acute = erf_right_triangle_acute_integral(rp.x, slope);

	if (use_obtuse)
	{
		obtuse = 0.25f * erf_fastf(rp.y) * erf_fastf(rp.x) - acute;
		obtuse = copysignf(obtuse, slope);
		return obtuse;
	}

	acute = copysignf(acute, slope);

	return acute;
}

xyf_t fast_normalise(xyf_t v)
{
	return mul_xyf(v, set_xyf(sse_rsqrtf_newton1(v.x*v.x+v.y*v.y)));
}

float calc_subtriangle_pixel_weight(xyf_t p0, xyf_t p1)
{
	xyf_t rot, r0, r1;
	float weight;

	// Rotate points
	rot = fast_normalise(sub_xyf(p1, p0));
	r0.x = rot.x*p0.y - rot.y*p0.x;
	r0.y = rot.x*p0.x + rot.y*p0.y;
	r1.x = r0.x;
	r1.y = rot.x*p1.x + rot.y*p1.y;

	// Calc weights
	weight = calc_right_triangle_pixel_weight(r1);
	weight -= calc_right_triangle_pixel_weight(r0);

	return weight;
}

float eval_polygon_at_pos(xy_t *p, int p_count, double radius, xy_t pos)
{
	int i;
	xyf_t p0, p1;

	// Prepare parameters
	xy_t rad = set_xy(1./radius);

	// Scale pos
	pos = mul_xy(pos, rad);

	// Calculate the weight for each subtriangle
	float weight = 0.f;
	p0 = xy_to_xyf(sub_xy(mul_xy(p[p_count-1], rad), pos));
	for (i=0; i < p_count; i++)
	{
		p1 = xy_to_xyf(sub_xy(mul_xy(p[i], rad), pos));
		weight += calc_subtriangle_pixel_weight(p0, p1);
		p0 = p1;
	}

	return weight;
}

#define POLYGON_FAR 3.25	// in blur radii, an edge further than this changes a pixel's weight by less than 0.5*erfc(3.25) = 2e-6

typedef struct
{
	xy_t p0;		// a point on the edge's line
	double nx, ny;		// unit normal pointing inside, so the signed distance is positive inside
} polygon_line_t;

static int polygon_prepare_lines(const xy_t *pl, int p_count, polygon_line_t *line)
{
	int i, pos=0, neg=0;
	double area = 0.;

	// Only convex polygons can use the shortcuts, find if all turns go the same way
	for (i=0; i < p_count; i++)
	{
		xy_t p0 = pl[(i+p_count-1) % p_count], p1 = pl[i], p2 = pl[(i+1) % p_count];
		double turn = (p1.x-p0.x)*(p2.y-p1.y) - (p1.y-p0.y)*(p2.x-p1.x);
		pos += turn > 0.;
		neg += turn < 0.;
		area += p0.x*p1.y - p1.x*p0.y;
	}
	if ((pos && neg) || area == 0.)
		return 0;

	// Each edge's line with its normal pointing inside, in the same order as the edges of the exact calculation
	for (i=0; i < p_count; i++)
	{
		xy_t p0 = pl[(i+p_count-1) % p_count], p1 = pl[i];
		double ex = p1.x - p0.x, ey = p1.y - p0.y, len = sqrt(ex*ex + ey*ey);
		if (len < 1e-9)
			return 0;
		double s = (area > 0. ? 1. : -1.) / len;
		line[i].p0 = p0;
		line[i].nx = -ey * s;
		line[i].ny = ex * s;
	}

	return 1;
}

static float polygon_pixel_weight(const xy_t *pl, int p_count, const polygon_line_t *line, float wsign, xy_t pf)
{
	int i;
	xyf_t p0, p1;

	// Shortcuts for convex polygons: outside an edge by more than POLYGON_FAR gives 0, inside every edge by more than that gives 1, and with only one edge in reach the weight is erfr of the distance to it (the blurred half-plane)
	if (line)
	{
		int n_near = 0;
		double s_near = 0.;
		for (i=0; i < p_count; i++)
		{
			double s = (pf.x - line[i].p0.x) * line[i].nx + (pf.y - line[i].p0.y) * line[i].ny;
			if (s < -POLYGON_FAR)
				return 0.f;
			if (s < POLYGON_FAR)
			{
				n_near++;
				s_near = s;
			}
		}

		if (n_near == 0)
			return wsign;
		if (n_near == 1)
			return wsign * erfr(s_near);
	}

	// Exact calculation, the weight for each subtriangle
	float weight = 0.f;
	p0 = xy_to_xyf(sub_xy(pl[p_count-1], pf));
	for (i=0; i < p_count; i++)
	{
		p1 = xy_to_xyf(sub_xy(pl[i], pf));
		weight += calc_subtriangle_pixel_weight(p0, p1);
		p0 = p1;
	}
	return weight;
}

float eval_polygon_at_pos_fast(xy_t *p, int p_count, double radius, xy_t pos)
{
	// Same as eval_polygon_at_pos() but through the shortcuts used by draw_polygon_lrgb(), for testing
	xy_t pl[16];
	polygon_line_t line[16];
	if (p_count > 16)
		return eval_polygon_at_pos(p, p_count, radius, pos);
	for (int i=0; i < p_count; i++)
		pl[i] = mul_xy(p[i], set_xy(1./radius));
	int convex = polygon_prepare_lines(pl, p_count, line);
	float wsign = polygon_signed_area(pl, p_count) > 0. ? 1.f : -1.f;
	return polygon_pixel_weight(pl, p_count, convex ? line : NULL, wsign, mul_xy(pos, set_xy(1./radius)));
}

void draw_polygon_lrgb(xy_t *p, int p_count, double radius, lrgb_t colour, const blend_func_t bf, double intensity)
{
	int i;
	size_t fbi;
	float rad, weight;
	xyi_t ip;
	xy_t pf;
	static _Thread_local xy_t *pl=NULL;
	static _Thread_local size_t pl_as=0;

	// Check polygon is on screen
	recti_t bbi;
	double grad = 3. * radius;

	if (get_dq_bounding_box_for_polygon(p, p_count, set_xy(grad), &bbi) == 0)
		return;

	// Prepare parameters
	rad = 1./radius;

	#ifdef RL_INTEL_INTR
	__m128i col_xmm = _mm_set_epi32(0, colour.b, colour.g, colour.r);		
	#endif

	// Scale polygon points
	alloc_enough(&pl, p_count, &pl_as, sizeof(xy_t), 1.0);

	for (i=0; i < p_count; i++)
		pl[i] = mul_xy(p[i], set_xy(rad));

	// Edge lines for the shortcuts, and the sign the exact calculation gives inside (polygons wound the other way come out negative and draw nothing)
	static _Thread_local polygon_line_t *line=NULL;
	static _Thread_local size_t line_as=0;
	alloc_enough(&line, p_count, &line_as, sizeof(polygon_line_t), 1.0);
	int convex = polygon_prepare_lines(pl, p_count, line);
	float wsign = polygon_signed_area(pl, p_count) > 0. ? 1.f : -1.f;
	if (wsign < 0.f && convex)
		return;

	// Go through all pixels
	for (ip.y=bbi.p0.y; ip.y < bbi.p1.y; ip.y++)
	{
		fbi = ip.y*fb->w + bbi.p0.x;
		pf.y = ip.y * rad;
		for (ip.x=bbi.p0.x; ip.x < bbi.p1.x; ip.x++, fbi++)
		{
			pf.x = ip.x * rad;

			weight = polygon_pixel_weight(pl, p_count, convex ? line : NULL, wsign, pf);
			if (weight <= 0.f)
				continue;

			weight = MAXN(0.f, weight);
			weight *= (float) intensity;
			int weight_fp = float_to_fixedpoint_15(weight);

			// Apply weight to colour
			bf(&fb->r.l[fbi], colour, weight_fp);
		}
	}
}

#define CONTOURS_FAR 3.25	// in blur radii, an edge further than this only counts through the winding number, which changes the weight by less than 0.5*erfc(3.25) = 2e-6 per edge

double subtriangle_angle(xy_t p0, xy_t p1)
{
	// The signed angle an edge subtends from the pixel in turns, which calc_subtriangle_pixel_weight() tends to away from the edge
	return -atan2(p0.x*p1.y - p0.y*p1.x, p0.x*p1.x + p0.y*p1.y) * (1./(2.*pi));
}

typedef struct
{
	xy_t a, b;		// in blur radii
	double len, inv_len;
	rect_t reach;		// bounding box expanded by CONTOURS_FAR
} contour_edge_t;

size_t contours_stat_pixels=0, contours_stat_cand=0, contours_stat_corr=0;	// TEMPORARY profiling counters

typedef struct
{
	double x;
	int dir;
} contour_crossing_t;

void draw_contours_lrgb(xy_t *p, int *cont_start, int cont_count, double radius, lrgb_t colour, const blend_func_t bf, double intensity)
{
	// Draws closed contours (any shape, holes included) with the same blurred coverage as draw_polygon_lrgb(), as the sum over every edge of calc_subtriangle_pixel_weight().
	// Far from an edge its term is just the angle it subtends, and all those angles add up to the winding number, so each pixel takes the winding number (from where the contours cross its row)
	// plus, for the edges within reach only, the difference between the exact term and the angle
	static _Thread_local contour_edge_t *edge=NULL, **cand=NULL, **active=NULL;
	static _Thread_local contour_crossing_t *cross=NULL;
	static _Thread_local size_t edge_as=0, cand_as=0, active_as=0, cross_as=0;
	int i, ie, edge_count=0, p_count = cont_start[cont_count];
	double rad = 1./radius, far2 = sq(CONTOURS_FAR), area = 0.;
	recti_t bbi;
	xyi_t ip;

	if (p_count < 3)
		return;

	if (get_dq_bounding_box_for_polygon(p, p_count, set_xy(CONTOURS_FAR * radius), &bbi) == 0)
		return;

	// Edges in blur radii, and the sign the sum gives inside, positive for outer contours wound like the polygons that draw
	alloc_enough(&edge, p_count, &edge_as, sizeof(contour_edge_t), 1.5);
	for (int ic=0; ic < cont_count; ic++)
	{
		int start = cont_start[ic], len = cont_start[ic+1] - start;
		if (len < 2)
			continue;
		area += polygon_signed_area(&p[start], len);
		for (i=0; i < len; i++)
		{
			contour_edge_t *e = &edge[edge_count];
			e->a = mul_xy(p[start + i], set_xy(rad));
			e->b = mul_xy(p[start + (i+1) % len], set_xy(rad));
			if (equal_xy(e->a, e->b))
				continue;
			e->len = hypot(e->b.x - e->a.x, e->b.y - e->a.y);
			e->inv_len = 1. / e->len;
			e->reach = rect_add_margin(sort_rect(rect(e->a, e->b)), set_xy(CONTOURS_FAR));
			edge_count++;
		}
	}
	if (area == 0. || edge_count == 0)
		return;
	float wsign = area > 0. ? 1.f : -1.f;

	alloc_enough(&cand, edge_count, &cand_as, sizeof(contour_edge_t *), 1.5);
	alloc_enough(&active, edge_count, &active_as, sizeof(contour_edge_t *), 1.5);
	alloc_enough(&cross, edge_count, &cross_as, sizeof(contour_crossing_t), 1.5);

	// Each pixel is evaluated a tiny distance away from its centre, so that no edge passes exactly through it (common with integer coordinates), where the angle would be ambiguous and could disagree with the crossing count
	const double shift_x = 1.3e-9, shift_y = 1.7e-9;

	for (ip.y=bbi.p0.y; ip.y < bbi.p1.y; ip.y++)
	{
		double py = ip.y * rad + shift_y;
		int cand_count = 0, cross_count = 0, act_count = 0, next = 0;

		// Where the contours cross this row and which way, sorted along the row, and the edges within reach of the row, sorted by where their reach starts along the row
		for (ie=0; ie < edge_count; ie++)
		{
			contour_edge_t *e = &edge[ie];
			if ((e->a.y <= py && py < e->b.y) || (e->b.y <= py && py < e->a.y))
			{
				contour_crossing_t c = { e->a.x + (py - e->a.y) * (e->b.x - e->a.x) / (e->b.y - e->a.y), e->b.y > e->a.y ? 1 : -1 };
				for (i = cross_count++; i > 0 && cross[i-1].x > c.x; i--)
					cross[i] = cross[i-1];
				cross[i] = c;
			}
			if (py >= e->reach.p0.y && py <= e->reach.p1.y)
			{
				for (i = cand_count++; i > 0 && cand[i-1]->reach.p0.x > e->reach.p0.x; i--)
					cand[i] = cand[i-1];
				cand[i] = e;
			}
		}

		size_t fbi = ip.y*fb->w + bbi.p0.x;
		int ic = 0, winding = 0;
		for (ip.x=bbi.p0.x; ip.x < bbi.p1.x; ip.x++, fbi++)
		{
			double px = ip.x * rad + shift_x;

			// Winding number from the crossings to the left of the pixel (the angles of all the edges add up to this)
			while (ic < cross_count && cross[ic].x <= px)
				winding += cross[ic++].dir;
			float weight = winding;
			contours_stat_pixels++;

			// The edges whose reach covers this pixel along the row
			while (next < cand_count && cand[next]->reach.p0.x <= px)
				active[act_count++] = cand[next++];

			// Corrections for the edges within reach
			for (i=0; i < act_count; )
			{
				contour_edge_t *e = active[i];
				if (px > e->reach.p1.x)
				{
					active[i] = active[--act_count];
					continue;
				}
				i++;
				contours_stat_cand++;

				// Position along the edge and signed distance from its line (positive on the side the sum fills)
				double ex = e->b.x - e->a.x, ey = e->b.y - e->a.y, apx = px - e->a.x, apy = py - e->a.y;
				double along = (apx*ex + apy*ey) * e->inv_len, s = (apx*ey - apy*ex) * e->inv_len;

				// Near only the middle of the edge, with both ends out of reach: the correction is the blurred step minus the sharp step
				if (along >= CONTOURS_FAR && along <= e->len - CONTOURS_FAR)
				{
					if (fabs(s) < CONTOURS_FAR)
					{
						contours_stat_corr++;
						weight += erfr(s) - (s > 0.);
					}
					continue;
				}

				// Otherwise the exact term minus the angle, when the segment is within reach
				double t = rangelimit(along * e->inv_len, 0., 1.);
				if (sq(apx - t*ex) + sq(apy - t*ey) >= far2)
					continue;

				contours_stat_corr++;
				xy_t p0 = xy(-apx, -apy), p1 = xy(e->b.x - px, e->b.y - py);
				weight += calc_subtriangle_pixel_weight(xy_to_xyf(p0), xy_to_xyf(p1)) - subtriangle_angle(p0, p1);
			}

			weight *= wsign;
			if (weight <= 0.f)
				continue;
			weight = MINN(weight, 1.f) * (float) intensity;
			bf(&fb->r.l[fbi], colour, float_to_fixedpoint_15(weight));
		}
	}
}

void draw_contours(xy_t *p, int *cont_start, int cont_count, double radius, col_t colour, const blend_func_t bf, double intensity)
{
	if (fb->discard)
		return;

	radius = drawing_focus_adjust(focus_rlg, radius, NULL, 0);	// adjusts the focus

	if (fb->use_drawq == 0 && fb->r.use_frgb == 0)
		draw_contours_lrgb(p, cont_start, cont_count, radius, col_to_lrgb(colour), bf, intensity);
}

void draw_polygon_dq(xy_t *p, int p_count, double radius, frgb_t colour, const int bf, double intensity)
{
#ifndef RL_FREESTANDING
	int i;
	float *df;
	double grad;
	xyi_t ip;
	recti_t bbi;

	if (p_count < 3)
		return;

	// Handle pentagons and higher by subdivision
	if (p_count > 4)
	{
		xy_t psub[4];
		psub[0] = p[0];
		for (i=1; i < p_count-1; i += 2)
		{
			int psub_count = p_count-i < 3 ? 3 : 4;
			memcpy(&psub[1], &p[i], (psub_count-1) * sizeof(xy_t));
			draw_polygon_dq(psub, psub_count, radius, colour, bf, intensity);
		}
		return;
	}

	grad = 3. * radius;

	if (drawq_get_bounding_box_for_polygon(p, p_count, set_xy(grad), &bbi) == 0)
		return;

	// Store the drawing parameters in the main drawing queue
	df = drawq_add_to_main_queue(p_count==3 ? DQT_TRIANGLE : DQT_TETRAGON);
	df[0] = 1./radius;
	df[1] = colour.r * intensity;
	df[2] = colour.g * intensity;
	df[3] = colour.b * intensity;
	for (i=0; i < p_count; i++)
	{
		df[4+i*2] = p[i].x;
		df[5+i*2] = p[i].y;
	}

	// Find the affected sectors
	for (ip.y=bbi.p0.y; ip.y<=bbi.p1.y; ip.y++)
		for (ip.x=bbi.p0.x; ip.x<=bbi.p1.x; ip.x++)
			drawq_add_sector_id(ip.y*fb->sector_w + ip.x);	// add sector reference
#endif	// RL_FREESTANDING
}

rect_t get_bounding_box_for_polygon(xy_t *p, int p_count)
{
	int i;
	rect_t bb;

	// Calculate the bounding box
	bb.p0 = p[0];
	bb.p1 = p[0];
	for (i=1; i < p_count; i++)
	{
		bb.p0 = min_xy(bb.p0, p[i]);
		bb.p1 = max_xy(bb.p1, p[i]);
	}

	return bb;
}

int get_dq_bounding_box_for_polygon(xy_t *p, int p_count, xy_t rad, recti_t *bbi)
{
	int i;
	rect_t bb, fb_box = rect(XY0, xy(fb->w-1, fb->h-1));

	// Calculate the bounding box
	bb = get_bounding_box_for_polygon(p, p_count);
	bb = rect_add_margin(bb, rad);

	// Check against framebuffer boundaries
	if (check_box_box_intersection(bb, fb_box)==0)
		return 0;

	// Convert to sector coordinates
	bbi->p0 = xy_to_xyi(max_xy(bb.p0, fb_box.p0));
	bbi->p1 = xy_to_xyi(min_xy(bb.p1, fb_box.p1));

	return 1;
}

void draw_polygon(xy_t *p, int p_count, double radius, col_t colour, const blend_func_t bf, double intensity)
{
	if (fb->discard)
		return;

	radius = drawing_focus_adjust(focus_rlg, radius, NULL, 0);	// adjusts the focus

	if (fb->use_drawq)
		draw_polygon_dq(p, p_count, radius, col_to_frgb(colour), 0, intensity);
	else if (fb->r.use_frgb == 0)
		draw_polygon_lrgb(p, p_count, radius, col_to_lrgb(colour), bf, intensity);
}

void draw_polygon_wc(xy_t *p, int p_count, double radius, col_t colour, const blend_func_t bf, double intensity)
{
	int i;
	xy_t p_sc[16];

	// Convert to screen coordinates in reverse order
	for (i=0; i < p_count; i++)
		p_sc[p_count-1-i] = sc_xy(p[i]);

	draw_polygon(p_sc, p_count, radius, colour, bf, intensity);
}

void draw_polygon_bothsides_wc(xy_t *p, int p_count, double radius, col_t colour, const blend_func_t bf, double intensity)
{
	int i, rev = 1;
	xy_t p_sc[16];

	// Check if the order must be reversed
	if (polygon_signed_area(p, p_count) < 0.)
		rev = 0;

	// Convert to screen coordinates in reverse order
	for (i=0; i < p_count; i++)
		p_sc[rev ? p_count-1-i : i] = sc_xy(p[i]);

	draw_polygon(p_sc, p_count, radius, colour, bf, intensity);
}

void draw_triangle(triangle_t tri, double radius, col_t colour, const blend_func_t bf, double intensity)
{
	draw_polygon((xy_t *) &tri, 3, radius, colour, bf, intensity);
}

void draw_triangle_wc(triangle_t tri, double radius, col_t colour, const blend_func_t bf, double intensity)
{
	draw_polygon_wc((xy_t *) &tri, 3, radius, colour, bf, intensity);
}
