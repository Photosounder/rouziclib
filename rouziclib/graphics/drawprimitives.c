void draw_circle(const int circlemode, raster_t fb, double x, double y, double circrad, double radius, lrgb_t colour, const blend_func_t bf, double intensity)
{
	double grad = GAUSSRAD(intensity, radius);
	int32_t ix, iy, fbi, lboundx, lboundy, rboundx, rboundy;
	int32_t p, ratio;	// 0.15
	int64_t lowbound2, highbound2, dx, dy, d, circradf;

	int32_t xf, yf, radf;
	const int32_t fp=16;
	const double fpratio = (double) (1<<fp);

	ratio = 32768. * intensity + 0.5;

	radf = roundaway(1./radius * fpratio);
	circradf = roundaway(circrad * fpratio);

	lboundx = (int32_t) ceil (x - circrad - grad);	if (lboundx<0) lboundx = 0; if (lboundx>=fb.w) lboundx = fb.w-1;
	lboundy = (int32_t) ceil (y - circrad - grad);	if (lboundy<0) lboundy = 0; if (lboundy>=fb.h) lboundy = fb.h-1;
	rboundx = (int32_t) floor (x + circrad + grad);	if (rboundx<0) rboundx = 0; if (rboundx>=fb.w) rboundx = fb.w-1;
	rboundy = (int32_t) floor (y + circrad + grad);	if (rboundy<0) rboundy = 0; if (rboundy>=fb.h) rboundy = fb.h-1;

	if (circlemode==HOLLOWCIRCLE)
	{
		lowbound2 = roundaway(MAXN(circrad - grad, 0.) * fpratio);
		lowbound2 *= lowbound2;
	}
	else
	{
		lowbound2 = -1;
	}

	highbound2 = roundaway((circrad + grad) * fpratio);
	highbound2 *= highbound2;

	xf = roundaway(x * fpratio);
	yf = roundaway(y * fpratio);

	for (iy=lboundy; iy<=rboundy; iy++)
	{
		dy = yf - (iy << fp);
		dy *= dy;

		for (ix=lboundx; ix<=rboundx; ix++)
		{
			fbi = iy*fb.w+ix;

			dx = xf - (ix << fp);
			dx *= dx;
			d = dx + dy;
			
			if (d>lowbound2 && d<highbound2)
			{
				d = (circradf - isqrt_d1i(d)) * radf >> fp;

				if (circlemode==FULLCIRCLE)
					p = fperfr_d0(d) >> 15;
				else	// HOLLOWCIRCLE
					p = fpgauss_d0(d) >> 15;

				p = p * ratio >> 15;

				bf(&fb.l[fbi], colour, p);
			}
		}
	}
}

void draw_rect(raster_t fb, xy_t p0, xy_t p1, double radius, lrgb_t colour, const blend_func_t bf, double intensity)
{
	draw_line_thin(fb, p0.x, p0.y, p0.x, p1.y, radius, colour, bf, intensity);
	draw_line_thin(fb, p0.x, p1.y, p1.x, p1.y, radius, colour, bf, intensity);
	draw_line_thin(fb, p1.x, p1.y, p1.x, p0.y, radius, colour, bf, intensity);
	draw_line_thin(fb, p1.x, p0.y, p0.x, p0.y, radius, colour, bf, intensity);
}

void draw_rect_chamfer(raster_t fb, xy_t p0, xy_t p1, double radius, lrgb_t colour, const blend_func_t bf, double intensity, double chamfer)
{
	xy_t p2, p3;
	double cs, csx, csy, sx=1., sy=1.;

	if (p1.x-p0.x < 0.)
		sx = -1.;
	if (p1.y-p0.y < 0.)
		sy = -1.;

	cs = chamfer * MINN(fabs(p1.x-p0.x), fabs(p1.y-p0.y));
	csx = cs*sx;
	csy = cs*sy;

	p2.x = p0.x+csx;
	p3.x = p1.x-csx;
	p2.y = p0.y+csy;
	p3.y = p1.y-csy;

	draw_line_thin(fb, p0.x, p2.y, p0.x, p3.y, radius, colour, bf, intensity);
	draw_line_thin(fb, p2.x, p1.y, p3.x, p1.y, radius, colour, bf, intensity);
	draw_line_thin(fb, p1.x, p3.y, p1.x, p2.y, radius, colour, bf, intensity);
	draw_line_thin(fb, p3.x, p0.y, p2.x, p0.y, radius, colour, bf, intensity);

	draw_line_thin(fb, p0.x, p3.y, p2.x, p1.y, radius, colour, bf, intensity);
	draw_line_thin(fb, p3.x, p1.y, p1.x, p3.y, radius, colour, bf, intensity);
	draw_line_thin(fb, p1.x, p2.y, p3.x, p0.y, radius, colour, bf, intensity);
	draw_line_thin(fb, p2.x, p0.y, p0.x, p2.y, radius, colour, bf, intensity);
}

