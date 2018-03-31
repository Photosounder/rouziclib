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

raster_t make_raster_srgb(srgb_t *srgb, int32_t w, int32_t h)
{
	raster_t r={0};

	r.dim = xyi(w, h);

	if (srgb)
		r.srgb = srgb;
	else
		if (w*h)
			r.srgb = calloc(w*h, sizeof(srgb_t));
		else
			r.srgb = NULL;

	return r;
}

raster_t make_raster_l(lrgb_t *l, int32_t w, int32_t h)
{
	raster_t r={0};

	r.dim = xyi(w, h);

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
	raster_t r={0};

	r.dim = xyi(w, h);
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

raster_t make_raster_sq(sqrgb_t *sq, int32_t w, int32_t h)
{
	raster_t r={0};

	r.dim = xyi(w, h);

	if (sq)
		r.sq = sq;
	else
		if (w*h)
			r.sq = calloc(w*h, sizeof(sqrgb_t));
		else
			r.sq = NULL;

	return r;
}

raster_t make_raster(void *data, int32_t w, int32_t h, const int mode)	// mode is IMAGE_USE_xRGB (S, L, F, SQ)
{
	if (mode & IMAGE_USE_SRGB)
		return make_raster_srgb(data, w, h);
	if (mode & IMAGE_USE_LRGB)
		return make_raster_l(data, w, h);
	if (mode & IMAGE_USE_FRGB)
		return make_raster_f(data, w, h);
	else //if (mode & IMAGE_USE_SQRGB)
		return make_raster_sq(data, w, h);
}

raster_t copy_raster(raster_t r0)
{
	raster_t r1;

	r1 = r0;

	r1.l = copy_alloc(r0.l, r0.dim.x*r0.dim.y*sizeof(lrgb_t));
	r1.f = copy_alloc(r0.f, r0.dim.x*r0.dim.y *sizeof(frgb_t));
	r1.srgb = copy_alloc(r0.srgb, r0.dim.x*r0.dim.y *sizeof(srgb_t));
	r1.sq = copy_alloc(r0.sq, r0.dim.x*r0.dim.y *sizeof(sqrgb_t));

	return r1;
}

void *get_raster_buffer_for_mode(raster_t r, const int mode)	// mode is IMAGE_USE_xRGB (S, L, F)
{
	if (mode & IMAGE_USE_SRGB)	return r.srgb;
	if (mode & IMAGE_USE_LRGB)	return r.l;
	if (mode & IMAGE_USE_FRGB)	return r.f;
	if (mode & IMAGE_USE_SQRGB)	return r.sq;

	return NULL;
}

srgb_t get_raster_pixel_in_srgb(raster_t r, const int index)
{
	srgb_t s={0};
	frgb_t f;
	lrgb_t l;
	sqrgb_t sq;
	const float mul_rb = 1.f / (1023.f*1023.f);
	const float mul_g = 1.f / (4092.f*4092.f);
	static int init=1;
	static lut_t lsrgb_l, lsrgb_fl_l;

	if (r.srgb)
		return r.srgb[index];

	if (init)
	{
		init = 0;
		lsrgb_l = get_lut_lsrgb();
		lsrgb_fl_l = get_lut_lsrgb_fl();
	}

	if (r.f || r.sq)
	{
		if (r.f)
			f = clamp_frgba(r.f[index]);
		else
		{
			sq = r.sq[index];
			f.r = (float) (sq.r*sq.r) * mul_rb;
			f.g = (float) (sq.g*sq.g) * mul_g;
			f.b = (float) (sq.b*sq.b) * mul_rb;
			f.a = 1.f;
		}

		s.r = lsrgb_fl(f.r, lsrgb_fl_l.lutint) + 16 >> 5;
		s.g = lsrgb_fl(f.g, lsrgb_fl_l.lutint) + 16 >> 5;
		s.b = lsrgb_fl(f.b, lsrgb_fl_l.lutint) + 16 >> 5;
		s.a = lsrgb_fl(f.a, lsrgb_fl_l.lutint) + 16 >> 5;

		return s;
	}

	if (r.l)
	{
		l = r.l[index];
		s.r = lsrgb_l.lutint[l.r] + 16 >> 5;
		s.g = lsrgb_l.lutint[l.g] + 16 >> 5;
		s.b = lsrgb_l.lutint[l.b] + 16 >> 5;
		s.a = lsrgb_l.lutint[l.a] + 16 >> 5;

		return s;
	}
}

void free_raster(raster_t *r)
{
	/*#ifdef RL_TINYCTHREAD
	if (r->mutex)
		mtx_lock(r->mutex);
	#endif*/

	#ifdef RL_OPENCL
	cl_data_table_remove_entry_by_host_ptr(r->referencing_fb, r->f);	// remove reference from cl data table
	cl_data_table_remove_entry_by_host_ptr(r->referencing_fb, r->sq);	// remove reference from cl data table
	#endif

	free_null(&r->l);
	free_null(&r->f);
	free_null(&r->srgb);
	free_null(&r->sq);

	/*#ifdef RL_TINYCTHREAD
	if (r->mutex)
	{
		mtx_unlock(r->mutex);
		//mtx_destroy_free(&r->mutex);
	}
	#endif*/

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

void screen_blank(framebuffer_t fb)
{
	if (fb.use_cl)
		return ;//zero_cl_mem(&fb.clctx, fb.clbuf, fb.w*fb.h*sizeof(frgb_t));
	else if (fb.r.use_frgb)
		memset (fb.r.f, 0, fb.w*fb.h*sizeof(frgb_t));
	else
		memset (fb.r.l, 0, fb.w*fb.h*sizeof(lrgb_t));
}
