// only used in spacewar's blit_hrgb_sprite* (game.c)
int sprite_offsets_old(int32_t fbw, int32_t fbh, int32_t spw, int32_t sph, int32_t *pos_x, int32_t *pos_y, int32_t *offset_x, int32_t *offset_y, int32_t *start_x, int32_t *start_y, int32_t *stop_x, int32_t *stop_y, int hmode, int vmode)
{
	if (hmode==A_CEN)	*pos_x -= (spw>>1);
	if (hmode==A_RIG)	*pos_x -= (spw-1);
	if (vmode==A_CEN)	*pos_y -= (sph>>1);
	if (vmode==A_BOT)	*pos_y -= (sph-1);

	*offset_x = *pos_x;
	*offset_y = *pos_y;
	if (*pos_x < 0)		*start_x = -*pos_x;	else	*start_x = 0;
	if (*pos_y < 0)		*start_y = -*pos_y;	else	*start_y = 0;
	if (*pos_x+spw >= fbw)	*stop_x = fbw - *pos_x;	else	*stop_x = spw;
	if (*pos_y+sph >= fbh)	*stop_y = fbh - *pos_y;	else	*stop_y = sph;

	if (*stop_x <= *start_x || *stop_y <= *start_y)
		return 1;					// a return value of 1 means the sprite is off-screen
	return 0;
}

int sprite_offsets(raster_t r0, raster_t r1, xyi_t *pos, xyi_t *offset, xyi_t *start, xyi_t *stop, int hmode, int vmode)
{
	if (hmode==A_CEN)	pos->x -= (r1.w>>1);
	if (hmode==A_RIG)	pos->x -= (r1.w-1);
	if (vmode==A_CEN)	pos->y -= (r1.h>>1);
	if (vmode==A_BOT)	pos->y -= (r1.h-1);

	offset->x = pos->x;
	offset->y = pos->y;
	if (pos->x < 0)			start->x = -pos->x;		else	start->x = 0;
	if (pos->y < 0)			start->y = -pos->y;		else	start->y = 0;
	if (pos->x+r1.w >= r0.w)	stop->x = r0.w - pos->x;	else	stop->x = r1.w;
	if (pos->y+r1.h >= r0.h)	stop->y = r0.h - pos->y;	else	stop->y = r1.h;

	if (stop->x <= start->x || stop->y <= start->y)
		return 1;					// a return value of 1 means the sprite is off-screen
	return 0;
}

#include <string.h>	// for memcpy
void blit_sprite(raster_t r0, raster_t r1, xyi_t pos, const blend_func_t bf, int hmode, int vmode)
{
	int32_t iy_r0, iy_r1;
	int32_t ix, iy;
	xyi_t offset, start, stop;

	if (sprite_offsets(r0, r1, &pos, &offset, &start, &stop, hmode, vmode))
		return ;

	if (bf==blend_solid)	// if the sprite is opaque then it can be blitted whole lines at a time
	{
		for (iy=start.y; iy<stop.y; iy++)
		{
			iy_r0 = (iy + offset.y) * r0.w + offset.x;
			iy_r1 = iy * r1.w;
			memcpy(&r0.l[iy_r0+start.x], &r1.l[iy_r1+start.x], (stop.x-start.x) * sizeof(lrgb_t));
		}
		return;
	}

	for (iy=start.y; iy<stop.y; iy++)
	{
		iy_r0 = (iy + offset.y) * r0.w + offset.x;
		iy_r1 = iy * r1.w;

		for (ix=start.x; ix<stop.x; ix++)
		{
			bf(&r0.l[iy_r0+ix], r1.l[iy_r1+ix], 32768);
		}
	}
}

void blit_layout(raster_t r0, raster_t r1)
{
	int32_t i, wh;
	lrgb_t *p;

	// Current layout: ONES: 349998   MIDS: 20470   ZEROS: 169436
	// Cycles:	ONES: 7		MIDS: 11	ZEROS: ~0

	wh = r0.w * r0.h;
	for (i=0; i<wh; i++)
	{
		p = &r1.l[i];

		if (p->a)
		if (p->a==ONE)
			r0.l[i] = *p;
		else
		{
			r0.l[i].r = (((int32_t) p->r - (int32_t) r0.l[i].r) * p->a >> LBD) + r0.l[i].r;
			r0.l[i].g = (((int32_t) p->g - (int32_t) r0.l[i].g) * p->a >> LBD) + r0.l[i].g;
			r0.l[i].b = (((int32_t) p->b - (int32_t) r0.l[i].b) * p->a >> LBD) + r0.l[i].b;
		}
	}
}

