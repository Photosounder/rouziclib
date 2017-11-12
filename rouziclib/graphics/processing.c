void gaussian_blur(float *a, float *b, xyi_t dim, const int channels, double radius)
{
	int i, j, ic, ix, iy, jx, jy, in_place=0;
	double *gk, sum;
	int gks, start, stop;
	float *column;
	int w, h;
	const int edge_norm = 1;	// FIXME

	w = dim.x;
	h = dim.y;

	if (a==NULL || b==NULL)
	{
		fprintf_rl(stderr, "gaussian_blur(0x%08x, 0x%08x, ... ) can't use NULL buffers.\n", a, b);
		return ;
	}

	if (a==b)
	{
		in_place = 1;
		b = calloc(w*h*channels, sizeof(float));
	}
	else
		memset(b, 0, w*h*channels*sizeof(float));

	gks = floor(GAUSSRAD_HQ * radius) + 1.;
	gk = calloc (gks, sizeof(double));

	for (i=0; i<gks; i++)
	{
		gk[i] = gaussian((double) i / radius);
		//gk[i] = gaussian((double) i / radius) / radius / sqrt(pi);
		//gk[i] = sinc((double) i, 0.6/4.) * blackman((double) i, gks);
		//fprintf_rl(stdout, "gk[%d] = %g\n", i, gk[i]);
	}

	// calculate proper kernel gain
	sum = 0.;
	for (i=-gks+1; i<gks; i++)
		sum += gk[abs(i)];
	sum = 1./sum;

	for (i=0; i<gks; i++)
		gk[i] *= sum;

	
	for (iy=0; iy<h; iy++)		// horizontal blurring
	{
		for (ix=0; ix<w; ix++)
		{
			start = MAXN (0, ix-(gks-1));
			stop = MINN (w-1, ix+(gks-1));

			for (jx=start; jx<=stop; jx++)
			{
				i = (iy*w + ix) * channels;
				j = (iy*w + jx) * channels;

				for (ic=0; ic < channels; ic++)
					b[i + ic] += gk[abs(jx-ix)] * a[j + ic];
			}
		}
	}

	column = calloc (h*channels, sizeof(float));

	for (ix=0; ix<w; ix++)
	{
		for (iy=0; iy<h; iy++)
		{
			// copy pixel from b into the column buffer
			memcpy(&column[iy*channels], &b[(iy*w + ix)*channels], channels*sizeof(float));
			memset(&b[(iy*w + ix)*channels], 0, channels*sizeof(float));
		}

		for (iy=0; iy<h; iy++)		// vertical blurring
		{
			start = MAXN (0, iy-(gks-1));
			stop = MINN (h-1, iy+(gks-1));
			i = (iy*w + ix) * channels;

			for (jy=start; jy<=stop; jy++)
				for (ic=0; ic < channels; ic++)
					b[i + ic] += gk[abs(jy-iy)] * column[(jy*channels)+ic];
		}
	}

	if (edge_norm)
	{
		for (ix=0; ix<w; ix++)
		{
			start = ix-(gks-1);
			stop = ix+(gks-1);

			if (start < 0 || stop > w-1)			// if we're on the edges
			{
				start = MAXN(0, start);
				stop = MINN(w-1, stop);

				sum = 0.;
				for (jx=start; jx<=stop; jx++)
					sum += gk[abs(jx-ix)];
				sum = 1./sum;

				for (iy=0; iy<h; iy++)
					for (ic=0; ic < channels; ic++)
						b[(iy*w + ix) * channels + ic] *= sum;
			}
		}

		for (iy=0; iy<h; iy++)
		{
			start = iy-(gks-1);
			stop = iy+(gks-1);

			if (start < 0 || stop > h-1)			// if we're on the edges
			{
				start = MAXN(0, start);
				stop = MINN(h-1, stop);

				sum = 0.;
				for (jy=start; jy<=stop; jy++)
					sum += gk[abs(jy-iy)];
				sum = 1./sum;

				for (ix=0; ix<w; ix++)
					for (ic=0; ic < channels; ic++)
						b[(iy*w + ix) * channels + ic] *= sum;
			}
		}
	}

	free (column);
	free (gk);

	if (in_place)
	{
		memcpy(a, b, w*h*channels*sizeof(float));
		free (b);
	}
}

