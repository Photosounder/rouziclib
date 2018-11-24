void convert_image_srgb8(raster_t *im, const uint8_t *data, const int mode)
{
	int32_t i;
	lut_t lut = get_lut_slrgb();
	lut_t sqlut = get_lut_ssqrgb();

	if (mode & IMAGE_USE_SRGB)
	{
		if (im->srgb != data)
		{
			im->srgb = calloc(im->dim.x * im->dim.y, sizeof(srgb_t));
			memcpy(im->srgb, data, im->dim.x * im->dim.y * sizeof(srgb_t));
		}
	}

	if (mode & IMAGE_USE_LRGB)
	{
		im->l = calloc(im->dim.x * im->dim.y, sizeof(lrgb_t));
		for (i=0; i<im->dim.x*im->dim.y*4; i++)
			((uint16_t *) im->l)[i] = lut.lutint[data[i]];
	}

	if (mode & IMAGE_USE_FRGB)
	{
		im->f = calloc(im->dim.x * im->dim.y, sizeof(frgb_t));
		for (i=0; i<im->dim.x*im->dim.y; i++)
		{
			im->f[i].r = lut.flut[((srgb_t *)data)[i].r];
			im->f[i].g = lut.flut[((srgb_t *)data)[i].g];
			im->f[i].b = lut.flut[((srgb_t *)data)[i].b];
			im->f[i].a = lut.flut[((srgb_t *)data)[i].a];
		}
	}

	if (mode & IMAGE_USE_SQRGB)
	{
		im->sq = calloc(im->dim.x * im->dim.y, sizeof(sqrgb_t));
		for (i=0; i<im->dim.x*im->dim.y; i++)
		{
			im->sq[i].r = sqlut.lutint[((srgb_t *)data)[i].r] >> 2;
			im->sq[i].g = sqlut.lutint[((srgb_t *)data)[i].g];
			im->sq[i].b = sqlut.lutint[((srgb_t *)data)[i].b] >> 2;
		}
	}

	if (mode & IMAGE_USE_SRGB == 0)
		free_null(&im->srgb);
}

void convert_image_srgb16(raster_t *im, const uint16_t *data, const int mode)
{
	int32_t i;
	lut_t lut = get_lut_slrgb();
	lut_t sqlut = get_lut_ssqrgb();
	const double ratio = 1./65535.;

	if (mode & IMAGE_USE_SRGB)
	{
		im->srgb = calloc(im->dim.x * im->dim.y, sizeof(srgb_t));
		for (i=0; i<im->dim.x*im->dim.y*4; i++)
			((uint8_t *) im->srgb)[i] = data[i] >> 8;
	}

	if (mode & IMAGE_USE_LRGB)
	{
		im->l = calloc(im->dim.x * im->dim.y, sizeof(lrgb_t));
		for (i=0; i<im->dim.x*im->dim.y*4; i++)
			((uint16_t *) im->l)[i] = slrgb((double) data[i] * ratio) * ONEF + 0.5;
	}

	if (mode & IMAGE_USE_FRGB)
	{
		im->f = calloc(im->dim.x * im->dim.y, sizeof(frgb_t));
		for (i=0; i<im->dim.x*im->dim.y*4; i++)
			((float *) im->f)[i] = slrgb((double) data[i] * ratio);
	}

	if (mode & IMAGE_USE_SQRGB)
	{
		im->sq = calloc(im->dim.x * im->dim.y, sizeof(sqrgb_t));
		for (i=0; i<im->dim.x*im->dim.y; i++)
		{
			im->sq[i].r = ssqrgb((double) data[i*4+0] * ratio) * 1023. + 0.5;
			im->sq[i].g = ssqrgb((double) data[i*4+1] * ratio) * 4092. + 0.5;
			im->sq[i].b = ssqrgb((double) data[i*4+2] * ratio) * 1023. + 0.5;
		}
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
#ifdef RL_INCL_NETWORK
	int data_size, data_alloc=0;
	uint8_t *data=NULL;
	raster_t im={0};

	data_size = http_get(url, -1, ONE_RETRY, &data, &data_alloc);

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
