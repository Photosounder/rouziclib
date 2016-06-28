#ifdef RL_DEVIL

raster_t load_image_libdevil(const char *in_path, const int mode, const void *clctx)	// mode is IMAGE_USE_xRGB (S, L, F)
{
	raster_t im;
	int32_t ix, iy, bpp, format;

	memset(&im, 0, sizeof(raster_t));

	ILuint	ImgId, size;
	ILubyte *raw_data;
	ILboolean err;

	// Initialize DevIL.
	ilInit();
	ilOriginFunc(IL_ORIGIN_UPPER_LEFT);
	ilEnable(IL_ORIGIN_SET);	

	ilGenImages(1, &ImgId);		// Generate the main image name to use.
	ilBindImage(ImgId);		// Bind this image name.

	raw_data = load_raw_file(in_path, &size);
	if (raw_data==NULL)
	{
		fprintf_rl(stderr, "Could not open image %s\n", in_path);
		return make_raster_l(NULL, 0, 0);
	}

	if (!ilLoadL(IL_TYPE_UNKNOWN, raw_data, size))
	{
		fprintf_rl(stderr, "Could not open image %s\n", in_path);
		return make_raster_l(NULL, 0, 0);
	}

	im.w = ilGetInteger(IL_IMAGE_WIDTH);
	im.h = ilGetInteger(IL_IMAGE_HEIGHT);
	bpp = ilGetInteger(IL_IMAGE_BITS_PER_PIXEL);
	format = ilGetInteger(IL_IMAGE_FORMAT);

	err = ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);	// TODO decide of IL_FLOAT conversion depending on bpp/format

	convert_image_srgb8(&im, ilGetData(), mode, clctx);		// fills all the necessary buffers with the image data

	ilDeleteImages(1, &ImgId);
	free (raw_data);

	return im;
}

#endif
