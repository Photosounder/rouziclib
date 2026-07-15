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

size_t get_raster_mode_elem_size(const int mode)
{
	switch (mode)
	{
		case IMAGE_USE_SRGB:	return sizeof(srgb_t);
		case IMAGE_USE_LRGB:	return sizeof(lrgb_t);
		case IMAGE_USE_FRGB:	return sizeof(frgb_t);
		case IMAGE_USE_SQRGB:	return sizeof(sqrgb_t);
		case IMAGE_USE_BUF:	return 1;
		default:		return 0;
	}
}

raster_t make_raster(void *data, const xyi_t dim, xyi_t maxdim, const int mode)	// maxdim can be XYI0
{
	raster_t r={0};
	void **ptr;

	if (is0_xyi(maxdim))
		maxdim = dim;

	r.dim = dim;
	r.table_index = -1;
	r.as = (size_t) maxdim.x * (size_t) maxdim.y;
	ptr = get_raster_buffer_for_mode_ptr(&r, mode);

	if (data)
		*ptr = data;
	else if (r.as && ptr)
		*ptr = calloc(r.as, get_raster_mode_elem_size(mode));

	return r;
}

raster_t make_raster_empty()
{
	raster_t r={0};
	r.table_index = -1;
	return r;
}

raster_t copy_raster(raster_t r0)
{
	raster_t r1;

	r1 = r0;

	r1.l = copy_alloc(r0.l, r0.dim.x*r0.dim.y*sizeof(lrgb_t));
	r1.f = copy_alloc(r0.f, r0.dim.x*r0.dim.y *sizeof(frgb_t));
	r1.srgb = copy_alloc(r0.srgb, r0.dim.x*r0.dim.y *sizeof(srgb_t));
	r1.sq = copy_alloc(r0.sq, r0.dim.x*r0.dim.y *sizeof(sqrgb_t));
	r1.buf = copy_alloc(r0.buf, r0.buf_size);
	r1.buf_size = r0.buf_size;

	return r1;
}

void blank_raster(raster_t *r, const int set_alpha_to_1)
{
	size_t i, pix_count = mul_x_by_y_xyi(r->dim);

	if (r->l)	memset(r->l, 0, pix_count*sizeof(lrgb_t));
	if (r->f)	memset(r->f, 0, pix_count*sizeof(frgb_t));
	if (r->srgb)	memset(r->srgb, 0, pix_count*sizeof(srgb_t));
	if (r->sq)	memset(r->sq, 0, pix_count*sizeof(sqrgb_t));
	if (r->buf)	memset(r->sq, 0, r->buf_size);

	if (set_alpha_to_1)
	{
		if (r->l)
			for (i=0; i < pix_count; i++)
				r->l[i].a = ONE;

		if (r->f)
			for (i=0; i < pix_count; i++)
				r->f[i].a = 1.f;

		if (r->srgb)
			for (i=0; i < pix_count; i++)
				r->srgb[i].a = 255;
	}
}

void **get_raster_buffer_for_mode_ptr(raster_t *r, const int mode)
{
	switch (mode)
	{
		case IMAGE_USE_SRGB:	return &r->srgb;	break;
		case IMAGE_USE_LRGB:	return &r->l;		break;
		case IMAGE_USE_FRGB:	return &r->f;		break;
		case IMAGE_USE_SQRGB:	return &r->sq;		break;
		case IMAGE_USE_BUF:	return &r->buf;		break;
	}

	return NULL;
}

void *get_raster_buffer_for_mode(raster_t r, const int mode)
{
	if (mode & IMAGE_USE_SRGB)	return r.srgb;
	if (mode & IMAGE_USE_LRGB)	return r.l;
	if (mode & IMAGE_USE_FRGB)	return r.f;
	if (mode & IMAGE_USE_SQRGB)	return r.sq;
	if (mode & IMAGE_USE_BUF)	return r.buf;

	return NULL;
}

void **get_raster_buffer_ptr(raster_t *r)
{
	if (r->srgb)	return &r->srgb;
	if (r->l)	return &r->l;
	if (r->f)	return &r->f;
	if (r->sq)	return &r->sq;
	if (r->buf)	return &r->buf;

	return NULL;
}

void *get_raster_buffer(raster_t *r)
{
	void **ptr;

	ptr = get_raster_buffer_ptr(r);

	if (ptr)
		return *ptr;
	return NULL;
}