int32_t get_dist_to_roundrect(int32_t lx1, int32_t ly1, int32_t lx2, int32_t ly2, int32_t corner, int32_t ixf, int32_t iyf)
{
	int32_t d;
	int32_t xsec, ysec, sec;

	ysec = 2;
	if (iyf < ly1)
		ysec = 1;
	else if (iyf > ly2)
		ysec = 3;

	xsec = 2;
	if (ixf < lx1)
		xsec = 1;
	else if (ixf > lx2)
		xsec = 3;

	sec = (xsec << 2) | ysec;

	// The lesser rectangle is the smaller rectangle from which the desired rounded rectangle is developed
	// It is what would happen if a circle of radius 'corner' was to be convolved with that lesser rectangle

	switch (sec)
	{
		case 5:  d = fphypot(lx1-ixf, ly1-iyf);	break;
		case 7:  d = fphypot(lx1-ixf, ly2-iyf);	break;
		case 13: d = fphypot(lx2-ixf, ly1-iyf);	break;
		case 15: d = fphypot(lx2-ixf, ly2-iyf);	break;
		case 6:  d = lx1 - ixf;	break;
		case 9:  d = ly1 - iyf;	break;
		case 11: d = iyf - ly2;	break;
		case 14: d = ixf - lx2;	break;

		case 10: d = 0;		break;
	}

	d -= corner;	// subtract the corner radius to the distance to the lesser rectangle to get the distance to the rounded rectangle

	return d;
}

void draw_roundrect(raster_t fb, double x1, double y1, double x2, double y2, double corner, double radius, lrgb_t colour, const blend_func_t bf, double intensity)
{	// The corner radius size must be greater than the full radius of the antialiasing (grad) which would be 0.8*2.9 = 2.32 px
	double grad = GAUSSRAD(intensity, radius);
	int32_t ix, iy, fbi, ratio, p, lboundx, lboundy, rboundx, rboundy;
	int32_t d, radf, x1f, y1f, x2f, y2f, ixf, iyf, cornerf;
	const int32_t fp=16;
	const double fpratio = (double) (1<<fp);

	ratio = 32768. * intensity + 0.5;

	radf = roundaway(1./radius * fpratio);
	x1f = roundaway((x1+corner) * fpratio);
	y1f = roundaway((y1+corner) * fpratio);
	x2f = roundaway((x2-corner) * fpratio);
	y2f = roundaway((y2-corner) * fpratio);
	cornerf = roundaway(corner * fpratio);

	lboundx = (int32_t) rangelimit(ceil (x1 - grad), 0., (double) (fb.w-1));
	lboundy = (int32_t) rangelimit(ceil (y1 - grad), 0., (double) (fb.h-1));
	rboundx = (int32_t) rangelimit(floor (x2 + grad), 0., (double) (fb.w-1));
	rboundy = (int32_t) rangelimit(floor (y2 + grad), 0., (double) (fb.h-1));

	for (iy=lboundy; iy<=rboundy; iy++)
	{
		iyf = iy << fp;

		for (ix=lboundx; ix<=rboundx; ix++)
		{
			fbi = iy*fb.w+ix;

			ixf = ix << fp;
			d = (int64_t) get_dist_to_roundrect(x1f, y1f, x2f, y2f, cornerf, ixf, iyf) * radf >> fp;
			p = fperfr_d0(-d) >> 15;
			p = p * ratio >> 15;

			bf(&fb.l[fbi], colour, p);
		}
	}
}

