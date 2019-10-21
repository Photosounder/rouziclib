void convert_image_srgb8_fullarg(raster_t *im, const uint8_t *data, const int mode, int free_srgb)
{
	int i;
	lut_t lut = get_lut_slrgb();
	lut_t sqlut = get_lut_ssqrgb();
	size_t pix_count = mul_x_by_y_xyi(im->dim);

	if (mode & IMAGE_USE_SRGB)
	{
		if (im->srgb != data)
		{
			im->srgb = calloc(pix_count, sizeof(srgb_t));
			memcpy(im->srgb, data, pix_count * sizeof(srgb_t));
		}
	}

	if (mode & IMAGE_USE_LRGB)
	{
		if (im->l==NULL)
			im->l = calloc(pix_count, sizeof(lrgb_t));

		for (i=0; i < pix_count*4; i++)
			((uint16_t *) im->l)[i] = lut.lutint[data[i]];
	}

	if (mode & IMAGE_USE_FRGB)
	{
		if (im->f==NULL)
			im->f = calloc(pix_count, sizeof(frgb_t));

		for (i=0; i < pix_count*4; i++)
			((float *) im->f)[i] = s8lrgb(data[i]);
	}

	if (mode & IMAGE_USE_SQRGB)
	{
		if (im->sq==NULL)
			im->sq = calloc(pix_count, sizeof(sqrgb_t));

		for (i=0; i < pix_count; i++)
		{
			im->sq[i].r = sqlut.lutint[((srgb_t *)data)[i].r] >> 2;
			im->sq[i].g = sqlut.lutint[((srgb_t *)data)[i].g];
			im->sq[i].b = sqlut.lutint[((srgb_t *)data)[i].b] >> 2;
		}
	}

	if (free_srgb)
		if (mode & IMAGE_USE_SRGB == 0)		// free srgb in case it's there but isn't needed
			free_null(&im->srgb);
}

void convert_image_srgb16(raster_t *im, const uint16_t *data, const int mode)
{
	int i;
	lut_t lut = get_lut_slrgb();
	lut_t sqlut = get_lut_ssqrgb();
	const double ratio = 1./65535.;
	size_t pix_count = mul_x_by_y_xyi(im->dim);

	if (mode & IMAGE_USE_SRGB)
	{
		im->srgb = calloc(pix_count, sizeof(srgb_t));
		for (i=0; i < pix_count*4; i++)
			((uint8_t *) im->srgb)[i] = data[i] >> 8;
	}

	if (mode & IMAGE_USE_LRGB)
	{
		im->l = calloc(pix_count, sizeof(lrgb_t));
		for (i=0; i < pix_count*4; i++)
			((uint16_t *) im->l)[i] = s16lrgb(data[i]) * ONEF + 0.5;
	}

	if (mode & IMAGE_USE_FRGB)
	{
		im->f = calloc(pix_count, sizeof(frgb_t));
		for (i=0; i < pix_count*4; i++)
			((float *) im->f)[i] = s16lrgb(data[i]);
	}

	if (mode & IMAGE_USE_SQRGB)
	{
		im->sq = calloc(pix_count, sizeof(sqrgb_t));
		for (i=0; i < pix_count; i++)
		{
			im->sq[i].r = ssqrgb((double) data[i*4+0] * ratio) * 1023. + 0.5;
			im->sq[i].g = ssqrgb((double) data[i*4+1] * ratio) * 4092. + 0.5;
			im->sq[i].b = ssqrgb((double) data[i*4+2] * ratio) * 1023. + 0.5;
		}
	}
}

