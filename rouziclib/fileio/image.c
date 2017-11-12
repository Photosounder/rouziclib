void convert_image_srgb8(raster_t *im, const uint8_t *data, const int mode, const void *clctx)
{
	int32_t i;
	lut_t lut = get_lut_slrgb();

	if (mode & IMAGE_USE_SRGB)
	{
		im->srgb = calloc(im->w * im->h, sizeof(srgb_t));
		memcpy(im->srgb, data, im->w * im->h * sizeof(srgb_t));
	}

	if (mode & IMAGE_USE_LRGB)
	{
		im->l = calloc(im->w * im->h, sizeof(lrgb_t));
		for (i=0; i<im->w*im->h; i++)
		{
			im->l[i].r = lut.lutint[((srgb_t *)data)[i].r];
			im->l[i].g = lut.lutint[((srgb_t *)data)[i].g];
			im->l[i].b = lut.lutint[((srgb_t *)data)[i].b];
			im->l[i].a = lut.lutint[((srgb_t *)data)[i].a];
		}
	}

	if (mode & (IMAGE_USE_FRGB | IMAGE_USE_CL))
	{
		im->f = calloc(im->w * im->h, sizeof(frgb_t));
		for (i=0; i<im->w*im->h; i++)
		{
			im->f[i].r = lut.flut[((srgb_t *)data)[i].r];
			im->f[i].g = lut.flut[((srgb_t *)data)[i].g];
			im->f[i].b = lut.flut[((srgb_t *)data)[i].b];
			im->f[i].a = lut.flut[((srgb_t *)data)[i].a];
		}
	}

	#ifdef RL_OPENCL
	if (mode & IMAGE_USE_CL)
	{
		init_raster_cl(im, clctx);

		if ((mode & IMAGE_USE_FRGB) == 0)
			free (im->f);
	}
	#endif
}

raster_t load_image_from_http(char *url, const int mode)
{
#ifndef RL_EXCL_NETWORK
	int data_size, data_alloc=0;
	uint8_t *data=NULL;
	raster_t im;

	data_size = http_get(url, -1, ONE_RETRY, &data, &data_alloc);

	#ifdef RL_DEVIL
	if (data_size > 0)
		im = load_image_libdevil_from_memory(data, data_size, mode, NULL);
	else
	#endif
		im = make_raster_f(NULL, 0, 0);

	free(data);

	return im;
#endif
}