void draw_roundrect_frame(raster_t fb, double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4, double corner1, double corner2, double radius, lrgb_t colour, const blend_func_t bf, double intensity)
{	// The corner radius size must be greater than the full radius of the antialiasing (grad) which would be 0.8*2.9 = 2.32 px
	double grad = GAUSSRAD(intensity, radius);
	int32_t ix, iy, fbi, ratio, p, lboundx, lboundy, rboundx, rboundy;
	int32_t din, dout, radf, x1f, y1f, x2f, y2f, x3f, y3f, x4f, y4f, ixf, iyf, corner1f, corner2f;
	const int32_t fp=16;
	const double fpratio = (double) (1<<fp);

	ratio = 32768. * intensity + 0.5;

	radf = roundaway(1./radius * fpratio);
	x1f = roundaway((x1+corner1) * fpratio);
	y1f = roundaway((y1+corner1) * fpratio);
	x2f = roundaway((x2-corner1) * fpratio);
	y2f = roundaway((y2-corner1) * fpratio);
	x3f = roundaway((x3+corner2) * fpratio);
	y3f = roundaway((y3+corner2) * fpratio);
	x4f = roundaway((x4-corner2) * fpratio);
	y4f = roundaway((y4-corner2) * fpratio);
	corner1f = roundaway(corner1 * fpratio);
	corner2f = roundaway(corner2 * fpratio);

	lboundx = (int32_t) rangelimit(ceil (x1 - grad), 0., (double) (fb.w-1));
	lboundy = (int32_t) rangelimit(ceil (y1 - grad), 0., (double) (fb.h-1));
	rboundx = (int32_t) rangelimit(floor (x2 + grad), 0., (double) (fb.w-1));
	rboundy = (int32_t) rangelimit(floor (y2 + grad), 0., (double) (fb.h-1));

	for (iy=lboundy; iy<=rboundy; iy++)
	{
		iyf = iy << fp;

		for (ix=lboundx; ix<=rboundx; ix++)
		{
			fbi = iy*fb.w+ix;

			ixf = ix << fp;
			dout = (int64_t) get_dist_to_roundrect(x1f, y1f, x2f, y2f, corner1f, ixf, iyf) * radf >> fp;
			din = (int64_t) get_dist_to_roundrect(x3f, y3f, x4f, y4f, corner2f, ixf, iyf) * radf >> fp;
			p = fperfr_d0(-dout) - fperfr_d0(-din) >> 15;
			p = p * ratio >> 15;

			bf(&fb.l[fbi], colour, p);
		}
	}
}

// TODO optimise using fixed point arithmetic and look-up tables
void draw_polar_glow(raster_t fb, double cx, double cy, lrgb_t col, double colmul, double scale, double rad, double gradr, double gradth, double angle, int32_t islog, int32_t riserf, double erfrad, double pixoffset)
{
	int32_t ix, iy, g, ginv;
	double ixf, iyf, r, th, gx, gy;

	for (iy=0; iy<fb.h; iy++)
	{
		iyf = (double) iy - cy;

		for (ix=0; ix<fb.w; ix++)
		{
			ixf = (double) ix - cx;
			th = fastatan2(iyf*65536., ixf*65536.);		// range is (-pi, pi]
			r = sqrt(ixf*ixf + iyf*iyf);
			gy = (rad - (r/scale)) / gradr;
			if (islog)
				if (gy > 0.)
					gy = log(gy);
				else
					gy = 8.;

			if (riserf)	// if it's an erf and not a gaussian
				gy = fasterfrf_d1(gy + 0.5*erfrad) * fasterfrf_d1(-gy + 0.5*erfrad);
				//gy = (0.5*erf(gy + 0.5*erfrad) + 0.5) * (0.5*erf(-gy + 0.5*erfrad) + 0.5);
			else
				gy = fastgaussianf_d1(gy);
			gx = fastgaussianf_d1(rangewrap(th-angle, -pi, pi) * gradth);
			gx *= gy;
			gx *= colmul;	// intensity of colour
			gx += pixoffset;
			if (gx < 0.)
				gx = 0.;
			g = 32768. * gx + 0.5;
			ginv = 32768 - g;

			fb.l[iy*fb.w+ix].r = g * col.r + ginv * fb.l[iy*fb.w+ix].r >> 15;
			fb.l[iy*fb.w+ix].g = g * col.g + ginv * fb.l[iy*fb.w+ix].g >> 15;
			fb.l[iy*fb.w+ix].b = g * col.b + ginv * fb.l[iy*fb.w+ix].b >> 15;
			fb.l[iy*fb.w+ix].a = ONEF;
		}
	}
}