double blit_scale_func_linear(double x, double unit_inv, interp_param_t p)
{
	uint64_t *xp = (uint64_t *) &x;
	
	*xp &= 0x7FFFFFFFFFFFFFFF;	// fabs() equivalent
	x *= unit_inv;

	return (1. - x) * unit_inv;
	//return (0.5+0.5*cos(pi*x)) * unit_inv;
}

double blit_scale_func_modlin(double x, double unit_inv, interp_param_t p)
{
	uint64_t *xp = (uint64_t *) &x;
	
	*xp &= 0x7FFFFFFFFFFFFFFF;	// fabs() equivalent

	if (x < p.knee)
		return p.top;
	else
		return p.c1*x + p.c0;
}

/*interp_param_t calc_interp_param_modlin(double n)
{
	interp_param_t p;
	double a, b, middle, peak, trough;
	double ni = 1./n;	// n = ]0 , 1], ni >= 1

	if (ni == floor(ni))				// if ni is an integer like 1.0, 2.0, 3.0...
		p.func = blit_scale_func_linear;	// this means regular linear filtering should be used
	else
		p.func = blit_scale_func_modlin;	// this means my modified filtering should be used

	// Knee position
	p.knee = 0.5 - fabs(fmod(ni, 1.) - 0.5);	// p.knee = [0 , 0.5]

	// Uncorrected trough height
	a = floor(ni-0.5) + 0.5;
	b = a + 1.;
	middle = 0.5*(a+b);
	trough = 2.*middle - n*middle*middle;
p.trough = trough;
	trough = (1. - p.knee*n) / trough;	// corrected

	// Uncorrected peak height
	a = floor(ni);
	b = a + 1.;
	peak = (2.*b - 1.) - a*b*n;
p.peak = peak;
	peak = trough;		// corrected

	// uncorrected_peak = 1. + k*trough
	// old troughs part = uncorrected_peak - 1.
	// old troughs ratio = (uncorrected_peak - 1.) / uncorrected_peak
	// new troughs part = (uncorrected_peak - 1.) * new trough / old trough

	p.ac1 = (trough-peak) / p.knee;
	p.ac0 = peak;

	p.bc1 = (0.-trough) / (ni-p.knee);
	p.bc0 = 0. - p.bc1*ni;

	return p;
}*/

interp_param_t calc_interp_param_modlin(double n)
{
	interp_param_t p;
	double m, trough, ni = 1./n;	// n = ]0 , 1], ni >= 1

	if (ni == floor(ni))				// if ni is an integer like 1.0, 2.0, 3.0...
		p.func = blit_scale_func_linear;	// this means regular linear filtering should be used
	else
		p.func = blit_scale_func_modlin;	// this means my modified filtering should be used

	p.knee = 0.5 - fabs(fmod(ni, 1.) - 0.5);	// Knee position = [0 , 0.5]

	m = floor(ni+0.5);			// mid-point of the current trough segment
	trough = 2.*m - n*m*m;			// uncorrected trough height
	trough = (1. - p.knee*n) / trough;	// corrected

	p.top = trough;
	p.c1 = -trough / (ni-p.knee);
	p.c0 = -p.c1 * ni;

	return p;
}

void blit_scale_nearest(raster_t r0, raster_t r1, xy_t pos, xy_t ipscale, xyi_t start, xyi_t stop)
{
	int32_t ix, iy, biyw;
	static int32_t lastw=0, *xluti=NULL;

	if (lastw <= r0.w)		// if the width of the screen has increased
	{
		lastw = r0.w;
		if (xluti) free (xluti);
		xluti = calloc (r0.w, sizeof(int32_t));	// change the size of xluti
	}

	for (ix=start.x; ix<stop.x; ix++)	// recompute LUT
		xluti[ix] = rangelimit_i32( nearbyint(((double) ix - pos.x) * ipscale.x) , 0, r1.w-1);

	for (iy=start.y; iy<stop.y; iy++)
	{
		biyw = rangelimit_i32( nearbyint(((double) iy - pos.y) * ipscale.y) , 0, r1.h-1) * r1.w;

		for (ix=start.x; ix<stop.x; ix++)
		{
			r0.l[iy*r0.w + ix] = r1.l[biyw + xluti[ix]];
		}
	}
}