int get_raster_mode(raster_t r)
{
	if (r.srgb)	return IMAGE_USE_SRGB;
	if (r.l)	return IMAGE_USE_LRGB;
	if (r.f)	return IMAGE_USE_FRGB;
	if (r.sq)	return IMAGE_USE_SQRGB;
	if (r.buf)	return IMAGE_USE_BUF;

	return -1;
}

size_t get_raster_buffer_size(raster_t r)
{
	size_t pix_count = mul_x_by_y_xyi(r.dim);

	if (r.srgb)	return pix_count*sizeof(srgb_t);
	if (r.l)	return pix_count*sizeof(lrgb_t);
	if (r.f)	return pix_count*sizeof(frgb_t);
	if (r.sq)	return pix_count*sizeof(sqrgb_t);
	if (r.buf)	return r.buf_size;

	return 0;
}


srgb_t get_raster_pixel_in_srgb(raster_t r, const size_t index)
{
	srgb_t s={0};
	frgb_t f;
	lrgb_t l;
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
			f = sqrgb_to_frgb(r.sq[index]);

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

	return s;
}

frgb_t get_raster_pixel_in_frgb(raster_t r, const size_t index)
{
	if (r.f)
		return r.f[index];

	if (r.l)
		return lrgb_to_frgb(r.l[index]);

	if (r.sq)
		return sqrgb_to_frgb(r.sq[index]);

	if (r.srgb)
		return srgb_to_frgb(r.srgb[index]);

	return make_colour_frgb(NAN, NAN, NAN, NAN);
}

frgb_t get_raster_pixel_in_frgb_xyi(raster_t r, xyi_t p)
{
	frgb_t pv={0};

	if (p.y >= 0 && p.y < r.dim.y && p.x >= 0 && p.x < r.dim.x)
		pv = get_raster_pixel_in_frgb(r, p.y * r.dim.x + p.x);

	return pv;
}

void free_raster(raster_t *r)
{
	void **ptr;

	// Release framebuffer-owned direct bracket layers before their base raster
	if (fb && r == &fb->r)
		drawq_bracket_deinit_direct();

	ptr = get_raster_buffer_ptr(r);

	while (ptr)	// free every possible buffer
	{
		#ifndef RL_ONLY_DRAW_LRGB
		cl_data_table_remove_entry_by_host_ptr(*ptr);	// remove reference from cl data table
		#endif

		free_null(ptr);

		ptr = get_raster_buffer_ptr(r);
	}

	memset(r, 0, sizeof(raster_t));
}

static lrgb_t drawq_bracket_blend_lrgb(lrgb_t bg, lrgb_t fg, enum dq_blend blending_mode)
{
	uint16_t *b = (uint16_t *) &bg;
	uint16_t *f = (uint16_t *) &fg;
	uint32_t v;
	int i;

	// Apply the requested bracket operation to every colour and alpha channel
	for (i=0; i < 4; i++)
	{
		switch (blending_mode)
		{
			case DQB_ADD:
				v = (uint32_t) b[i] + f[i];
				b[i] = MINN(v, ONE);
				break;

			case DQB_SUB:
				b[i] = b[i] > f[i] ? b[i] - f[i] : 0;
				break;

			case DQB_MUL:
				b[i] = ((uint32_t) b[i] * f[i] + (ONE >> 1)) >> LBD;
				break;

			case DQB_DIV:
				if (f[i])
					b[i] = MINN(((uint32_t) b[i] << LBD) / f[i], ONE);
				else
					b[i] = b[i] ? ONE : 0;
				break;

			case DQB_BLEND:
				v = (uint32_t) f[i] * fg.a + (uint32_t) b[i] * (ONE - fg.a);
				b[i] = (v + (ONE >> 1)) >> LBD;
				break;

			case DQB_SOLID:
				b[i] = f[i];
				break;
		}
	}

	return bg;
}

static frgb_t drawq_bracket_blend_frgb(frgb_t bg, frgb_t fg, enum dq_blend blending_mode)
{
	float *b = (float *) &bg;
	float *f = (float *) &fg;
	int i;

	// Apply the requested bracket operation to every colour and alpha channel
	for (i=0; i < 4; i++)
	{
		switch (blending_mode)
		{
			case DQB_ADD:		b[i] += f[i];					break;
			case DQB_SUB:		b[i] -= f[i];					break;
			case DQB_MUL:		b[i] *= f[i];					break;
			case DQB_DIV:		b[i] /= f[i];					break;
			case DQB_BLEND:	b[i] = f[i] * fg.a + b[i] * (1.f - fg.a);	break;
			case DQB_SOLID:	b[i] = f[i];					break;
		}
	}

	return bg;
}

