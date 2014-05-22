void draw_circle(const int circlemode, lrgb_t *fb, int32_t w, int32_t h, double x, double y, double circrad, double radius, lrgb_t colour, const int blendingmode, double intensity)
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

	lboundx = (int32_t) ceil (x - circrad - grad);	if (lboundx<0) lboundx = 0; if (lboundx>=w) lboundx = w-1;
	lboundy = (int32_t) ceil (y - circrad - grad);	if (lboundy<0) lboundy = 0; if (lboundy>=h) lboundy = h-1;
	rboundx = (int32_t) floor (x + circrad + grad);	if (rboundx<0) rboundx = 0; if (rboundx>=w) rboundx = w-1;
	rboundy = (int32_t) floor (y + circrad + grad);	if (rboundy<0) rboundy = 0; if (rboundy>=h) rboundy = h-1;

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
			fbi = iy*w+ix;

			dx = xf - (ix << fp);
			dx *= dx;
			d = dx + dy;
			
			if (d>lowbound2 && d<highbound2)
			{
				d = (circradf - isqrt(d)) * radf >> fp;

				if (circlemode==FULLCIRCLE)
					p = fperfr(d) >> 15;
				else	// HOLLOWCIRCLE
					p = fpgauss(d) >> 15;

				p = p * ratio >> 15;

				fb[fbi] = blend_pixels(fb[fbi], colour, p, blendingmode);
			}
		}
	}
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

void draw_roundrect(lrgb_t *fb, int32_t w, int32_t h, double x1, double y1, double x2, double y2, double corner, double radius, lrgb_t colour, const int mode, double intensity)
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

	lboundx = (int32_t) rangelimit(ceil (x1 - grad), 0., (double) (w-1));
	lboundy = (int32_t) rangelimit(ceil (y1 - grad), 0., (double) (h-1));
	rboundx = (int32_t) rangelimit(floor (x2 + grad), 0., (double) (w-1));
	rboundy = (int32_t) rangelimit(floor (y2 + grad), 0., (double) (h-1));

	for (iy=lboundy; iy<=rboundy; iy++)
	{
		iyf = iy << fp;

		for (ix=lboundx; ix<=rboundx; ix++)
		{
			fbi = iy*w+ix;

			ixf = ix << fp;
			d = (int64_t) get_dist_to_roundrect(x1f, y1f, x2f, y2f, cornerf, ixf, iyf) * radf >> fp;
			p = fperfr(-d) >> 15;
			p = p * ratio >> 15;

			fb[fbi] = blend_pixels(fb[fbi], colour, p, mode);
		}
	}
}

void draw_roundrect_frame(lrgb_t *fb, int32_t w, int32_t h, double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4, double corner1, double corner2, double radius, lrgb_t colour, const int mode, double intensity)
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

	lboundx = (int32_t) rangelimit(ceil (x1 - grad), 0., (double) (w-1));
	lboundy = (int32_t) rangelimit(ceil (y1 - grad), 0., (double) (h-1));
	rboundx = (int32_t) rangelimit(floor (x2 + grad), 0., (double) (w-1));
	rboundy = (int32_t) rangelimit(floor (y2 + grad), 0., (double) (h-1));

	for (iy=lboundy; iy<=rboundy; iy++)
	{
		iyf = iy << fp;

		for (ix=lboundx; ix<=rboundx; ix++)
		{
			fbi = iy*w+ix;

			ixf = ix << fp;
			dout = (int64_t) get_dist_to_roundrect(x1f, y1f, x2f, y2f, corner1f, ixf, iyf) * radf >> fp;
			din = (int64_t) get_dist_to_roundrect(x3f, y3f, x4f, y4f, corner2f, ixf, iyf) * radf >> fp;
			p = fperfr(-dout) - fperfr(-din) >> 15;
			p = p * ratio >> 15;

			fb[fbi] = blend_pixels(fb[fbi], colour, p, mode);
		}
	}
}

// TODO optimise using fixed point arithmetic and look-up tables
void draw_polar_glow(lrgb_t *fb, int32_t w, int32_t h, double cx, double cy, lrgb_t col, double colmul, double scale, double rad, double gradr, double gradth, double angle, int32_t islog, int32_t riserf, double erfrad, double pixoffset)
{
	int32_t ix, iy, g, ginv;
	double ixf, iyf, r, th, gx, gy;

	for (iy=0; iy<h; iy++)
	{
		iyf = (double) iy - cy;

		for (ix=0; ix<w; ix++)
		{
			ixf = (double) ix - cx;
			th = atan2(iyf, ixf);		// range is (-pi, pi]
			r = sqrt(ixf*ixf + iyf*iyf);
			gy = (rad - (r/scale)) / gradr;
			if (islog)
				if (gy > 0.)
					gy = log(gy);
				else
					gy = 8.;

			if (riserf)	// if it's an erf and not a gaussian
				gy = (0.5*erf(gy + 0.5*erfrad) + 0.5) * (0.5*erf(-gy + 0.5*erfrad) + 0.5);
			else
				gy = gaussian(gy);
			gx = gaussian(rangewrap(th-angle, -pi, pi) * gradth);
			gx *= gy;
			gx *= colmul;	// intensity of colour
			gx += pixoffset;
			if (gx < 0.)
				gx = 0.;
			g = 32768. * gx + 0.5;
			ginv = 32768 - g;

			fb[iy*w+ix].r = g * col.r + ginv * fb[iy*w+ix].r >> 15;
			fb[iy*w+ix].g = g * col.g + ginv * fb[iy*w+ix].g >> 15;
			fb[iy*w+ix].b = g * col.b + ginv * fb[iy*w+ix].b >> 15;
			fb[iy*w+ix].a = ONEF;
		}
	}
}

// TODO optimise using fixed point arithmetic and look-up tables
void draw_gaussian_gradient(lrgb_t *fb, int32_t w, int32_t h, double cx, double cy, lrgb_t c0, lrgb_t c1, double gausrad, double gausoffx, double gausoffy)
{
	int32_t ix, iy, p;
	double gx, gy;

	for (iy=0; iy<h; iy++)
	{
		gy = gaussian((cy - (double) iy) / gausrad + gausoffy);

		for (ix=0; ix<w; ix++)
		{
			gx = gaussian((cx - (double) ix) / gausrad + gausoffx);
			gx *= gy;
			p = 32768. * gx + 0.5;

			fb[iy*w+ix] = blend_pixels(c0, c1, p, BLEND);
		}
	}
}