void blit_scale_lrgb(raster_t r0, raster_t r1, xy_t pscale, xy_t pos, int interp)
{
	int32_t i, ic, iy, ix, jx, jy;
	xyi_t start, stop, jstart, jstop;
	float sumf[4];
	xy_t p0, p1, pin, kr0, ikr0, kr1, ipscale = inv_xy(pscale), iw;
	int32_t nsx, nsy;	// number of samples to get
	double iw_xy;
	interp_param_t param_x, param_y;

	if (r1.l==NULL)
		return ;

	switch (interp)
	{
		case NEAREST_INTERP:
			kr1 = set_xy(0.5);	// kernel radiuses at the rescaled level
			break;

		case LINEAR_INTERP:
			kr1 = set_xy(1.0);
			break;

		case GAUSSIAN_INTERP:
			kr1 = set_xy(2.4);	// pretty arbitrary
			break;
	}

	kr0 = kr1;				// kernel radiuses at the unscaled level
	if (pscale.x < 1.)
		kr0.x *= ipscale.x;

	if (pscale.y < 1.)
		kr0.y *= ipscale.y;

	ikr0 = inv_xy(kr0);			// ikr0 = ]0 , 1]

	param_x = calc_interp_param_modlin(ikr0.x);
	param_y = calc_interp_param_modlin(ikr0.y);

	nsx = kr0.x * 2.;			// number of input samples necessary for each pixel
	nsy = kr0.y * 2.;

	// find start and stop indices
	p0 = add_xy(pos, mul_xy(pscale, neg_xy(kr1)));
	p1 = add_xy(pos, mul_xy(pscale, add_xy(kr1, xy(r1.w-1, r1.h-1))));

	start.x = MAXN(0, floor(MINN(p0.x, p1.x))+1);
	start.y = MAXN(0, floor(MINN(p0.y, p1.y))+1);
	stop.x = MINN(r0.w, ceil(MAXN(p0.x, p1.x)));
	stop.y = MINN(r0.h, ceil(MAXN(p0.y, p1.y)));

	if (nsx*nsy == 1)		// if unfiltered nearest neighbour (only 1 pixel in -> 1 pixel out)
	{
		blit_scale_nearest(r0, r1, pos, ipscale, start, stop);
	}
	else
	{
		for (iy=start.y; iy<stop.y; iy++)
		{
			pin.y = ((double) iy - pos.y) * ipscale.y;
			jstart.y = floor(pin.y - kr0.y)+1;		if (jstart.y < 0) jstart.y = 0;
			jstop.y = ceil(pin.y + kr0.y);			if (jstop.y > r1.h) jstop.y = r1.h;

			for (ix=start.x; ix<stop.x; ix++)
			{
				pin.x = ((double) ix - pos.x) * ipscale.x;
				jstart.x = floor(pin.x - kr0.x)+1;	if (jstart.x < 0) jstart.x = 0;
				jstop.x = ceil(pin.x + kr0.x);		if (jstop.x > r1.w) jstop.x = r1.w;
				//jstop.x = jstart.x + nsx;	if (jstop.x > r1.w) jstop.x = r1.w;

				memset(sumf, 0, 4*sizeof(float));		// blank the new sum pixel

			/*	switch (jstop.x-jstart.x-1)
				{
					case 0:
						sumf[0] = ONEF*0.1;
						sumf[2] = ONEF*0.1;
						break;
					case 1:
						sumf[2] = ONEF;
						break;
					case 2:
						sumf[0] = ONEF;
						break;
					case 3:
						sumf[1] = ONEF;
						break;
					default:
						sumf[0] = ONEF;
						sumf[1] = ONEF;
						sumf[2] = ONEF;
						break;
				}*/

				for (jy=jstart.y; jy<jstop.y; jy++)
				{
					iw.y = param_y.func((double) jy - pin.y, ikr0.y, param_y);

					for (jx=jstart.x; jx<jstop.x; jx++)
					{
						iw.x = param_x.func((double) jx - pin.x, ikr0.x, param_x);
						iw_xy = iw.x * iw.y;		// interpolation weight
						i = jy * r1.w + jx;

						for (ic=0; ic<3; ic++)
						{
							sumf[ic] += ((uint16_t *) &r1.l[i])[ic] * iw_xy;
						}
					}
				}

				for (ic=0; ic<3; ic++)
					((uint16_t *) &r0.l[iy*r0.w + ix])[ic] = sumf[ic] + 0.5;
			}
		}
	}
}

