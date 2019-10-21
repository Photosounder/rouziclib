void alloc_mipmap_level(mipmap_level_t *ml, xyi_t fulldim, xyi_t tilesize, const int mode)
{
	xyi_t it, rdim;
	size_t elem_size = get_raster_mode_elem_size(mode);

	ml->tiledim = tilesize;
	ml->fulldim = fulldim;
	ml->tilecount = div_round_up_xyi(fulldim , tilesize);

	ml->r = calloc(mul_x_by_y_xyi(ml->tilecount), sizeof(raster_t));
	ml->total_bytes = mul_x_by_y_xyi(ml->tilecount) * sizeof(raster_t);

	for (it.y=0; it.y < ml->tilecount.y; it.y++)
		for (it.x=0; it.x < ml->tilecount.x; it.x++)
		{
			rdim = get_dim_of_tile(fulldim, tilesize, it);
			ml->r[it.y*ml->tilecount.x + it.x] = make_raster(NULL, rdim, XYI0, mode);
			ml->total_bytes += mul_x_by_y_xyi(rdim) * elem_size;
		}
}

void *get_tile_pixel_ptr(mipmap_level_t ml, xyi_t pos, const int mode)
{
	xyi_t ti, pt;
	raster_t *tile_ptr;

	if (check_pixel_within_image(pos, ml.fulldim)==0)
		return NULL;				// returns NULL pointer if pixel is out of bounds

	ti = div_xyi(pos, ml.tiledim);			// tile index
	pt = sub_xyi(pos, mul_xyi(ti, ml.tiledim));	// position in tile

	tile_ptr = &ml.r[ti.y * ml.tilecount.x + ti.x];

	if (mode & IMAGE_USE_SQRGB)
		return &tile_ptr->sq[pt.y * tile_ptr->dim.x + pt.x];
	else
		return &tile_ptr->f[pt.y * tile_ptr->dim.x + pt.x];
}

/*frgb_t get_tile_pixel(mipmap_level_t ml, xyi_t pos)
{
	frgb_t pv={0};
	frgb_t *pp;

	pp = get_tile_pixel_ptr(ml, pos);
	if (pp==NULL)
		return pv;
	else
		pv = *pp;

	return pv;
}

void set_tile_pixel(mipmap_level_t ml, xyi_t pos, frgb_t pv)
{
	frgb_t *pp;

	pp = get_tile_pixel_ptr(ml, pos);
	if (pp==NULL)
		return ;
	else
		*pp = pv;
}*/

void copy_from_raster_to_tiles(raster_t r, mipmap_level_t ml, const int mode)
{
	xyi_t it, rdim, tilestart, pstart;
	int iy;
	frgb_t *pp;

	for (it.y=0; it.y < ml.tilecount.y; it.y++)
		for (it.x=0; it.x < ml.tilecount.x; it.x++)	// this copies a rect from the raster into the matching tile, line by line
		{
			tilestart = mul_xyi(ml.tiledim, it);
			rdim = get_dim_of_tile(ml.fulldim, ml.tiledim, it);

			for (iy=0; iy<rdim.y; iy++)
			{
				pstart = add_xyi(tilestart, xyi(0, iy));
				pp = get_tile_pixel_ptr(ml, pstart, mode);

				if (mode & IMAGE_USE_SQRGB)
					memcpy(pp, &r.sq[pstart.y*r.dim.x + pstart.x], rdim.x*sizeof(sqrgb_t));
				else
					memcpy(pp, &r.f[pstart.y*r.dim.x + pstart.x], rdim.x*sizeof(frgb_t));
			}
		}
}

/*void tile_downscale_fast_box(mipmap_level_t ml0, mipmap_level_t ml1, const xyi_t ratio)
{
	xyi_t ip0, ip1;
	recti_t pixbox;
	xyz_t fsum, pix0;
	double fratio;

	for (ip1.y=0; ip1.y < ml1.fulldim.y; ip1.y++)
	{
		for (ip1.x=0; ip1.x < ml1.fulldim.x; ip1.x++)
		{
			memset(&fsum, 0, sizeof(fsum));
			fratio = 0.;

			pixbox.p0 = mul_xyi(ip1, ratio);
			pixbox.p1 = min_xyi( add_xyi(pixbox.p0, ratio) , ml0.fulldim );

			for (ip0.y = pixbox.p0.y; ip0.y < pixbox.p1.y; ip0.y++)
			{
				for (ip0.x = pixbox.p0.x; ip0.x < pixbox.p1.x; ip0.x++)
				{
					pix0 = frgb_to_xyz(get_tile_pixel(ml0, ip0));

					fsum = add_xyz(fsum, pix0);
					fratio += 1.;
				}
			}

			fratio = 1./fratio;
			fsum = mul_xyz(fsum, set_xyz(1. / mul_x_by_y_xyi(ratio)));		// weighting of the sum with a fixed ratio
			set_tile_pixel(ml1, ip1, make_colour_frgb(fsum.x, fsum.y, fsum.z, mul_x_by_y_xyi(ratio) * fratio) );
		}
	}
}*/

