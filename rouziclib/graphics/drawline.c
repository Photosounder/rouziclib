void draw_line_thin(lrgb_t *fb, int32_t w, int32_t h, double x1, double y1, double x2, double y2, double radius, lrgb_t colour, const int mode, double intensity)
{
	int32_t i, iy, ix, fbi;
	double x3, y3, x4, y4;
	double d12, d12s, d12si, dx12, dy12, vx12, vy12, grad;
	int32_t p, ratio;		// 0.15
	double bradius, bvx, bvy, x1l, y1l, x2l, y2l, x3l, y3l, x4l, y4l;
	int32_t bstartx, bstarty, bendx, bendy, incx, incy, incc;
	int bx0, by0, bx1, by1, dx, dy, sx, sy, err, e2;	// Bresenham routine variables

	int32_t x1f, y1f, x2f, y2f;
	int32_t xr1, yr1, xr2, xrp, yrp;
	int32_t th, costh, sinth;
	const int32_t fp=16, fpi=30-fp;
	const double fpratio = (double) (1<<fp);

	grad = GAUSSRAD(intensity, radius);	// solves e^-x² = GAUSSLIMIT for x, giving 2.92 (the necessary Gaussian radius) for GAUSSLIMIT of 0.0002
	bradius = grad * sqrt(2.);	// bounding radius, the maximum radius necessary at each end of the line

	border_clip(w, h, &x1, &y1, &x2, &y2, grad);	// cut the part of the segment outside the screen

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

	radius = 1./radius;

	x1f = roundaway(x1 * fpratio);
	y1f = roundaway(y1 * fpratio);
	x2f = roundaway(x2 * fpratio);
	y2f = roundaway(y2 * fpratio);

	th = fpatan2(y2f-y1f, x2f-x1f);
	costh = radius * fpcos(-th);			// radius is double, can be above 1.0, not sure how that doesn't overflow
	sinth = radius * fpcos(-th - (1L<<30));
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
			if (ix>=0 && ix<w && iy>=0 && iy<h)
			{
				fbi = iy*w+ix;
				xrp = ((int64_t) ix * costh >> fpi) - ((int64_t) iy * sinth >> fpi);
				yrp = ((int64_t) ix * sinth >> fpi) + ((int64_t) iy * costh >> fpi);

				p = fperfr(xrp-xr1) >> 15;
				p *= fperfr(xr2-xrp) >> 15;
				p >>= 15;
				p *= fpgauss(yrp-yr1) >> 15;
				p >>= 15;
				p = p * ratio >> 15;

				fb[fbi] = blend_pixels(fb[fbi], colour, p, mode);
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
