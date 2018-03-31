#ifdef RL_LIBRAW

rawphoto_t init_rawphoto(int32_t width, int32_t height)
{
	rawphoto_t rp;

	memset(&rp, 0, sizeof(rawphoto_t));

	rp.w = width;
	rp.h = height;
	rp.data = calloc(rp.w*rp.h, sizeof(*rp.data));

	return rp;
}

void free_rawphoto(rawphoto_t *rp)
{
	free (rp->data);
	free_raster(&rp->preview);

	memset(rp, 0, sizeof(rawphoto_t));
}

raster_t load_raw_thumb(libraw_data_t *rd)
{
	int ret;
	libraw_processed_image_t *proc_image;
	raster_t r;

	ret = libraw_unpack_thumb(rd);
	if (ret)
	{
		fprintf_rl(stderr, "libraw %s\n", libraw_strerror(ret));
		return make_raster_f(NULL, 0, 0);
	}

	proc_image = libraw_dcraw_make_mem_thumb(rd, &ret);
	if (ret)
		fprintf_rl(stderr, "libraw %s\n", libraw_strerror(ret));
	else if (proc_image->type == LIBRAW_IMAGE_JPEG)
	{
		r = load_image_mem(proc_image->data, proc_image->data_size, IMAGE_USE_FRGB);
	}
	else if (proc_image->colors!=3 || proc_image->bits!=8)
	{
		fprintf_rl(stderr, "Wrong decoded thumbnail format, %d %d-bit channels, type %d\n", proc_image->colors, proc_image->bits, proc_image->type);
		r = make_raster_f(NULL, 0, 0);
	}
	else
	{
		r = make_raster_f(NULL, proc_image->width, proc_image->height);
		convert_image_srgb8(&r, proc_image->data, IMAGE_USE_FRGB);
	}

	libraw_dcraw_clear_mem(proc_image);

	return r;
}

rawphoto_t load_raw_photo_bayered(char *path, int load_thumb)
{
	int i, ret;
	rawphoto_t rp={0};
	libraw_data_t *rd = libraw_init(0);
	uint8_t *raw_data;
	size_t raw_data_size;

	raw_data = load_raw_file(path, &raw_data_size);
	if (raw_data==NULL)
	{
		fprintf_rl(stderr, "Could not open image %s\n", path);
		return rp;
	}

	//ret = libraw_open_file(rd, path);
	ret = libraw_open_buffer(rd, raw_data, raw_data_size);
	//free (raw_data);
	if (ret)
	{
		fprintf_rl(stderr, "%s: libraw %s\n", path, libraw_strerror(ret));
		return rp;
	}

	//fprintf_rl(stdout, "Processing %s (%s %s)\n", path, rd->idata.make, rd->idata.model);

	ret = libraw_unpack(rd);
	if (ret)
		fprintf_rl(stderr, "%s: libraw %s\n", path, libraw_strerror(ret));

	// copy the wanted data
	rp = init_rawphoto(rd->rawdata.sizes.raw_width, rd->rawdata.sizes.raw_height);
	memcpy(rp.data, rd->rawdata.raw_image, rp.w*rp.h * sizeof(uint16_t));

	if (load_thumb)
		rp.preview = load_raw_thumb(rd);

	rp.image_area = make_rect_off( xy(rd->rawdata.sizes.left_margin, rd->rawdata.sizes.top_margin), xy(rd->rawdata.sizes.iwidth-1, rd->rawdata.sizes.iheight-1), XY0 );
	rp.image_centre = get_rect_centre(rp.image_area);

	// white balance
	rp.wb = xyz(rd->color.cam_mul[0], (rd->color.cam_mul[1]+rd->color.cam_mul[3])*0.5, rd->color.cam_mul[2]);
	rp.wb = div_xyz(rp.wb, set_xyz(min_of_xyz(rp.wb)));

	// black levels
	for (i=0; i<4; i++)
		rp.bayer_black[i] = rd->color.black + rd->color.cblack[i];
	
	rp.maximum_value = rd->color.maximum;

	fprintf_rl(stdout, "wb: %g %g %g\n", rp.wb.x, rp.wb.y, rp.wb.z);
	fprintf_rl(stdout, "Focal length: %g mm (%gx)\n", rd->other.focal_len, rd->other.focal_len/rd->lens.makernotes.MinFocal);
	if (rd->other.shutter > 0.26)
		fprintf_rl(stdout, "Shutter speed %g sec, ", rd->other.shutter);
	else
		fprintf_rl(stdout, "Shutter speed 1/%g sec, ", 1./rd->other.shutter);
	fprintf_rl(stdout, "F%.1f, ISO %g\n", rd->lens.makernotes.CurAp, rd->other.iso_speed);

	libraw_close(rd);

	return rp;
}

raster_t raw_photo_to_raster(framebuffer_t fb, rawphoto_t rp)
{
	int i, bayer_ind, col_ind;
	double vl, v;
	raster_t r;
	xyi_t ip, im_p0 = xy_to_xyi(rp.image_area.p0), im_p1 = xy_to_xyi(rp.image_area.p1);
	xyi_t im_dim = xy_to_xyi(get_rect_dim(rp.image_area));
	double H, S, L;

	if (rp.data==NULL)
		return make_raster_f(NULL, 0, 0);
	
	r = make_raster_f(NULL, im_dim.x+1, im_dim.y+1);

	for (ip.y=im_p0.y; ip.y <= im_p1.y; ip.y++)
		for (ip.x=im_p0.x; ip.x <= im_p1.x; ip.x++)
		{
			bayer_ind = (ip.x&1) + ((ip.y&1) << 1);
			col_ind = bayer_ind - (bayer_ind>>1);

			vl = rp.data[ip.y * rp.w + ip.x];
			v = (vl - rp.bayer_black[bayer_ind]) / (rp.maximum_value-rp.bayer_black[bayer_ind]) * get_xyz_index(rp.wb, col_ind);// * gain;

			set_frgb_channel(&r.f[(ip.y-im_p0.y) * r.dim.x + (ip.x-im_p0.x)], col_ind, v * (col_ind==1 ? 2. : 4.));
		}

	gaussian_blur(r.f, r.f, r.dim, 4, 1.4);

/*	for (ip.y=im_p0.y; ip.y <= im_p1.y; ip.y++)
		for (ip.x=im_p0.x; ip.x <= im_p1.x; ip.x++)
		{
			bayer_ind = (ip.x&1) + ((ip.y&1) << 1);
			col_ind = bayer_ind - (bayer_ind>>1);

			vl = rp.data[ip.y * rp.w + ip.x];
			v = (vl - rp.bayer_black[bayer_ind]) / (rp.maximum_value-rp.bayer_black[bayer_ind]) * get_xyz_index(rp.wb, col_ind);// * gain;

			frgb_to_hsl(r.f[(ip.y-im_p0.y) * r.dim.x + (ip.x-im_p0.x)], &H, &S, &L, HUEDEG);
			r.f[(ip.y-im_p0.y) * r.dim.x + (ip.x-im_p0.x)] = hsl_to_frgb(H, S, v / get_frgb_channel(hsl_to_frgb(H, S, 1., HUEDEG, 0), col_ind) , HUEDEG, 0);

			//set_frgb_channel(&r.f[(ip.y-im_p0.y) * r.dim.x + (ip.x-im_p0.x)], col_ind, v * (col_ind==1 ? 2. : 4.));
		}*/

#ifdef RL_OPENCL
	init_raster_cl(&r, &fb.clctx);			// copies the data to an OpenCL buffer
#endif

	return r;
}

#endif
