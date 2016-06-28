void draw_line_thin_lrgb(raster_t fb, double x1, double y1, double x2, double y2, double radius, lrgb_t colour, const blend_func_t bf, double intensity)
{
	int32_t i, iy, ix, fbi;
	double x3, y3, x4, y4;
	double d12, d12s, d12si, dx12, dy12, vx12, vy12, grad;
	int32_t p, ratio;		// 0.15
	double iradius, bradius, bvx, bvy, x1l, y1l, x2l, y2l, x3l, y3l, x4l, y4l;
	int32_t bstartx, bstarty, bendx, bendy, incx, incy, incc;
	int bx0, by0, bx1, by1, dx, dy, sx, sy, err, e2;	// Bresenham routine variables

	int32_t x1f, y1f, x2f, y2f;
	int32_t xr1, yr1, xr2, xrp, yrp;
	int32_t th, costh, sinth;
	const int32_t fp=16, fpi=30-fp;
	const double fpratio = (double) (1<<fp);

	grad = GAUSSRAD(intensity, radius);	// solves e^-x² = GAUSSLIMIT for x, giving 2.92 (the necessary Gaussian radius) for GAUSSLIMIT of 0.0002
	bradius = grad * sqrt(2.);		// bounding radius, the maximum radius necessary at each end of the line

	border_clip(fb.w-1, fb.h-1, &x1, &y1, &x2, &y2, grad);	// cut the part of the segment outside the screen

	ratio = 32768. * intensity + 0.5;

	dx12 = x2-x1;
	dy12 = y2-y1;
	d12 = hypot(dx12, dy12); if (d12==0.) return;
	d12s = d12 * d12;
	d12si = 1./d12s;
	vx12 = dx12/d12;
	vy12 = dy12/d12;

	x3 = x1 - bradius * vx12;
	y3 = y1 - bradius * vy12;
	x4 = x2 + bradius * vx12;
	y4 = y2 + bradius * vy12;
	bvx = -vy12;
	bvy = vx12;
	x1l = x1 - bvx * grad;
	y1l = y1 - bvy * grad;
	x2l = x2 - bvx * grad;
	y2l = y2 - bvy * grad;

	iradius = 1./radius;

	x1f = roundaway(x1 * fpratio);
	y1f = roundaway(y1 * fpratio);
	x2f = roundaway(x2 * fpratio);
	y2f = roundaway(y2 * fpratio);

	th = fpatan2_d2(y2f-y1f, x2f-x1f);
	costh = iradius * fpcos_d2(-th);			// iradius is double, can be above 1.0, not sure how that doesn't overflow
	sinth = iradius * fpsin_d2(-th);
	xr1 = ((int64_t) x1f * costh >> 30) - ((int64_t) y1f * sinth >> 30);
	yr1 = ((int64_t) x1f * sinth >> 30) + ((int64_t) y1f * costh >> 30);
	xr2 = ((int64_t) x2f * costh >> 30) - ((int64_t) y2f * sinth >> 30);

	incx = 949;
	if (dx12 >= fabs(dy12))	// start line left side of x1
	{
		x3l = x3;
		y3l = y3 - 1.;
		incx = 0;
		incy = 1;
	}
	if (dy12 >= fabs(dx12))	// start line up side of x1
	{
		x3l = x3 + 1.;
		y3l = y3;
		incx = -1;
		incy = 0;
	}
	if (-dx12 >= fabs(dy12))	// start line right side of x1
	{
		x3l = x3;
		y3l = y3 + 1.;
		incx = 0;
		incy = -1;
	}
	if (-dy12 >= fabs(dx12))	// start line down side of x1
	{
		x3l = x3 - 1.;
		y3l = y3;
		incx = 1;
		incy = 0;
	}
	if (incx==949)		// if the coordinates are invalid (like a NaN)
		return ;	// return

	line_line_intersection(x1l, y1l, x2l, y2l, x3, y3, x3l, y3l, &x3l, &y3l);

	x4l = x4 - x3 + x3l;
	y4l = y4 - y3 + y3l;

	if (incx+incy == 1)
	{
		bstartx = (int32_t) floor (x3l);
		bstarty = (int32_t) floor (y3l);
		bendx = (int32_t) floor (x4l);
		bendy = (int32_t) floor (y4l);

		if (incx==1)
			incc = (int32_t) ceil (x3*2-x3l) - bstartx;
		else
			incc = (int32_t) ceil (y3*2-y3l) - bstarty;
	}
	else
	{
		bstartx = (int32_t) ceil (x3l);
		bstarty = (int32_t) ceil (y3l);
		bendx = (int32_t) ceil (x4l);
		bendy = (int32_t) ceil (y4l);

		if (incx==-1)
			incc = bstartx - (int32_t) floor (x3*2-x3l);
		else
			incc = bstarty - (int32_t) floor (y3*2-y3l);
	}

	incc = fastabs(incc) + 1;

	// check for >45 degree angles
	if (dx12 >= fabs(dy12))	// start line left side of x1
		if (bendx-bstartx < fastabs(bendy-bstarty))
			bendy = fastabs(bendx-bstartx) * sign(bendy-bstarty) + bstarty;
	if (dy12 >= fabs(dx12))	// start line up side of x1
		if (bendy-bstarty < fastabs(bendx-bstartx))
			bendx = fastabs(bendy-bstarty) * sign(bendx-bstartx) + bstartx;
	if (-dx12 >= fabs(dy12))	// start line right side of x1
		if (-bendx+bstartx < fastabs(bendy-bstarty))
			bendy = fastabs(bendx-bstartx) * sign(bendy-bstarty) + bstarty;
	if (-dy12 >= fabs(dx12))	// start line down side of x1
		if (-bendy+bstarty < fastabs(bendx-bstartx))
			bendx = fastabs(bendy-bstarty) * sign(bendx-bstartx) + bstartx;

	for (i=0; i<incc; i++)
	{
		//********Bresenham routine********
		bx0 = bstartx;
		by0 = bstarty;
		bx1 = bendx;
		by1 = bendy;
	
		dx = fastabs(bx1-bx0);
		dy = fastabs(by1-by0);
		sx = -1; if (bx0 < bx1) sx = 1;
		sy = -1; if (by0 < by1) sy = 1;
		err = dx-dy; ix = bx0; iy = by0;
		
		while (1)
		{
			if (ix>=0 && ix<fb.w && iy>=0 && iy<fb.h)
			{
				fbi = iy*fb.w+ix;
				xrp = ((int64_t) ix * costh >> fpi) - ((int64_t) iy * sinth >> fpi);
				yrp = ((int64_t) ix * sinth >> fpi) + ((int64_t) iy * costh >> fpi);

				p = fperfr_d0(xrp-xr1) >> 15;
				p -= fperfr_d0(xrp-xr2) >> 15;
				p *= fpgauss_d0(yrp-yr1) >> 15;
				p >>= 15;
				p = p * ratio >> 15;

				bf(&fb.l[fbi], colour, p);
			}
	
			if (ix==bx1 && iy==by1) break;
			e2 = err*2;
			if (e2 > -dy) { err -= dy; ix += sx; }
			if (e2 < dx) { err += dx; iy += sy; }
		}
		//--------Bresenham routine--------

		bstartx += incx;
		bendx += incx;
		bstarty += incy;
		bendy += incy;
	}
}

