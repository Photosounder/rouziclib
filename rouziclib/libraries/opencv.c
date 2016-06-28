#ifdef RL_OPENCV

void convert_cvimage_to_raster(IplImage *frame, raster_t *image)
{
	int32_t ix, iy, ic;
	static int init=1;
	float *pf;
	uint16_t *pl;
	uint8_t *ps;
	static lut_t slrgb_l;

	if (init)
	{
		init = 0;
		slrgb_l = get_lut_slrgb();
	}

	if (frame==NULL)		return ;
	if (frame->imageData==NULL)	return ;

	if (image->w != frame->width || image->h != frame->height)
	{
		image->w = frame->width;
		image->h = frame->height;

		if (image->use_frgb)
		{
			if (image->f)
				free (image->f);
			image->f = calloc(image->w*image->h, sizeof(frgb_t));
		}
		else
		{
			if (image->l)
				free (image->l);
			image->l = calloc(image->w*image->h, sizeof(lrgb_t));
		}
	}


	if (image->use_frgb)
		for (iy=0; iy<image->h; iy++)
			for (ix=0; ix<image->w; ix++)
			{
				pf = &image->f[iy*image->w+ix];
				ps = &frame->imageData[(iy*image->w+ix)*3];

				for (ic=0; ic<3; ic++)
					pf[ic] = slrgb_l.flut[ps[2-ic]];
			}
	else
		for (iy=0; iy<image->h; iy++)
			for (ix=0; ix<image->w; ix++)
			{
				pl = &image->l[iy*image->w+ix];
				ps = &frame->imageData[(iy*image->w+ix)*3];

				for (ic=0; ic<3; ic++)
					pl[ic] = slrgb_l.lutint[ps[2-ic]];
			}
}

void convert_srgb_to_cvimage(raster_t *image, IplImage *frame)
{
	int32_t ix, iy, ic;
	uint8_t *ps0, *ps1;

	for (iy=0; iy<image->h; iy++)
		for (ix=0; ix<image->w; ix++)
		{
			ps0 = &image->srgb[iy*image->w+ix];
			ps1 = &frame->imageData[(iy*image->w+ix)*3];

			for (ic=0; ic<3; ic++)
				ps1[ic] = ps0[ic];
		}
}

int get_webcam_frame(raster_t *image, char *debug_str)
{
	static CvCapture *capture;
	IplImage *frame;
	static int init=1;

	if (init)
	{
		init = 0;

		capture = cvCreateCameraCapture(0);
		if (capture==NULL)		return 0;

		cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH, 1280.);
		cvSetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT, 960.);
	}

	frame = cvQueryFrame(capture);

	convert_cvimage_to_raster(frame, image);

	sprintf(debug_str, "%gx%g, %.3g FPS, %g, %g", 
			cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_WIDTH), 
			cvGetCaptureProperty(capture, CV_CAP_PROP_FRAME_HEIGHT),
			cvGetCaptureProperty(capture, CV_CAP_PROP_FPS),
			cvGetCaptureProperty(capture, CV_CAP_PROP_GAIN),
			cvGetCaptureProperty(capture, CV_CAP_PROP_EXPOSURE),
			0);

	//cvReleaseCapture(&capture);

	return 1;
}

int get_video_frame(raster_t *image, char *path)
{
	static CvCapture *capture;
	IplImage *frame;
	static int init=1;

	if (init)
	{
		init = 0;

		capture = cvCaptureFromFile(path);
		if (capture==NULL)
			return 0;
	}

	frame = cvQueryFrame(capture);

	convert_cvimage_to_raster(frame, image);

	//cvReleaseCapture(&capture);		// TODO close it at some point

	return 1;
}

void write_video_frame(raster_t *image, char *path, double fps, int action)	// FIXME can only handle one video file at a time
{
	static CvVideoWriter *writer=NULL;
	static IplImage *frame=NULL;
return ;

	if (action==0 && path)		// initiate new file
	{
		if (writer)
		{
			cvReleaseVideoWriter(writer);
			cvReleaseImage(frame);
		}

		writer = cvCreateVideoWriter(path, CV_FOURCC('X','V','I','D'), fps, cvSize(image->w, image->h), 1);
		frame = cvCreateImage(cvSize(image->w, image->h), IPL_DEPTH_8U, 3);
	}

	if (action==2 && writer)	// close and clean up
	{
			cvReleaseImage(frame);	// FIXME crashes
			frame = NULL;

			cvReleaseVideoWriter(writer);
			writer = NULL;
	}

	if (action==1 && writer)
	{
		convert_srgb_to_cvimage(image, frame);
		cvWriteFrame(writer, frame);
	}
}

#endif