void tile_pixel_sum_float(raster_t *tile0, const int i0, raster_t *tile1, const int i1, const int width0, const int partial)
{
	frgb_t sum, *tp0 = &tile0->f[i0];

	switch (partial)
	{
		case 0:	sum = add_frgba( add_frgba(tp0[0], tp0[1]) , add_frgba(tp0[width0], tp0[width0+1]) );	break;
		case 1:	sum = add_frgba(tp0[0], tp0[width0]);							break;
		case 2:	sum = add_frgba(tp0[0], tp0[1]);							break;
		default:
		case 3:	sum = tp0[0];										break;
	}

	tile1->f[i1] = mul_scalar_frgba(sum, 0.25);
}

void tile_pixel_sum_sq(raster_t *tile0, const int i0, raster_t *tile1, const int i1, const int width0, const int partial)
{
	sqrgb_t sum, *tp0 = &tile0->sq[i0];

	switch (partial)
	{
		case 0:	sum = average_sqrgb_4(tp0[0], tp0[1], tp0[width0], tp0[width0+1]);	break;
		case 1:	sum = average_sqrgb_2(tp0[0], tp0[width0]);				break;
		case 2:	sum = average_sqrgb_2(tp0[0], tp0[1]);					break;
		default:
		case 3:	sum = average_sqrgb_1(tp0[0]);						break;
	}

	tile1->sq[i1] = sum;
}


void tile_downscale_box_2x2(mipmap_level_t ml0, mipmap_level_t ml1, const int mode)
{
	xyi_t it0, it1, ip0, ip1, off1, partial;
	raster_t *tile0, *tile1;
	int y0w, y1w;
	void (*tile_pixel_sum_func)(raster_t *, const int, raster_t *, const int, const int, const int);

	if (mode & IMAGE_USE_SQRGB)
		tile_pixel_sum_func = &tile_pixel_sum_sq;
	else
		tile_pixel_sum_func = &tile_pixel_sum_float;

	for (it0.y=0; it0.y < ml0.tilecount.y; it0.y++)
	{
		it1.y = it0.y >> 1;
		off1.y = (it0.y - (it1.y << 1)) * (ml0.tiledim.y >> 1);			// position offset for the tile0s on the bottom side

		for (it0.x=0; it0.x < ml0.tilecount.x; it0.x++)
		{
			it1.x = it0.x >> 1;
			off1.x = (it0.x - (it1.x << 1)) * (ml0.tiledim.x >> 1);		// position offset for the tile0s on the right side

			tile0 = &ml0.r[it0.y*ml0.tilecount.x + it0.x];
			tile1 = &ml1.r[it1.y*ml1.tilecount.x + it1.x];

			if ((tile0->dim.x & 1) + (tile0->dim.y & 1) == 0)		// if tile0 doesn't have partial 2x2 blocks
			{
				for (ip0.y=0; ip0.y < tile0->dim.y; ip0.y+=2)
				{
					ip1.y = (ip0.y >> 1) + off1.y;
					y0w = ip0.y*tile0->dim.x;
					y1w = ip1.y*tile1->dim.x;

					for (ip0.x=0; ip0.x < tile0->dim.x; ip0.x+=2)
					{
						ip1.x = (ip0.x >> 1) + off1.x;

						tile_pixel_sum_func(tile0, y0w + ip0.x, tile1, y1w + ip1.x, tile0->dim.x, 0);
					}
				}
			}
			else		// if it has partial 2x2 blocks
			{
				for (ip0.y=0; ip0.y < tile0->dim.y; ip0.y+=2)
				{
					ip1.y = (ip0.y >> 1) + off1.y;
					y0w = ip0.y*tile0->dim.x;
					y1w = ip1.y*tile1->dim.x;
					partial.y = (tile0->dim.y - ip0.y < 2) << 1;	// is set to 2 if partial in y

					for (ip0.x=0; ip0.x < tile0->dim.x; ip0.x+=2)
					{
						ip1.x = (ip0.x >> 1) + off1.x;
						partial.x = tile0->dim.x - ip0.x < 2;	// is set to 1 if partial in x

						tile_pixel_sum_func(tile0, y0w + ip0.x, tile1, y1w + ip1.x, tile0->dim.x, partial.y | partial.x);
					}
				}
			}
		}
	}
}

mipmap_t raster_to_tiled_mipmaps_fast(raster_t r, xyi_t tilesize, xyi_t mindim, const int mode)
{
	int i;
	mipmap_t m={0};

	m.fulldim = r.dim;
	m.lvl_count = 1. + log2(max_of_xy(div_xy(xyi_to_xy(m.fulldim), xyi_to_xy(mindim))));
	m.lvl_count = MAXN(m.lvl_count, 1);
	m.lvl = calloc(m.lvl_count, sizeof(mipmap_level_t));
	m.total_bytes = m.lvl_count * sizeof(mipmap_level_t);

	// the first level is a tiled copy of the original
	m.lvl[0].scale = set_xy(1.);
	alloc_mipmap_level(&m.lvl[0], m.fulldim, tilesize, mode);
	copy_from_raster_to_tiles(r, m.lvl[0], mode);

	for (i=1; i<m.lvl_count; i++)
	{
		m.lvl[i].fulldim = div_round_up_xyi(m.lvl[i-1].fulldim, set_xyi(2));
		m.lvl[i].scale = mul_xy(m.lvl[i-1].scale, set_xy(2.));
		alloc_mipmap_level(&m.lvl[i], m.lvl[i].fulldim, tilesize, mode);
		//tile_downscale_fast_box(m.lvl[i-1], m.lvl[i], set_xyi(2));
		tile_downscale_box_2x2(m.lvl[i-1], m.lvl[i], mode);
	}
	
	for (i=0; i<m.lvl_count; i++)
		m.total_bytes += m.lvl[i].total_bytes;

	return m;
}