void draw_line_thin_frgb(raster_t fb, double x1, double y1, double x2, double y2, double radius, frgb_t colour, const blend_func_fl_t bf, double intensity)
{
	int32_t i, iy, ix, fbi;
	double x3, y3, x4, y4;
	double d12, d12s, d12si, dx12, dy12, vx12, vy12, grad;
	float ixf, iyf, p, ratio=intensity;
	double iradius, bradius, bvx, bvy, x1l, y1l, x2l, y2l, x3l, y3l, x4l, y4l;
	int32_t bstartx, bstarty, bendx, bendy, incx, incy, incc;
	int bx0, by0, bx1, by1, dx, dy, sx, sy, err, e2;	// Bresenham routine variables
	float xr1, yr1, xr2, xrp, yrp;
	float th, costh, sinth;

	//grad = GAUSSRAD(intensity, radius);	// solves e^-x² = GAUSSLIMIT for x, giving 2.92 (the necessary Gaussian radius) for GAUSSLIMIT of 0.0002
	grad = GAUSSRAD_HQ * radius;			// erfr and gaussian can go up to x = ±4
	bradius = grad * sqrt(2.);		// bounding radius, the maximum radius necessary at each end of the line

	border_clip(fb.w-1, fb.h-1, &x1, &y1, &x2, &y2, grad);	// cut the part of the segment outside the screen

	dx12 = x2-x1;
	dy12 = y2-y1;
	d12 = hypot(dx12, dy12); if (d12==0.) return;
	d12s = d12 * d12;
	d12si = 1./d12s;
	vx12 = dx12/d12;
	vy12 = dy12/d12;

	x3 = x1 - bradius * vx12;
	y3 = y1 - bradius * vy12;
	x4 = x2 + bradius * vx12;
	y4 = y2 + bradius * vy12;
	bvx = -vy12;
	bvy = vx12;
	x1l = x1 - bvx * grad;
	y1l = y1 - bvy * grad;
	x2l = x2 - bvx * grad;
	y2l = y2 - bvy * grad;

	iradius = 1./radius;

	th = atan2(y2-y1, x2-x1);
	costh = iradius * cos(-th);
	sinth = iradius * sin(-th);
	xr1 = x1 * costh - y1 * sinth;
	yr1 = x1 * sinth + y1 * costh;
	xr2 = x2 * costh - y2 * sinth;

	incx = 949;
	if (dx12 >= fabs(dy12))	// start line left side of x1
	{
		x3l = x3;
		y3l = y3 - 1.;
		incx = 0;
		incy = 1;
	}
	if (dy12 >= fabs(dx12))	// start line up side of x1
	{
		x3l = x3 + 1.;
		y3l = y3;
		incx = -1;
		incy = 0;
	}
	if (-dx12 >= fabs(dy12))	// start line right side of x1
	{
		x3l = x3;
		y3l = y3 + 1.;
		incx = 0;
		incy = -1;
	}
	if (-dy12 >= fabs(dx12))	// start line down side of x1
	{
		x3l = x3 - 1.;
		y3l = y3;
		incx = 1;
		incy = 0;
	}
	if (incx==949)		// if the coordinates are invalid (like a NaN)
		return ;	// return

	line_line_intersection(x1l, y1l, x2l, y2l, x3, y3, x3l, y3l, &x3l, &y3l);

	x4l = x4 - x3 + x3l;
	y4l = y4 - y3 + y3l;

	if (incx+incy == 1)
	{
		bstartx = (int32_t) floor (x3l);
		bstarty = (int32_t) floor (y3l);
		bendx = (int32_t) floor (x4l);
		bendy = (int32_t) floor (y4l);

		if (incx==1)
			incc = (int32_t) ceil (x3*2-x3l) - bstartx;
		else
			incc = (int32_t) ceil (y3*2-y3l) - bstarty;
	}
	else
	{
		bstartx = (int32_t) ceil (x3l);
		bstarty = (int32_t) ceil (y3l);
		bendx = (int32_t) ceil (x4l);
		bendy = (int32_t) ceil (y4l);

		if (incx==-1)
			incc = bstartx - (int32_t) floor (x3*2-x3l);
		else
			incc = bstarty - (int32_t) floor (y3*2-y3l);
	}

	incc = fastabs(incc) + 1;

	// check for >45 degree angles
	if (dx12 >= fabs(dy12))	// start line left side of x1
		if (bendx-bstartx < fastabs(bendy-bstarty))
			bendy = fastabs(bendx-bstartx) * sign(bendy-bstarty) + bstarty;
	if (dy12 >= fabs(dx12))	// start line up side of x1
		if (bendy-bstarty < fastabs(bendx-bstartx))
			bendx = fastabs(bendy-bstarty) * sign(bendx-bstartx) + bstartx;
	if (-dx12 >= fabs(dy12))	// start line right side of x1
		if (-bendx+bstartx < fastabs(bendy-bstarty))
			bendy = fastabs(bendx-bstartx) * sign(bendy-bstarty) + bstarty;
	if (-dy12 >= fabs(dx12))	// start line down side of x1
		if (-bendy+bstarty < fastabs(bendx-bstartx))
			bendx = fastabs(bendy-bstarty) * sign(bendx-bstartx) + bstartx;

	for (i=0; i<incc; i++)
	{
		//********Bresenham routine********
		bx0 = bstartx;
		by0 = bstarty;
		bx1 = bendx;
		by1 = bendy;
	
		dx = fastabs(bx1-bx0);
		dy = fastabs(by1-by0);
		sx = -1; if (bx0 < bx1) sx = 1;
		sy = -1; if (by0 < by1) sy = 1;
		err = dx-dy; ixf = ix = bx0; iyf = iy = by0;
		
		while (1)
		{
			if (ix>=0 && ix<fb.w && iy>=0 && iy<fb.h)
			{
				fbi = iy*fb.w+ix;

				xrp = ixf * costh - iyf * sinth;
				yrp = ixf * sinth + iyf * costh;

				p = fasterfrf_d1(xrp-xr1);
				p -= fasterfrf_d1(xrp-xr2);
				p *= fastgaussianf_d1(yrp-yr1);
				p *= ratio;

				bf(&fb.f[fbi], colour, p);
			}
	
			if (ix==bx1 && iy==by1) break;
			e2 = err*2;
			if (e2 > -dy) { err -= dy; ix += sx; ixf = ix; }
			if (e2 < dx) { err += dx; iy += sy; iyf = iy; }
		}
		//--------Bresenham routine--------

		bstartx += incx;
		bendx += incx;
		bstarty += incy;
		bendy += incy;
	}
}

