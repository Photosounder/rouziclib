void dqsb_draw_line_thin_add(float *le, float *block, xy_t start_pos, const int bs, int chan_stride)	// SSE2
{
	// Generic variables
	xyi_t ip;
	float y, x;
	__m128 yv, xv, *bp;
	int ic, ib=0;

	// Specific variables
	__m128 r1x, r1y, r2x, costh, sinth, col[4];
	__m128 rpx, rpy, ycos, ysin, weight;
	__m128 d1y, d1x, d2x;
	const float gl = 2.96;	// gaussian drawing limit

	r1x = _mm_set_ps1(le[0]);
	r1y = _mm_set_ps1(le[1]);
	r2x = _mm_set_ps1(le[2]);
	costh = _mm_set_ps1(le[3]);
	sinth = _mm_set_ps1(le[4]);
	col[0] = _mm_set_ps1(le[5]);
	col[1] = _mm_set_ps1(le[6]);
	col[2] = _mm_set_ps1(le[7]);
	col[3] = _mm_set_ps1(le[8]);

	// Load parameters

	for (y=start_pos.y, ip.y=0; ip.y < bs; ip.y++, y+=1.f)
	{
		yv = _mm_set_ps1(y);
		ycos = _mm_mul_ps(yv, costh);
		ysin = _mm_mul_ps(yv, sinth);

		for (x=start_pos.x, ip.x=0; ip.x < bs; ip.x+=4, x+=4.f, ib+=4)
		{
			// Initialise coordinates
			xv = _mm_add_ps(_mm_set_ps1(x), _mm_set_ps(3.f, 2.f, 1.f, 0.f));

			// Rotate coordinates
			rpx = _mm_mul_ps(xv, costh);	rpy = _mm_mul_ps(xv, sinth);
			rpx = _mm_sub_ps(rpx, ysin);	rpy = _mm_add_ps(rpy, ycos);

			// Distances
			d1y = _mm_abs_ps(_mm_sub_ps(rpy, r1y));
			d2x = _mm_sub_ps(rpx, r2x);
			d1x = _mm_sub_ps(rpx, r1x);

			// Distance checks
			if (_mm_movemask_ps(_mm_cmplt_ps(d1y, _mm_set_ps1(gl))))
			if (_mm_movemask_ps(_mm_cmplt_ps(d2x, _mm_set_ps1(gl))))
			if (_mm_movemask_ps(_mm_cmpgt_ps(d1x, _mm_set_ps1(-gl))))
			{
				// Compute pixel weight
				if (_mm_movemask_ps(_mm_cmple_ps(d2x, _mm_set_ps1(-gl))) & _mm_movemask_ps(_mm_cmpge_ps(d1x, _mm_set_ps1(gl))))	// if we're far from the ends
					weight = _mm_gaussian_d1_ps(d1y);		// do only the Gaussian weight
				else							// otherwise do the ends too
				{
					weight = _mm_sub_ps( _mm_erfr_d1_ps(d1x) , _mm_erfr_d1_ps(d2x) );
					weight = _mm_mul_ps(weight, _mm_gaussian_d1_ps(d1y));
				}

				// Add weighted colour
				for (ic=0; ic < 4; ic++)
				{
					bp = (__m128 *) &block[ic*chan_stride + ib];
					*bp = _mm_add_ps(_mm_mul_ps(weight, col[ic]), *bp);
				}
			}
		}
	}
}