void blit_scale_cl(raster_t r0, raster_t r1, xy_t pscale, xy_t pos, int interp)
{
#ifdef RL_OPENCL
	double grad;
	int32_t ix, iy;
	int32_t *di;
	float *df;
	xyi_t bb0, bb1;

	if (r1.f==NULL)
		return ;

	grad = 1.;	// 1 for triangular filtering
	
/*	if (pos.x + grad < 0.)			return ;
	if (pos.y + grad < 0.)			return ;
	if (pos.x - grad > (double) (r0.w-1))	return ;
	if (pos.y - grad > (double) (r0.h-1))	return ;

	// calculate the bounding box
	bb0.x = MAXN(ceil(x - grad), 0);
	bb0.y = MAXN(ceil(y - grad), 0);
	bb1.x = MINN(floor(x + grad), r0.w-1);
	bb1.y = MINN(floor(y + grad), r0.h-1);*/
bb0 = xyi(0, 0);
bb1.x = MINN(r0.w-1, r0.w-1);
bb1.y = MINN(r0.h-1, r0.h-1);

	bb0.x >>= r0.sector_size;
	bb0.y >>= r0.sector_size;
	bb1.x >>= r0.sector_size;
	bb1.y >>= r0.sector_size;

	// store the drawing parameters in the main drawing queue
	df = di = drawq_add_to_main_queue(r0, DQT_BLIT_SPRITE);
	di[0] = r1.clbuf_da;
	di[1] = r1.clbuf_da >> 32;
	di[2] = r1.w;
	di[3] = r1.h;
	df[4] = 1./pscale.x;
	df[5] = 1./pscale.y;
	df[6] = -pos.x;
	df[7] = -pos.y;

	// go through the affected sectors
	for (iy=bb0.y; iy<=bb1.y; iy++)
		for (ix=bb0.x; ix<=bb1.x; ix++)
			drawq_add_sector_id(r0, iy*r0.sector_w + ix);	// add sector reference
#endif
}

void blit_scale(raster_t r0, raster_t r1, xy_t pscale, xy_t pos, int interp)
{
	if (r0.use_cl)
		blit_scale_cl(r0, r1, pscale, pos, interp);
	else if (r0.use_frgb==0)
		blit_scale_lrgb(r0, r1, pscale, pos, interp);
}

void blit_in_rect(raster_t r0, raster_t r1, rect_t r, int keep_aspect_ratio, int interp)
{
	xy_t pscale, pos;

	pscale = div_xy(get_rect_dim(r), xy(r1.w, r1.h));
	pos = add_xy(rect_p01(r), mul_xy(pscale, set_xy(0.5)));

	blit_scale(r0, r1, pscale, pos, interp);
}

void blit_scale_photo_cl(raster_t r0, raster_t r1, xy_t pscale, xy_t pos, int interp, xy_t pc, double distortion, double gain)
{
#ifdef RL_OPENCL
	double grad;
	int32_t ix, iy;
	int32_t *di;
	float *df;
	xyi_t bb0, bb1;

	if (r1.f==NULL)
		return ;

	grad = 1.;	// 1 for triangular filtering
	
/*	if (pos.x + grad < 0.)			return ;
	if (pos.y + grad < 0.)			return ;
	if (pos.x - grad > (double) (r0.w-1))	return ;
	if (pos.y - grad > (double) (r0.h-1))	return ;

	// calculate the bounding box
	bb0.x = MAXN(ceil(x - grad), 0);
	bb0.y = MAXN(ceil(y - grad), 0);
	bb1.x = MINN(floor(x + grad), r0.w-1);
	bb1.y = MINN(floor(y + grad), r0.h-1);*/
bb0 = xyi(0, 0);
bb1.x = MINN(r0.w-1, r0.w-1);
bb1.y = MINN(r0.h-1, r0.h-1);

	bb0.x >>= r0.sector_size;
	bb0.y >>= r0.sector_size;
	bb1.x >>= r0.sector_size;
	bb1.y >>= r0.sector_size;

	// store the drawing parameters in the main drawing queue
	df = di = drawq_add_to_main_queue(r0, DQT_BLIT_PHOTO);
	di[0] = r1.clbuf_da;
	di[1] = r1.clbuf_da >> 32;
	di[2] = r1.w;
	di[3] = r1.h;
	df[4] = 1./pscale.x;
	df[5] = 1./pscale.y;
	df[6] = -pos.x;
	df[7] = -pos.y;
	df[8] = pc.x;
	df[9] = pc.y;
	df[10] = distortion;
	df[11] = 1. / (MINN(r1.w, r1.h)*0.5);
	df[12] = gain;

	// go through the affected sectors
	for (iy=bb0.y; iy<=bb1.y; iy++)
		for (ix=bb0.x; ix<=bb1.x; ix++)
			drawq_add_sector_id(r0, iy*r0.sector_w + ix);	// add sector reference
#endif
}

void blit_photo_in_rect(raster_t r0, raster_t r1, rect_t r, int keep_aspect_ratio, int interp, xy_t pc, double distortion, double gain)
{
	xy_t pscale, pos;

	pscale = div_xy(get_rect_dim(r), xy(r1.w, r1.h));
	pos = add_xy(rect_p01(r), mul_xy(pscale, set_xy(0.5)));

	blit_scale_photo_cl(r0, r1, pscale, pos, interp, pc, distortion, gain);
}