static void drawq_bracket_open_direct()
{
	raster_t *layer;
	void **active_ptr;
	void *layer_buffer;
	size_t pixel_count;
	int level, mode;

	if (fb->discard)
		return;

	// Suppress overflowing nested brackets without closing a valid parent level
	if (fb->drawq_direct_bracket_overflow || fb->drawq_direct_bracket_depth >= DRAWQ_DIRECT_BRACKET_LEVELS)
	{
		if (fb->drawq_direct_bracket_overflow++ == 0)
			fprintf_rl(stderr, "Direct bracket nesting exceeds %d levels\n", DRAWQ_DIRECT_BRACKET_LEVELS);
		return;
	}

	// Select the direct framebuffer buffer independently of transient SDL texture memory
	mode = fb->r.use_frgb ? IMAGE_USE_FRGB : IMAGE_USE_LRGB;
	active_ptr = get_raster_buffer_for_mode_ptr(&fb->r, mode);
	if (active_ptr==NULL || *active_ptr==NULL)
	{
		fprintf_rl(stderr, "Direct bracket cannot open without a framebuffer\n");
		fb->drawq_direct_bracket_overflow++;
		return;
	}

	// Lazily allocate a reusable zeroed layer for this nesting level
	level = fb->drawq_direct_bracket_depth;
	layer = &fb->drawq_direct_bracket_layer[level];
	pixel_count = mul_x_by_y_xyi(fb->r.dim);
	if (get_raster_mode(*layer) != mode || layer->as < pixel_count)
	{
		free_raster(layer);
		*layer = make_raster(NULL, fb->r.dim, fb->r.dim, mode);
	}
	else
		layer->dim = fb->r.dim;

	// Keep the matching close harmless if layer allocation failed
	layer_buffer = get_raster_buffer_for_mode(*layer, mode);
	if (layer_buffer==NULL)
	{
		fprintf_rl(stderr, "Direct bracket layer allocation failed for %zu pixels\n", pixel_count);
		fb->drawq_direct_bracket_overflow++;
		return;
	}

	// Save the parent and redirect every immediate drawing primitive to the child layer
	fb->drawq_direct_bracket_parent[level] = *active_ptr;
	*active_ptr = layer_buffer;
	fb->drawq_direct_bracket_depth++;
}

static void drawq_bracket_close_direct(enum dq_blend blending_mode)
{
	void **active_ptr;
	void *child, *parent;
	size_t i, pixel_count;
	int level, mode;

	if (fb->discard)
		return;

	// Consume a close belonging to an open that was suppressed after overflow
	if (fb->drawq_direct_bracket_overflow)
	{
		fb->drawq_direct_bracket_overflow--;
		return;
	}

	// Reject unmatched closes without changing the active framebuffer
	if (fb->drawq_direct_bracket_depth <= 0)
	{
		fprintf_rl(stderr, "Direct bracket close has no matching open\n");
		return;
	}

	// Recover invalid blend values as additive brackets
	if (blending_mode < DQB_ADD || blending_mode > DQB_SOLID)
	{
		fprintf_rl(stderr, "Invalid direct bracket blend mode %d, using DQB_ADD\n", blending_mode);
		blending_mode = DQB_ADD;
	}

	// Restore the parent before compositing the completed child layer
	mode = fb->r.use_frgb ? IMAGE_USE_FRGB : IMAGE_USE_LRGB;
	active_ptr = get_raster_buffer_for_mode_ptr(&fb->r, mode);
	level = --fb->drawq_direct_bracket_depth;
	child = *active_ptr;
	parent = fb->drawq_direct_bracket_parent[level];
	*active_ptr = parent;
	fb->drawq_direct_bracket_parent[level] = NULL;
	pixel_count = mul_x_by_y_xyi(fb->r.dim);

	// Composite and clear in one pass so the retained layer is ready for reuse
	if (mode==IMAGE_USE_FRGB)
	{
		frgb_t *parent_f = parent;
		frgb_t *child_f = child;

		for (i=0; i < pixel_count; i++)
		{
			parent_f[i] = drawq_bracket_blend_frgb(parent_f[i], child_f[i], blending_mode);
			child_f[i] = (frgb_t) {0};
		}
	}
	else
	{
		lrgb_t *parent_l = parent;
		lrgb_t *child_l = child;

		for (i=0; i < pixel_count; i++)
		{
			parent_l[i] = drawq_bracket_blend_lrgb(parent_l[i], child_l[i], blending_mode);
			child_l[i] = (lrgb_t) {0};
		}
	}
}