#ifdef __GNUC__
__attribute__((__target__("ssse3")))
#endif
void dqs_block_to_srgb(srgb_t *srgb, float *block, int r_pitch, int srgb_order, xyi_t dim, int ss, int chan_stride) // SSSE3, calls SSE4.1
{
	xyi_t ip;
	int i, ib;
	__m128 f, sf;
	__m128i index, sv, shuf_mask;

	switch (srgb_order)
	{
		case ORDER_RGBA:
			shuf_mask = _mm_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 12, 8, 4, 0);

		case ORDER_ABGR:
			shuf_mask = _mm_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 8, 4, 12);
			break;

		default:
		case ORDER_BGRA:
			shuf_mask = _mm_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 12, 0, 4, 8);
			break;

		case ORDER_ARGB:
			shuf_mask = _mm_set_epi8(-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 8, 4, 0, 12);
			break;
	}

	index = _mm_set_epi32(chan_stride*3, chan_stride*2, chan_stride, 0);

	for (ip.y=0; ip.y < dim.y; ip.y++)
	{
		i = ip.y * r_pitch;
		ib = ip.y << ss;

		for (ip.x=0; ip.x < dim.x; ip.x++, i++, ib++)
		{
			// Load frgb from block channel planes
			f = _mm_set_ps(block[chan_stride*3 + ib], block[chan_stride*2 + ib], block[chan_stride + ib], block[ib]);
			sf = _mm_mul_ps(_mm_frgb_to_srgb(f), _mm_set_ps1(255.f));	// convert and multiply by 255
			sv = _mm_cvtps_epi32(sf);					// cast to 32-bit int (rounded, no dithering)
			sv = _mm_shuffle_epi8(sv, shuf_mask);				// reorder the channels and pack them into the lower 32-bits
			_mm_storeu_si32(&srgb[i], sv);					// save final pixel to framebuffer
			index = _mm_add_epi32(index, _mm_set1_epi32(1));		// increment indexes
		}
	}
}

typedef struct
{
	volatile thread_on;
	rl_thread_t thread_handle;
	rl_mutex_t proc_mtx, cont_mtx;
	int current_locks;
	uint8_t *volatile data;
	int32_t *volatile drawq_data, *volatile sector_pos, *volatile entry_list;
	volatile size_t data_as, drawq_as, sector_list_as, entry_list_as;
	volatile int sector_size, sector_w, r_pitch, srgb_order;
	srgb_t *volatile srgb;
	volatile xyi_t r_dim;
	float **block;
	int thread_id, thread_count;
} drawq_soft_data_t;

#define DQS_THREADS 1
drawq_soft_data_t *dqs_data=NULL;

int drawq_soft_thread(drawq_soft_data_t *d)
{
	xyi_t ip, is, bpos, sec_dim, out_dim;
	float *df;
	int32_t *di;
	int ss, sec_pix, chan_stride;
	int i, eli, qi, sec, entry_count;
	xy_t pos;
	int brlvl;

	rl_mutex_unlock(&d->cont_mtx);		// unlock cont_mtx so that the calling thread can go on
	rl_thread_set_priority_high();

	while (d->thread_on)
	{
		rl_mutex_lock(&d->proc_mtx);

		df = d->drawq_data;
		di = d->drawq_data;
		ss = d->sector_size;
		sec_pix = 1 << ss;		// 1 << 4 = 16
		chan_stride = sec_pix*sec_pix;
		//sec_dim = rshift_xyi(d->r_dim, ss);			// 1920x1080 >> 4 = 120x67
		sec_dim = ceil_rshift_xyi(d->r_dim, ss);		// 1920x1080 >> 4 = 120x68
		brlvl = 0;			// bracket level

		// Go through each sector
		for (is.y=d->thread_id; is.y < sec_dim.y; is.y+=d->thread_count)
			for (is.x=0; is.x < sec_dim.x; is.x++)
			{
				bpos = lshift_xyi(is, ss);
				out_dim = min_xyi(sub_xyi(d->r_dim, bpos), set_xyi(sec_pix));

				sec = is.y * d->sector_w + is.x;	// sector index
				eli = d->sector_pos[sec];		// entry list index

				if (eli < 0)				// if the index is -1 that means there's nothing to do
				{
					// Blank the whole sector
					for (ip.y = bpos.y; ip.y < bpos.y+out_dim.y; ip.y++)
						memset(&d->srgb[ip.y*d->r_pitch + bpos.x], 0, out_dim.x*sizeof(srgb_t));
				}
				else
				{
					memset(d->block[0], 0, chan_stride*4*sizeof(float));

					pos = xyi_to_xy(bpos);
					entry_count = d->entry_list[eli];

					// Go through each entry for this sector
					for (i=0; i < entry_count; i++)
					{
						qi = d->entry_list[eli + i + 1];	// queue index

						switch (di[qi])				// type of the entry
						{
							case DQT_LINE_THIN_ADD:		dqsb_draw_line_thin_add(&df[qi+1], d->block[brlvl], pos, sec_pix, chan_stride);	break;
						}
					}

					dqs_block_to_srgb(&d->srgb[bpos.y*d->r_pitch + bpos.x], d->block[0], d->r_pitch, d->srgb_order, out_dim, ss, chan_stride);
				}
			}

		rl_mutex_unlock(&d->proc_mtx);
		rl_mutex_lock(&d->cont_mtx);
		rl_mutex_unlock(&d->cont_mtx);
	}

	return 0;
}

