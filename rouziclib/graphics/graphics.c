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

raster_t make_raster(void *data, int32_t w, int32_t h, const int mode)	// mode is IMAGE_USE_xRGB (S, L, F)
{
	if (mode & IMAGE_USE_LRGB)
		return make_raster_l(data, w, h);
	else						// if (mode & IMAGE_USE_FRGB)
		return make_raster_f(data, w, h);
}

raster_t copy_raster(raster_t r0)
{
	raster_t r1;

	r1 = r0;

	r1.l = copy_alloc(r0.l, r0.w*r0.h*sizeof(lrgb_t));
	r1.f = copy_alloc(r0.f, r0.w*r0.h*sizeof(frgb_t));
	r1.srgb = copy_alloc(r0.srgb, r0.w*r0.h*sizeof(srgb_t));

	return r1;
}

void cl_copy_raster_to_device(raster_t fb, raster_t r)
{
#ifdef RL_OPENCL
	cl_int ret = clEnqueueWriteBuffer(fb.clctx.command_queue, fb.data_cl, CL_FALSE, r.clbuf_da, r.w*r.h*4*sizeof(float), r.f, 0, NULL, NULL);
	CL_ERR_NORET("clEnqueueWriteBuffer (in cl_copy_raster_to_device, for fb.data_cl)", ret);
#endif
}

void *get_raster_buffer_for_mode(raster_t r, const int mode)	// mode is IMAGE_USE_xRGB (S, L, F)
{
	if (mode & IMAGE_USE_SRGB)	return r.srgb;
	if (mode & IMAGE_USE_LRGB)	return r.l;
	if (mode & IMAGE_USE_FRGB)	return r.f;

	return NULL;
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
