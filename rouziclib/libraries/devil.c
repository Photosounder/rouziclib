#ifdef RL_DEVIL

raster_t load_image_libdevil_from_memory(ILubyte *raw_data, ILuint size, const int mode, const void *clctx)
{
	ILboolean err;
	ILuint ImgId;
	int32_t bpp, format;
	raster_t im;

	if (raw_data==NULL || size==0)
		return make_raster_l(NULL, 0, 0);

	// Initialize DevIL.
	ilInit();
	ilOriginFunc(IL_ORIGIN_UPPER_LEFT);
	ilEnable(IL_ORIGIN_SET);

	ilGenImages(1, &ImgId);		// Generate the main image name to use.
	ilBindImage(ImgId);		// Bind this image name.

	if (!ilLoadL(IL_TYPE_UNKNOWN, raw_data, size))
	{
		fprintf_rl(stderr, "Could not open image from the %d byte buffer in memory\n", size);
		return make_raster_l(NULL, 0, 0);
	}

	memset(&im, 0, sizeof(raster_t));
	im.w = ilGetInteger(IL_IMAGE_WIDTH);
	im.h = ilGetInteger(IL_IMAGE_HEIGHT);
	bpp = ilGetInteger(IL_IMAGE_BITS_PER_PIXEL);
	format = ilGetInteger(IL_IMAGE_FORMAT);

	err = ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);	// TODO decide of IL_FLOAT conversion depending on bpp/format

	convert_image_srgb8(&im, ilGetData(), mode, clctx);	// fills all the necessary buffers with the image data

	ilDeleteImages(1, &ImgId);

	return im;
}

raster_t load_image_libdevil(const char *in_path, const int mode, const void *clctx)	// mode is IMAGE_USE_xRGB (S, L, F) and optionally | IMAGE_USE_CL
{
	ILubyte *raw_data;
	size_t size;
	raster_t im;

	raw_data = load_raw_file(in_path, &size);
	if (raw_data==NULL)
	{
		fprintf_rl(stderr, "Could not open image %s\n", in_path);
		return make_raster_l(NULL, 0, 0);
	}

	im = load_image_libdevil_from_memory(raw_data, size, mode, clctx);
	if (im.w+im.h == 0)
		fprintf_rl(stderr, "Could not open image %s\n", in_path);

	free (raw_data);

	return im;
}

/*int load_image_libdevil_thread(load_image_libdevil_thread_data_t *d)
{
	raster_t r;

	r = load_image_libdevil(d->in_path, d->mode, d->clctx);
}

void load_image_libdevil_threaded(raster_t *im, const char *in_path, const int mode, const void *clctx)
{
	load_image_libdevil_thread_data_t d;

	d.im = im;
	d.in_path = in_path;
	d.mode = mode;
	d.clctx = clctx;
	
	// Wait for thread to end
	thrd_join(im->thread_handle, NULL);
	free_raster(im);

	// Create thread
	thrd_create(&im->thread_handle, load_image_libdevil_thread, &d);
}*/

#endif