// TODO optimise using fixed point arithmetic and look-up tables
void draw_gaussian_gradient(raster_t fb, double cx, double cy, lrgb_t c0, lrgb_t c1, double gausrad, double gausoffx, double gausoffy, const blend_func_t bf)
{
	int32_t ix, iy, p;
	double gx, gy;
	lrgb_t gc;

	for (iy=0; iy<fb.h; iy++)
	{
		gy = fastgaussianf_d1((cy - (double) iy) / gausrad + gausoffy);

		for (ix=0; ix<fb.w; ix++)
		{
			gx = fastgaussianf_d1((cx - (double) ix) / gausrad + gausoffx);
			gx *= gy;
			p = 32768. * gx + 0.5;

			gc = c0;
			blend_blend(&gc, c1, p);
			bf(&fb.l[iy*fb.w+ix], gc, 32768);
		}
	}
}

void draw_point_lrgb(raster_t fb, double x, double y, double radius, lrgb_t colour, const blend_func_t bf, double intensity)
{
	int32_t iy, ix, fbi;
	double grad;
	int32_t p, ratio;		// 0.15
	double bradius;
	int32_t bstartx, bstarty, bendx, bendy;

	const int32_t fp=16, fpr = 29;
	const double fpratio = (double) (1<<fp);
	int32_t xf, yf, radiusf;

	xf = roundaway(x * fpratio);
	yf = roundaway(y * fpratio);

	grad = sqrt(log(1. / (GAUSSLIMIT*intensity)));

	ratio = 32768. * intensity + 0.5;

	bradius = grad * radius;	// 2.36 for a radius of 1
	radius = 1./radius;
	radiusf = roundaway(radius * (double) (1<<29));

	bstartx = (int32_t) ceil(x - bradius);	if (bstartx<0)	bstartx = 0;
	bstarty = (int32_t) ceil(y - bradius);	if (bstarty<0)	bstarty = 0;
	bendx = (int32_t) floor(x + bradius);	if (bendx >= fb.w) bendx = fb.w-1;
	bendy = (int32_t) floor(y + bradius);	if (bendy >= fb.h) bendy = fb.h-1;

	for (iy=bstarty; iy<=bendy; iy++)
	for (ix=bstartx; ix<=bendx; ix++)
	{
		fbi = iy*fb.w+ix;

		p = fpgauss_d0((int64_t) (xf - (ix<<fp)) * radiusf >> fpr) >> 15;	// 0.15
		p *= fpgauss_d0((int64_t) (yf - (iy<<fp)) * radiusf >> fpr) >> 15;
		p = (p>>15) * ratio >> 15;

		bf(&fb.l[fbi], colour, p);
	}
}

void draw_point_frgb(raster_t fb, double x, double y, double radius, frgb_t colour, const blend_func_fl_t bf, double intensity)
{
	int32_t iy, ix, fbi;
	float ixf, iyf, p, bradius, ratio=intensity;
	int32_t bstartx, bstarty, bendx, bendy;

	bradius = GAUSSRAD_HQ * radius;

	radius = 1./radius;

	bstartx = (int32_t) ceil(x - bradius);	if (bstartx<0)	bstartx = 0;
	bstarty = (int32_t) ceil(y - bradius);	if (bstarty<0)	bstarty = 0;
	bendx = (int32_t) floor(x + bradius);	if (bendx >= fb.w) bendx = fb.w-1;
	bendy = (int32_t) floor(y + bradius);	if (bendy >= fb.h) bendy = fb.h-1;

	for (iyf=iy=bstarty; iy<=bendy; iy++, iyf+=1.)
	for (ixf=ix=bstartx; ix<=bendx; ix++, ixf+=1.)
	{
		fbi = iy*fb.w+ix;

		p = fastgaussianf_d1((x - ixf) * radius);
		p *= fastgaussianf_d1((y - iyf) * radius);
		p *= ratio;

		bf(&fb.f[fbi], colour, p);
	}
}