void drawq_soft_run()
{
	int i, r_pitch;

	// Init once
	if (dqs_data==NULL)
	{
		dqs_data = calloc(DQS_THREADS, sizeof(drawq_soft_data_t));

		for (i=0; i < DQS_THREADS; i++)
		{
			drawq_soft_data_t *d = &dqs_data[i];

			// Init everything permanent
			d->thread_on = 1;
			rl_mutex_init(&d->proc_mtx);
			rl_mutex_init(&d->cont_mtx);
			d->thread_id = i;
			d->thread_count = DQS_THREADS;
			d->block = calloc_2d(4, 1 << 2*fb.sector_size, 4*sizeof(float));	// alloc float blocks once, one block per bracket level

			// Create thread
			rl_mutex_lock(&d->proc_mtx);		// lock proc_mtx so the thread has to wait to proceed
			rl_mutex_lock(&d->cont_mtx);
			rl_thread_create(&d->thread_handle, drawq_soft_thread, d);
			rl_mutex_lock(&d->cont_mtx);		// double lock cont_mtx so we can't go on until the thread unlocks it
			rl_mutex_unlock(&d->cont_mtx);
		}
	}

	// Acquire destination texture
	#ifdef RL_SDL
	fb.tex_lock = 1;
	SDL_LockTexture(fb.texture, NULL, &fb.r.srgb, &r_pitch);
	#endif
	r_pitch /= sizeof(srgb_t);

	for (i=0; i < DQS_THREADS; i++)
	{
		drawq_soft_data_t *d = &dqs_data[i];

		// Realloc and copy data
		alloc_enough_and_copy(&d->data, fb.data, fb.data_cl_as, &d->data_as, 1, 1.);
		alloc_enough_and_copy(&d->drawq_data, fb.drawq_data, fb.drawq_data[DQ_END], &d->drawq_as,  sizeof(int32_t), 1.5);
		alloc_enough_and_copy(&d->sector_pos, fb.sector_pos, fb.sectors, &d->sector_list_as,       sizeof(int32_t), 1.5);
		alloc_enough_and_copy(&d->entry_list, fb.entry_list, fb.entry_list_end, &d->entry_list_as, sizeof(int32_t), 1.5);
		d->sector_size = fb.sector_size;
		d->sector_w = fb.sector_w;

		d->srgb = fb.r.srgb;
		d->r_dim = fb.r.dim;
		d->srgb_order = fb.srgb_order;
		d->r_pitch = r_pitch;

		d->current_locks++;
		rl_mutex_unlock(&d->proc_mtx);	// this makes the thread start processing this frame
		rl_mutex_lock(&d->cont_mtx);	// will make the thread stop after processing is done
	}
}

void drawq_soft_finish()
{
	int i;

	if (dqs_data==NULL)
		return ;

	for (i=0; i < DQS_THREADS; i++)
	{
		drawq_soft_data_t *d = &dqs_data[i];

		if (d)
		if (d->current_locks==1)
		{
			d->current_locks--;
			rl_mutex_lock(&d->proc_mtx);		// will make the thread wait for drawq_soft_run() to add the new frame data
			rl_mutex_unlock(&d->cont_mtx);		// makes the thread complete the loop and wait for proc_mtx
			//rl_thread_join_and_null(&d->thread_handle);
		}
	}
}

void drawq_soft_quit()
{
	int i;

	if (dqs_data==NULL)
		return ;

	for (i=0; i < DQS_THREADS; i++)
	{
		drawq_soft_data_t *d = &dqs_data[i];

		if (d)
		{
			d->thread_on = 0;
			rl_mutex_unlock(&d->cont_mtx);
			rl_thread_join_and_null(&d->thread_handle);
		}
	}
}