#ifdef RL_OPENCL
void draw_line_thin_cl(raster_t fb, double x1, double y1, double x2, double y2, double radius, frgb_t colour, const int bf, double intensity, int quality)
{
	double grad, iradius, th;
	cl_float r1x, r1y, r2x, costh, sinth;
	int32_t i, ix, iy;
	int32_t end, *di = fb.drawq_data;
	float *df = fb.drawq_data;
	const int dqtype = DQT_LINE_THIN_ADD;
	const int entry_size = drawq_entry_size(dqtype);
	xyi_t bb0, bb1;
	xy_t l1, l2, b1, b2;
	
	grad = GAUSSRAD_HQ * radius;		// erfr and gaussian can go up to x = ±4

	border_clip(fb.w-1, fb.h-1, &x1, &y1, &x2, &y2, grad);	// cut the part of the segment outside the screen
	if (x1==x2 && y1==y2)					// if there's no line to display
		return ;

	// calculate the bounding box for a first rectangular estimate of the sectors needed
	bb0.x = ceil (MINN(x1, x2) - grad);	bb0.x = MAXN(bb0.x, 0);
	bb0.y = ceil (MINN(y1, y2) - grad);	bb0.y = MAXN(bb0.y, 0);
	bb1.x = floor(MAXN(x1, x2) + grad);	bb1.x = MINN(bb1.x, fb.w-1);
	bb1.y = floor(MAXN(y1, y2) + grad);	bb1.y = MINN(bb1.y, fb.h-1);

	bb0.x >>= fb.sector_size;
	bb0.y >>= fb.sector_size;
	bb1.x >>= fb.sector_size;
	bb1.y >>= fb.sector_size;

	// calculate the drawing parameters
	iradius = 1./radius;
	th = atan2(y2-y1, x2-x1);	// TODO optimise atan2, cos and sin
	costh = cos(-th) * iradius;
	sinth = sin(-th) * iradius;
	r1x = x1 * costh - y1 * sinth;
	r1y = x1 * sinth + y1 * costh;
	r2x = x2 * costh - y2 * sinth;
	colour.r *= intensity;
	colour.g *= intensity;
	colour.b *= intensity;

	// store the drawing parameters in the main drawing queue
	end = di[DQ_END];
	if (end + entry_size + 2 >= fb.drawq_size)		// if there's not enough room left
	{
		fprintf(stderr, "Draw queue size exceeded, %d numbers already in (%02d)\n", di[0], rand()%100);
		return ;
	}

	di[end] = dqtype;
	end++;

	df[end + 0] = r1x;
	df[end + 1] = r1y;
	df[end + 2] = r2x;
	df[end + 3] = costh;
	df[end + 4] = sinth;
	df[end + 5] = colour.r;
	df[end + 6] = colour.g;
	df[end + 7] = colour.b;

	di[DQ_END] += entry_size + 1;

	fb.sector_list[DQ_ENTRY_START] = fb.sector_list[DQ_END];	// set the start of the new entry
	fb.sector_list[DQ_END]++;					// sector_list becomes 1 larger by having the sector count (=0) for the new entry added

	// find the affected sectors
	for (iy=bb0.y; iy<=bb1.y; iy++)
	{
		b1.y = iy << fb.sector_size;
		b2.y = b1.y + (1<<fb.sector_size) - 1;
		b1.y -= grad;
		b2.y += grad;

		for (ix=bb0.x; ix<=bb1.x; ix++)
		{
			// test if sector is actually needed
			b1.x = ix << fb.sector_size;
			b2.x = b1.x + (1<<fb.sector_size) - 1;
			b1.x -= grad;
			b2.x += grad;

			l1 = xy(x1, y1);
			l2 = xy(x2, y2);

			line_rect_clip(&l1, &l2, b1, b2);	// TODO optimise this

			// if sector is needed
			if (l1.x!=l2.x || l1.y!=l2.y)
			{
				drawq_add_sector_id(fb, iy*fb.sector_w + ix);	// add sector reference
			}
		}
	}
}
#endif