void convert_image_frgb(raster_t *im, const float *data, const int mode)
{
	int i;
	size_t pix_count = mul_x_by_y_xyi(im->dim);

	if (mode & IMAGE_USE_SRGB)
	{
		lut_t lsrgb_fl_l = get_lut_lsrgb_fl();

		im->srgb = calloc(pix_count, sizeof(srgb_t));
		for (i=0; i < pix_count*4; i++)
			((uint8_t *) im->srgb)[i] = lsrgb_fl(rangelimitf(data[i], 0.f, 1.f), lsrgb_fl_l.lutint) >> 5;
	}

	if (mode & IMAGE_USE_LRGB)
	{
		im->l = calloc(pix_count, sizeof(lrgb_t));
		for (i=0; i < pix_count*4; i++)
			((uint16_t *) im->l)[i] = rangelimitf(data[i], 0.f, 1.f) * ONEF + 0.5;
	}

	if (mode & IMAGE_USE_FRGB)
	{
		if (im->f != data)
		{
			im->f = calloc(pix_count, sizeof(frgb_t));
			memcpy(im->f, data, pix_count * sizeof(frgb_t));
		}
	}

	if (mode & IMAGE_USE_SQRGB)
	{
		im->sq = calloc(pix_count, sizeof(sqrgb_t));
		for (i=0; i < pix_count; i++)
		{
			im->sq[i].r = sqrtf(rangelimitf(data[i*4+0], 0.f, 1.f)) * 1023. + 0.5;
			im->sq[i].g = sqrtf(rangelimitf(data[i*4+1], 0.f, 1.f)) * 4092. + 0.5;
			im->sq[i].b = sqrtf(rangelimitf(data[i*4+2], 0.f, 1.f)) * 1023. + 0.5;
		}
	}

	if (mode & IMAGE_USE_FRGB == 0)		// free f in case it's there but isn't needed
		free_null(&im->f);
}

void convert_image_to_srgb16(raster_t *im, uint16_t *data, const int mode, const int chan)
{
	int i, ic, v;
	size_t pix_count = mul_x_by_y_xyi(im->dim);
	const double lratio = 1./ONEF;
	frgb_t fv;

	if (mode & IMAGE_USE_SRGB)
		for (i=0; i < pix_count; i++)
			for (ic=0; ic < chan; ic++)
			{
				v = ((uint8_t *) im->srgb)[i*4+ic];
				data[i*chan+ic] = (v << 8) | v;
			}

	if (mode & IMAGE_USE_LRGB)
		for (i=0; i < pix_count; i++)
			for (ic=0; ic < chan; ic++)
				data[i*chan+ic] = 65535. * lsrgb(((uint16_t *) im->l)[i*4+ic] * lratio) + 0.5;	// could use the LUT

	if (mode & IMAGE_USE_FRGB)
		for (i=0; i < pix_count; i++)
			for (ic=0; ic < chan; ic++)
				data[i*chan+ic] = 65535. * lsrgb(((float *) im->f)[i*4+ic]) + 0.5;

	if (mode & IMAGE_USE_SQRGB)
		for (i=0; i < pix_count; i++)
		{
			fv = sqrgb_to_frgb(im->sq[i]);
			for (ic=0; ic < chan; ic++)
				data[i*chan+ic] = 65535. * ((float *) &fv)[ic] + 0.5;
		}
}

raster_t load_image_mem_lib(image_load_mem_func_t load_func, uint8_t *raw_data, size_t size, const int mode)
{
	raster_t im={0};

	if (load_func)
		im = load_func(raw_data, size, mode);
	else
	{
		#ifdef load_image_mem
		im = load_image_mem(raw_data, size, mode);
		#else
		fprintf_rl(stderr, "No loading function defined for load_image_lib()\n");
		return im;
		#endif
	}

	return im;
}

raster_t load_image_lib(image_load_mem_func_t load_func, const char *path, const int mode)	// mode is IMAGE_USE_xRGB (S, SQ, L, F)
{
	uint8_t *raw_data;
	size_t size;
	raster_t im={0};

	raw_data = load_raw_file(path, &size);
	if (raw_data==NULL)
	{
		fprintf_rl(stderr, "Could not open image %s\n", path);
		return im;
	}

	im = load_image_mem_lib(load_func, raw_data, size, mode);

	if (im.dim.x+im.dim.y == 0)
		fprintf_rl(stderr, "Could not open image %s\n", path);

	free (raw_data);

	return im;
}

raster_t load_image_from_http_lib(image_load_mem_func_t load_func, char *url, const int mode)
{
#if defined(RL_INCL_NETWORK) || defined(RL_LIBCURL)
	size_t data_size, data_alloc=0;
	uint8_t *data=NULL;
	raster_t im={0};

	#ifdef RL_LIBCURL
	data_size = curl_https_get(url, -1, ONE_RETRY, &data, &data_alloc);
	#else
	data_size = http_get(url, -1, ONE_RETRY, &data, &data_alloc);
	#endif

	if (data_size > 0)
		im = load_image_mem_lib(load_func, data, data_size, mode);

	free(data);

	return im;
#else
	return noop_raster("load_image_from_http");
#endif
}

