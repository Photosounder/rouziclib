double gaussrad(double intensity, double radius)
{
	static double last_intensity=0., last_radius=0., last_result=0.;

	if (last_intensity==intensity && last_radius==radius)
		return last_result;
	else
	{
		last_intensity = intensity;
		last_radius = radius;
		if (intensity > GAUSSLIMIT)
			last_result = (sqrt(log(intensity / GAUSSLIMIT))*radius);
		else
			last_result = 0.;
		return last_result;
	}
}

raster_t make_raster_l(lrgb_t *l, int32_t w, int32_t h)
{
	raster_t r;

	memset(&r, 0, sizeof(raster_t));

	r.w = w;
	r.h = h;
	r.use_frgb = 0;

	if (l)
		r.l = l;
	else
		if (w*h)
			r.l = calloc(w*h, sizeof(lrgb_t));
		else
			r.l = NULL;

	return r;
}

raster_t make_raster_f(frgb_t *f, int32_t w, int32_t h)
{
	raster_t r;

	memset(&r, 0, sizeof(raster_t));

	r.w = w;
	r.h = h;
	r.use_frgb = 1;

	if (f)
		r.f = f;
	else
		if (w*h)
			r.f = calloc(w*h, sizeof(frgb_t));
		else
			r.f = NULL;

	return r;
}

void free_raster(raster_t *r)
{
	free (r->l);
	free (r->f);
	free (r->srgb);

	#ifdef RL_OPENCL
	clReleaseMemObject(r->clbuf);
	#endif

	memset(r, 0, sizeof(raster_t));
}

double intensity_scaling(double scale, double scale_limit)	// gives an intensity ratio that decreases if the scale of the thing to be drawn is below a scale threshold
{
	double ratio = 1.;

	if (scale < scale_limit)
		ratio = scale / scale_limit;

	return ratio;
}

void thickness_limit(double *thickness, double *brightness, double limit)	// same except also limits thickness
{
	if (*thickness < limit)
	{
		*brightness *= *thickness / limit;
		*thickness = limit;
	}
}

void screen_blank(raster_t fb)
{
	if (fb.use_cl)
		return ;//zero_cl_mem(&fb.clctx, fb.clbuf, fb.w*fb.h*sizeof(frgb_t));
	else if (fb.use_frgb)
		memset (fb.f, 0, fb.w*fb.h*sizeof(frgb_t));
	else
		memset (fb.l, 0, fb.w*fb.h*sizeof(lrgb_t));
}