void draw_line_thin(raster_t fb, double x1, double y1, double x2, double y2, double radius, lrgb_t colour, const blend_func_t bf, double intensity)
{
	if (fb.use_cl)
			draw_line_thin_cl(fb, x1, y1, x2, y2, radius, make_colour_frgb_from_lrgb(colour), 0, intensity, 0);
	else if (fb.use_frgb)
		draw_line_thin_frgb(fb, x1, y1, x2, y2, radius, make_colour_frgb_from_lrgb(colour), get_blend_fl_equivalent(bf), intensity);
	else
		draw_line_thin_lrgb(fb, x1, y1, x2, y2, radius, colour, bf, intensity);
}

void draw_line_thin_xy(raster_t fb, xy_t p1, xy_t p2, double radius, lrgb_t colour, const blend_func_t bf, double intensity)
{
	draw_line_thin(fb, p1.x, p1.y, p2.x, p2.y, radius, colour, bf, intensity);
}

void draw_line_thin_rectclip(raster_t fb, xy_t p1, xy_t p2, xy_t b1, xy_t b2, double radius, lrgb_t colour, const blend_func_t bf, double intensity)
{
	line_rect_clip(&p1, &p2, b1, b2);
	draw_line_thin_xy(fb, p1, p2, radius, colour, bf, intensity);
}

void draw_line_thin_short(raster_t fb, double x1, double y1, double x2, double y2, double u1, double u2, double radius, lrgb_t colour, const blend_func_t bf, double intensity)
{
	double x3, y3, x4, y4;

	if (u2<=u1)
		return;

	if (u1!=0.)
	{
		x3 = x1 + u1 * (x2-x1);
		y3 = y1 + u1 * (y2-y1);
	}
	else
	{
		x3 = x1;
		y3 = y1;
	}

	if (u2!=1.)
	{
		x4 = x1 + u2 * (x2-x1);
		y4 = y1 + u2 * (y2-y1);
	}
	else
	{
		x4 = x2;
		y4 = y2;
	}

	draw_line_thin(fb, x3, y3, x4, y4, radius, colour, bf, intensity);
}