#ifdef RL_OPENCL
void draw_point_cl(raster_t fb, double x, double y, double radius, frgb_t colour, const blend_func_fl_t bf, double intensity)
{
	double grad;
	int32_t ix, iy;
	int32_t end, *di = fb.drawq_data;
	float *df = fb.drawq_data;
	const int dqtype = DQT_POINT_ADD;
	const int entry_size = drawq_entry_size(dqtype);
	xyi_t bb0, bb1;
	
	grad = GAUSSRAD_HQ * radius;		// gaussian will go to x = ±4, radially

	if (x + grad < 0.)			return ;
	if (y + grad < 0.)			return ;
	if (x - grad > (double) (fb.w-1))	return ;
	if (y - grad > (double) (fb.h-1))	return ;

	// calculate the bounding box
	bb0.x = MAXN(ceil(x - grad), 0);
	bb0.y = MAXN(ceil(y - grad), 0);
	bb1.x = MINN(floor(x + grad), fb.w-1);
	bb1.y = MINN(floor(y + grad), fb.h-1);

	bb0.x >>= fb.sector_size;
	bb0.y >>= fb.sector_size;
	bb1.x >>= fb.sector_size;
	bb1.y >>= fb.sector_size;

	// store the drawing parameters in the main drawing queue
	end = di[DQ_END];
	if (end + entry_size + 2 >= fb.drawq_size)		// if there's not enough room left
	{
		fprintf(stderr, "Draw queue size exceeded, %d numbers already in (%02d)\n", di[0], rand()%100);
		return ;
	}

	di[end] = dqtype;
	end++;

	// enter drawing parameters
	df[end + 0] = x;
	df[end + 1] = y;
	df[end + 2] = 1./radius;
	df[end + 3] = colour.r * intensity;
	df[end + 4] = colour.g * intensity;
	df[end + 5] = colour.b * intensity;

	di[DQ_END] += entry_size + 1;

	fb.sector_list[DQ_ENTRY_START] = fb.sector_list[DQ_END];	// set the start of the new entry
	fb.sector_list[DQ_END]++;					// sector_list becomes 1 larger by having the sector count (=0) for the new entry added

	// go through the affected sectors
	for (iy=bb0.y; iy<=bb1.y; iy++)
		for (ix=bb0.x; ix<=bb1.x; ix++)
			drawq_add_sector_id(fb, iy*fb.sector_w + ix);	// add sector reference
}
#endif

void draw_point(raster_t fb, double x, double y, double radius, lrgb_t colour, const blend_func_t bf, double intensity)
{
	if (fb.use_cl)
		draw_point_cl(fb, x, y, radius, make_colour_frgb_from_lrgb(colour), get_blend_fl_equivalent(bf), intensity);
	else if (fb.use_frgb)
		draw_point_frgb(fb, x, y, radius, make_colour_frgb_from_lrgb(colour), get_blend_fl_equivalent(bf), intensity);
	else
		draw_point_lrgb(fb, x, y, radius, colour, bf, intensity);
}

void draw_point_xy(raster_t fb, xy_t p, double radius, lrgb_t colour, const blend_func_t bf, double intensity)
{
	draw_point(fb, p.x, p.y, radius, colour, bf, intensity);
}

void draw_point_on_row(raster_t fb, double x, int32_t y, double radius, lrgb_t colour, const blend_func_t bf, double intensity)
{
	int32_t ix, fbi, dp;
	double grad = GAUSSRAD(intensity, radius);
	int32_t p, ratio;	// 0.15
	int32_t bstartx, bendx;

	int32_t xf, ixf, radf;
	const int32_t fp=16;
	const double fpratio = (double) (1<<fp);

	ratio = 32768. * intensity + 0.5;

	radf = roundaway(1./radius * fpratio);

	bstartx = (int32_t) ceil(x - grad);	if (bstartx<0)	bstartx = 0;
	bendx = (int32_t) floor(x + grad);	if (bendx >= fb.w) bendx = fb.w-1;

	xf = roundaway(x * fpratio);

	for (ix=bstartx; ix<=bendx; ix++)
	{
		fbi = y*fb.w+ix;
		ixf = ix << fp;

		dp = xf - ixf;
		dp = (int64_t) dp * radf >> fp;

		p = fpgauss_d0(dp) >> 15;
		p = p * ratio >> 15;

		bf(&fb.l[fbi], colour, p);
	}
}