void drawq_bracket_deinit_direct()
{
	void **active_ptr;
	int i, mode;

	if (fb==NULL)
		return;

	// Restore the base framebuffer if teardown catches unmatched bracket opens
	mode = fb->r.use_frgb ? IMAGE_USE_FRGB : IMAGE_USE_LRGB;
	active_ptr = get_raster_buffer_for_mode_ptr(&fb->r, mode);
	if (fb->drawq_direct_bracket_depth || fb->drawq_direct_bracket_overflow)
		fprintf_rl(stderr, "Discarding %d open and %d suppressed brackets while resetting direct bracket state\n", fb->drawq_direct_bracket_depth, fb->drawq_direct_bracket_overflow);
	while (fb->drawq_direct_bracket_depth > 0)
	{
		fb->drawq_direct_bracket_depth--;
		*active_ptr = fb->drawq_direct_bracket_parent[fb->drawq_direct_bracket_depth];
		fb->drawq_direct_bracket_parent[fb->drawq_direct_bracket_depth] = NULL;
	}
	fb->drawq_direct_bracket_overflow = 0;

	// Release every lazily allocated child layer
	for (i=0; i < DRAWQ_DIRECT_BRACKET_LEVELS; i++)
		free_raster(&fb->drawq_direct_bracket_layer[i]);
}

void drawq_bracket_open()
{
	// Execute direct brackets immediately on their reusable raster stack
	if (fb->use_drawq==0)
	{
		drawq_bracket_open_direct();
		return;
	}

	#ifndef RL_FREESTANDING
	// Route queued brackets to the selected queue implementation
	if (fb->use_dqnq)
		drawq_bracket_open_dqnq();
	else
		drawq_bracket_open_dq();
	#endif
}

void drawq_bracket_close(enum dq_blend blending_mode)
{
	// Execute direct bracket compositing immediately
	if (fb->use_drawq==0)
	{
		drawq_bracket_close_direct(blending_mode);
		return;
	}

	#ifndef RL_FREESTANDING
	// Route queued brackets to the selected queue implementation
	if (fb->use_dqnq)
		drawq_bracket_close_dqnq(blending_mode);
	else
		drawq_bracket_close_dq(blending_mode);
	#endif
}

void cl_unref_raster(raster_t *r)
{
	void **ptr;

	ptr = get_raster_buffer_ptr(r);

	if (ptr == NULL)
		return;

	cl_data_table_remove_entry_by_host_ptr(*ptr);	// remove reference from cl data table
}

framebuffer_t *init_framebuffer(xyi_t dim, xyi_t maxdim, const int mode)
{
	framebuffer_t *fb = calloc(1, sizeof(framebuffer_t));

	if (is0_xyi(maxdim))
		maxdim = dim;

	fb->r = make_raster(NULL, dim, maxdim, mode);
	fb->w = fb->r.dim.x;
	fb->h = fb->r.dim.y;
	fb->maxdim = maxdim;

	return fb;
}

void init_tls_fb(xyi_t dim)	// initalisation of thread-local fb and zc in fRGB mode, used for video generation. Just free_raster(&fb->r); and free_null(&fb); at the end
{
	fb = init_framebuffer(dim, XYI0, IMAGE_USE_FRGB);
	fb->r.use_frgb = 1;
	fb->use_drawq = 0;

	zc = init_zoom(&mouse, drawing_thickness);
	calc_screen_limits(&zc);
}

void enlarge_framebuffer(xyi_t newdim)
{
	if (fb->use_drawq==0)
		alloc_enough(get_raster_buffer_ptr(&fb->r), mul_x_by_y_xyi(newdim), &fb->r.as, get_raster_mode_elem_size(get_raster_mode(fb->r)), 1.2);

	fb->r.dim = newdim;
	fb->w = fb->r.dim.x;
	fb->h = fb->r.dim.y;
}

double intensity_scaling(double scale, double scale_limit)	// gives an intensity ratio that decreases if the scale of the thing to be drawn is below a scale threshold
{
	double ratio = 1., x;
	const double knee_width = 0.25, y_offset = sqrt(1.+knee_width) - 1.;

	// Linear adjust knee smoothing
	x = scale / scale_limit;
	ratio = sqrt(sq(x-1.)+knee_width) - x - 1. - y_offset;
	ratio *= -0.5;
	ratio = MINN(ratio, 1.);

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

void screen_blank()
{
	if (fb->use_drawq)
		return ;
	else if (fb->r.use_frgb)
		memset(fb->r.f, 0, fb->w*fb->h*sizeof(frgb_t));
	else
		memset(fb->r.l, 0, fb->w*fb->h*sizeof(lrgb_t));
}