void image_downscale_fast_box(raster_t r0, raster_t *r1, const int ratio, const int mode)
{
	xyi_t dim0, dim1, ip0, ip1;
	recti_t pixbox;
	xyz_t fsum, pix0;
	double fratio;

	if (get_raster_buffer_for_mode(r0, mode)==NULL)
	{
		fprintf_rl(stderr, "image_downscale_fast_box() can't process empty image.\n");
		return ;
	}

	dim0 = xyi(r0.w, r0.h);
	dim1 = div_xyi( add_xyi(dim0, set_xyi(ratio)) , set_xyi(ratio) );

	// if r1 is inadequate
	if (equal_xyi(dim1, xyi(r1->w, r1->h))==0 || get_raster_buffer_for_mode(*r1, mode)==NULL)
	{
		free_raster(r1);
		*r1 = make_raster(NULL, dim1.x, dim1.y, mode);
	}

	if (mode & IMAGE_USE_FRGB)
	{
		for (ip1.y=0; ip1.y < dim1.y; ip1.y++)
		{
			for (ip1.x=0; ip1.x < dim1.x; ip1.x++)
			{
				memset(&fsum, 0, sizeof(fsum));
				fratio = 0.;

				pixbox.p0 = mul_xyi(ip1, set_xyi(ratio));
				pixbox.p1 = min_xyi( add_xyi(pixbox.p0, set_xyi(ratio)) , dim0 );

				for (ip0.y = pixbox.p0.y; ip0.y < pixbox.p1.y; ip0.y++)
				{
					for (ip0.x = pixbox.p0.x; ip0.x < pixbox.p1.x; ip0.x++)
					{
						pix0 = frgb_to_xyz(r0.f[ip0.y * r0.w + ip0.x]);

						fsum = add_xyz(fsum, pix0);
						fratio += 1.;
					}
				}

				fratio = 1./fratio;
				fsum = mul_xyz(fsum, set_xyz(1. / ratio));		// weighting of the sum with a fixed ratio
				r1->f[ip1.y * r1->w + ip1.x] = make_colour_frgb(fsum.x, fsum.y, fsum.z, ratio * ratio * fratio);
			}
		}
	}

	if (mode & IMAGE_USE_LRGB)	// TODO
	{
	}
}

// Functions that process a whole image using a pointer to a per-pixel processing function
void image_pixel_process_arg0(raster_t r, const int mode, void (*func)(void))
{
	int i;

	if (get_raster_buffer_for_mode(r, mode)==NULL)
	{
		fprintf_rl(stderr, "image_pixel_process_arg0() can't process empty image.\n");
		return ;
	}

	if (mode & IMAGE_USE_FRGB)
		for (i=0; i < r.w*r.h; i++)
			((void (*)(float *, float *))func)(&r.f[i], &r.f[i]);

	if (mode & IMAGE_USE_LRGB)
		for (i=0; i < r.w*r.h; i++)
			((void (*)(uint16_t *, uint16_t *))func)(&r.f[i], &r.f[i]);
}

void image_pixel_process_arg1f(raster_t r, const int mode, void (*func)(void), float arg1)
{
	int i, arg1_int;

	if (get_raster_buffer_for_mode(r, mode)==NULL)
	{
		fprintf_rl(stderr, "image_pixel_process_arg1d() can't process empty image.\n");
		return ;
	}

	if (mode & IMAGE_USE_FRGB)
		for (i=0; i < r.w*r.h; i++)
			((void (*)(float *, float *, float))func)(&r.f[i], &r.f[i], arg1);

	arg1_int = nearbyint(arg1 * ONE);
	if (mode & IMAGE_USE_LRGB)
		for (i=0; i < r.w*r.h; i++)
			((void (*)(uint16_t *, uint16_t *, int))func)(&r.f[i], &r.f[i], arg1_int);
}

// Per-pixel processing functions
void pixel_invert_linear(float *p0, float *p1)
{
	int ic;

	for (ic=0; ic<3; ic++)
		p1[ic] = 1. - p0[ic];

	p1[3] = p0[3];
}

void pixel_alpha_to_grey(float *p0, float *p1)
{
	int ic;

	for (ic=0; ic<4; ic++)
		p1[ic] = p0[3];
}

void pixel_mul_by_own_alpha(float *p0, float *p1)
{
	int ic;

	for (ic=0; ic<3; ic++)
		p1[ic] = p0[ic] * p0[3];

	p1[3] = p0[3];
}

void pixel_mul_by_ratio(float *p0, float *p1, float ratio)
{
	int ic;

	for (ic=0; ic<3; ic++)
		p1[ic] = p0[ic] * ratio;

	p1[3] = p0[3];
}