mipmap_t raster_to_tiled_mipmaps_fast_defaults(raster_t r, const int mode)
{
	return raster_to_tiled_mipmaps_fast(r, set_xyi(MIPMAP_TILE_SIZE), set_xyi(MIPMAP_MIN_SIZE), mode);
}

mipmap_t raster_to_mipmap_then_free(raster_t *r, const int mode)
{
	mipmap_t mm;

	mm = raster_to_tiled_mipmaps_fast_defaults(*r, mode);
	free_raster(r);

	return mm;
}

void free_mipmap_level(mipmap_level_t *ml)
{
	int i;

	for (i=0; i < mul_x_by_y_xyi(ml->tilecount); i++)
		free_raster(&ml->r[i]);

	memset(ml, 0, sizeof(mipmap_level_t));
}

void free_mipmap(mipmap_t *m)
{
	int i;

	if (m==NULL)
		return;

	for (i=0; i<m->lvl_count; i++)
		free_mipmap_level(&m->lvl[i]);

	memset(m, 0, sizeof(mipmap_t));
}

void free_mipmap_array(mipmap_t *m, int count)
{
	int i;

	if (m == NULL)
		return ;

	for (i=0; i<count; i++)
		free_mipmap(&m[i]);
}

void remove_mipmap_levels_above_dim(mipmap_t *m, xyi_t dim)
{
	int i;

	for (i=0; i < m->lvl_count; i++)
		if (m->lvl[i].fulldim.x > dim.x || m->lvl[i].fulldim.y > dim.y)
		{
			m->total_bytes -= m->lvl[i].total_bytes;
			free_mipmap_level(&m->lvl[i]);
		}
}

void blit_mipmap_rotated(mipmap_t m, xy_t pscale, xy_t pos, double angle, xy_t rot_centre, int interp)
{
	int i;
	xyi_t it;
	xy_t pscale_inv = inv_xy(pscale), ml_scale;
	mipmap_level_t *ml = &m.lvl[0];

	// find the right mipmap level
	for (i=1; i < m.lvl_count; i++)
		if (m.lvl[i].r)			// skip over any lower level that have might been removed to save memory
			if ((m.lvl[i].scale.x <= pscale_inv.x && m.lvl[i].scale.y <= pscale_inv.y) || ml->r==NULL)
				ml = &m.lvl[i];
			else
				break;

	ml_scale = mul_xy(ml->scale, pscale);
	pos = sub_xy( add_xy( pos, mul_xy(ml_scale, set_xy(0.5)) ) , mul_xy(pscale, set_xy(0.5)) );	// add an offset necessary for higher mipmap levels

	for (it.y=0; it.y < ml->tilecount.y; it.y++)
		for (it.x=0; it.x < ml->tilecount.x; it.x++)
			blit_scale_rotated(&ml->r[it.y*ml->tilecount.x + it.x], ml_scale, add_xy(pos, mul_xy(ml_scale, xyi_to_xy(mul_xyi(it, ml->tiledim)))), angle, rot_centre, interp);
}

rect_t blit_mipmap_in_rect_rotated(mipmap_t m, rect_t r, int keep_aspect_ratio, double angle, xy_t rot_centre, int interp)
{
	xy_t pscale, pos;
	rect_t image_frame = r;

	if (m.lvl_count < 1 || m.lvl==NULL)
		return wc_rect(image_frame);

	if (keep_aspect_ratio)
		image_frame = fit_rect_in_area( xyi_to_xy(m.fulldim), image_frame, xy(0.5, 0.5) );

	pscale = div_xy(get_rect_dim(image_frame), xyi_to_xy(m.fulldim));
	pos = add_xy(keep_aspect_ratio ? image_frame.p0 : rect_p01(image_frame), mul_xy(pscale, set_xy(0.5)));

	blit_mipmap_rotated(m, pscale, pos, angle, rot_centre, interp);

	return wc_rect(image_frame);
}

int get_largest_mipmap_lvl_index(mipmap_t m)
{
	int i;

	for (i=0; i < m.lvl_count; i++)
		if (m.lvl[i].r)
			return i;

	return -1;
}

xyi_t get_largest_mipmap_lvl_dim(mipmap_t m)
{
	int i;

	for (i=0; i < m.lvl_count; i++)
		if (m.lvl[i].r)
			return m.lvl[i].fulldim;

	return xyi(0, 0);
}