mipmap_t load_mipmap_from_http_lib(image_load_mem_func_t load_func, char *url, const int mode)
{
	mipmap_t m={0};
	raster_t image = load_image_from_http_lib(load_func, url, mode);

	if (get_raster_buffer_for_mode(image, mode)==NULL)
		return m;

	m = raster_to_tiled_mipmaps_fast_defaults(image, mode);
	free_raster(&image);

	return m;
}

mipmap_t load_mipmap_lib(image_load_mem_func_t load_func, const char *path, const int mode)
{
	mipmap_t m={0};
	raster_t image = load_image_lib(load_func, path, mode);

	if (get_raster_buffer_for_mode(image, mode)==NULL)
		return m;

	m = raster_to_tiled_mipmaps_fast_defaults(image, mode);
	free_raster(&image);

	return m;
}

raster_t load_image_mem_builtin(uint8_t *raw_data, size_t size, const int mode)
{
	raster_t im={0};

	if (is_file_tiff_mem(raw_data))
	{
		im = load_tiff_mem_raster(raw_data);
		convert_image_frgb(&im, im.f, mode);
	}
	else
		im = load_image_mem_libstb_image(raw_data, size, mode);

	return im;
}

raster_t load_file_tiles_to_raster(const char *dir_path, const char *filename_fmt, int x_first, const int mode)
{
	xyi_t it, tile_count={0};
	raster_t full_im={0}, *tile_r;
	char filename[PATH_MAX*4], path[PATH_MAX*4];
	int iy, pixel_size = get_raster_mode_elem_size(mode);

	// Find and count the tile files
	for (it.y=0; ; it.y++)
	{
		for (it.x=0; ; it.x++)
		{
			// look for the file with a name containing x and y
			if (x_first)
				sprintf(filename, filename_fmt, it.x, it.y);
			else
				sprintf(filename, filename_fmt, it.y, it.x);

			append_name_to_path(path, dir_path, filename);

			// stop going up x for this line this one was missing
			if (check_file_is_readable(path)==0)
			{
				tile_count.x = MAXN(tile_count.x, it.x);
				break;
			}
		}

		// stop going up y if there was no x=0 tile
		if (it.x==0)
		{
			tile_count.y = it.y;
			break;
		}
	}

	tile_r = calloc(mul_x_by_y_xyi(tile_count), sizeof(raster_t));

	// Load every tile
	for (it.y=0; it.y < tile_count.y; it.y++)
		for (it.x=0; it.x < tile_count.x; it.x++)
		{
			if (x_first)
				sprintf(filename, filename_fmt, it.x, it.y);
			else
				sprintf(filename, filename_fmt, it.y, it.x);

			append_name_to_path(path, dir_path, filename);

			tile_r[it.y*tile_count.x + it.x] = load_image(path, mode);
		}

	// Get the X dim
	for (it.y=0; it.y < tile_count.y; it.y++)
	{
		int width=0;

		for (it.x=0; it.x < tile_count.x; it.x++)
			width += tile_r[it.y*tile_count.x + it.x].dim.x;

		full_im.dim.x = MAXN(full_im.dim.x, width);
	}

	// Get the Y dim
	for (it.x=0; it.x < tile_count.x; it.x++)
	{
		int height=0;

		for (it.y=0; it.y < tile_count.y; it.y++)
			height += tile_r[it.y*tile_count.x + it.x].dim.y;

		full_im.dim.y = MAXN(full_im.dim.y, height);
	}

	// Copy each tile to the full image
	full_im = make_raster(NULL, full_im.dim, XYI0, mode);

	xyi_t offset={0};
	raster_t tile_p;
	for (it.y=0; it.y < tile_count.y; it.y++)
	{
		for (it.x=0; it.x < tile_count.x; it.x++)
		{
			tile_p = tile_r[it.y*tile_count.x + it.x];
			uint8_t *im_buf = get_raster_buffer_for_mode(full_im, mode);
			uint8_t *tile_buf = get_raster_buffer_for_mode(tile_p, mode);

			// Copy each line of the tile
			for (iy=0; iy < tile_p.dim.y; iy++)
				memcpy(&im_buf[((offset.y+iy)*full_im.dim.x + offset.x)*pixel_size], &tile_buf[iy*tile_p.dim.x*pixel_size], tile_p.dim.x * pixel_size);

			offset.x += tile_p.dim.x;

			free_raster(&tile_r[it.y*tile_count.x + it.x]);
		}

		offset.x = 0;
		offset.y += tile_p.dim.y;
	}

	free(tile_r);

	return full_im;
}
