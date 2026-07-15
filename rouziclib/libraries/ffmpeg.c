#ifdef RL_FFMPEG

static char *ffmpeg_get_error_text(const int error)
{
	static char error_buffer[255];
	av_strerror(error, error_buffer, sizeof(error_buffer));
	return error_buffer;
}

int ffmpeg_retval(const int ret)
{
	if (ret < 0 && ret != AVERROR(EAGAIN))
		fprintf_rl(stdout, "LibAV error: %s\n", ffmpeg_get_error_text(ret));

	return ret < 0;
}

// TODO try hwaccel stuff from https://www.ffmpeg.org/doxygen/4.0/hw_decode_8c-example.html

int ff_init_stream(ffstream_t *s, const int stream_type)	// returns 1 on success
{
	int ret;

	// Find stream
	s->stream_id = av_find_best_stream(s->fmt_ctx, stream_type, -1, -1, NULL, 0);

	if (s->stream_id < 0)
	{
		ffmpeg_retval(s->stream_id);
		s->stream_id = -1;
		return 0;
	}

	// Load codec
	s->codec = avcodec_find_decoder(s->fmt_ctx->streams[s->stream_id]->codecpar->codec_id);
	if (s->codec==NULL)
	{
		fprintf_rl(stderr, "Couldn't find FFmpeg decoder for stream %d\n", s->stream_id);
		s->stream_id = -1;
		return 0;
	}

	s->codec_ctx = avcodec_alloc_context3(s->codec);
	if (s->codec_ctx==NULL)
	{
		fprintf_rl(stderr, "avcodec_alloc_context3() failed in ff_init_stream()\n");
		s->stream_id = -1;
		return 0;
	}

	s->codec_ctx->thread_count = s->thread_count;
	s->codec_ctx->thread_type = FF_THREAD_FRAME;

	ret = avcodec_parameters_to_context(s->codec_ctx, s->fmt_ctx->streams[s->stream_id]->codecpar);
	ffmpeg_retval(ret);
	if (ret < 0)
	{
		avcodec_close(s->codec_ctx);
		s->stream_id = -1;
		return 0;
	}

	ret = avcodec_open2(s->codec_ctx, s->codec, NULL);
	ffmpeg_retval(ret);
	if (ret < 0)
	{
		avcodec_close(s->codec_ctx);
		s->stream_id = -1;
		return 0;
	}

	// Frame
	s->frame = av_frame_alloc();
	if (s->frame==NULL)
	{
		fprintf_rl(stderr, "av_frame_alloc() failed in ff_init_stream()\n");
		avcodec_close(s->codec_ctx);
		s->stream_id = -1;
		return 0;
	}

	// Packet allocation
	s->packet = av_packet_alloc();
	if (s->packet==NULL)
	{
		fprintf_rl(stderr, "av_packet_alloc() failed in ff_init_stream()\n");
		av_frame_free(&s->frame);
		avcodec_close(s->codec_ctx);
		s->stream_id = -1;
		return 0;
	}

	return 1;
}

ffstream_t ff_load_stream_init(char const *path, const int stream_type, const int thread_count)
{
	int ret;
	ffstream_t s={0};

	s.stream_id = -1;
	s.thread_count = thread_count;

	if (path == NULL)
		return s;

	// Open file
	if ((ret = avformat_open_input(&s.fmt_ctx, path, NULL, NULL)))
	{
		ffmpeg_retval(ret);
		fprintf_rl(stderr, "Couldn't avformat_open_input() file '%s'\n", path);
		return s;
	}

	ret = avformat_find_stream_info(s.fmt_ctx, NULL);
	ffmpeg_retval(ret);
	if (ret < 0)
	{
		avformat_close_input(&s.fmt_ctx);
		return s;
	}

	// Init
	if (ff_init_stream(&s, stream_type)==0)
		ffstream_close_free(&s);

	return s;
}

static int ff_receive_stream_frame_ret(ffstream_t *s, int *retp)
{
	// Return decoded output data (in frame) from a decoder
	int ret = avcodec_receive_frame(s->codec_ctx, s->frame);
	if (retp)
		*retp = ret;

	if (ret >= 0)
		return 1;

	if (ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
		ffmpeg_retval(ret);

	return 0;
}

static int ff_receive_stream_frame(ffstream_t *s)
{
	return ff_receive_stream_frame_ret(s, NULL);
}

static void ff_flush_stream_decoder(ffstream_t *s)
{
	if (s->packet_pending)
	{
		av_packet_unref(s->packet);
		s->packet_pending = 0;
	}

	if (s->codec_ctx)
		avcodec_flush_buffers(s->codec_ctx);
}

static int ff_frame_key_frame(AVFrame *frame)
{
#ifdef AV_FRAME_FLAG_KEY
	return (frame->flags & AV_FRAME_FLAG_KEY) != 0;
#else
	return frame->key_frame;
#endif
}

static int64_t ff_frame_pkt_pos(ffstream_t *s)
{
#if LIBAVUTIL_VERSION_MAJOR >= 60
	return s->byte_pos;
#else
	return s->frame->pkt_pos;
#endif
}

static int64_t ff_frame_timestamp(AVFrame *frame)
{
	if (frame->best_effort_timestamp != AV_NOPTS_VALUE)
		return frame->best_effort_timestamp;
	if (frame->pts != AV_NOPTS_VALUE)
		return frame->pts;
	if (frame->pkt_dts != AV_NOPTS_VALUE)
		return frame->pkt_dts;

	return AV_NOPTS_VALUE;
}

static int64_t ff_frame_duration(AVFrame *frame)
{
#if LIBAVUTIL_VERSION_MAJOR >= 60
	return frame->duration;
#else
	return frame->pkt_duration;
#endif
}

static int64_t ff_frame_duration_ts(ffstream_t *s, AVFrame *frame)
{
	int64_t duration = ff_frame_duration(frame);
	AVStream *st;
	AVRational frame_rate;
	double tb;

	if (duration > 0)
		return duration;

	st = s->fmt_ctx->streams[s->stream_id];
	tb = av_q2d(st->time_base);
	if (tb <= 0.)
		return 0;

	if (s->codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO && frame->nb_samples > 0 && s->codec_ctx->sample_rate > 0)
		return (int64_t) nearbyint(((double) frame->nb_samples / (double) s->codec_ctx->sample_rate) / tb);

	if (s->codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO)
	{
		frame_rate = av_guess_frame_rate(s->fmt_ctx, st, frame);
		if (frame_rate.num > 0 && frame_rate.den > 0)
			return (int64_t) nearbyint(((double) frame_rate.den / (double) frame_rate.num) / tb);
	}

	return 0;
}

static int ff_frame_pix_fmt(ffstream_t *s)
{
	if (s->frame && s->frame->format != AV_PIX_FMT_NONE)
		return s->frame->format;

	return s->codec_ctx->pix_fmt;
}

static xyi_t ff_frame_dim(ffstream_t *s)
{
	if (s->frame && s->frame->width > 0 && s->frame->height > 0)
		return xyi(s->frame->width, s->frame->height);

	return xyi(s->codec_ctx->width, s->codec_ctx->height);
}

int ff_load_stream_packet(ffstream_t *s)
{
	int ret, recv_ret;

	// Return already decoded frames before feeding more packets. Otherwise codecs
	// with delayed output can reject the next packet with EAGAIN and the packet is lost.
	if (ff_receive_stream_frame_ret(s, &recv_ret))
		return 1;
	if (recv_ret == AVERROR_EOF)
	{
		if (s->packet_pending)
		{
			av_packet_unref(s->packet);
			s->packet_pending = 0;
		}
		return 0;
	}
	if (recv_ret != AVERROR(EAGAIN))
	{
		return 0;
	}

	while (s->packet_pending)
	{
		ret = avcodec_send_packet(s->codec_ctx, s->packet);
		if (ret == AVERROR(EAGAIN))
		{
			if (ff_receive_stream_frame_ret(s, &recv_ret))
				return 1;
			if (recv_ret == AVERROR_EOF)
			{
				av_packet_unref(s->packet);
				s->packet_pending = 0;
			}

			return 0;
		}

		av_packet_unref(s->packet);
		s->packet_pending = 0;

		if (ret == AVERROR_EOF)
			return 0;
		if (ret < 0)
		{
			ffmpeg_retval(ret);
			return 0;
		}

		if (ff_receive_stream_frame(s))
			return 1;
	}

	while ((ret = av_read_frame(s->fmt_ctx, s->packet)) >= 0)	// get the next frame from the file
	{
		if (s->packet->stream_index == s->stream_id)		// check that it's the right stream
		{
			s->byte_pos = s->packet->pos;
			s->packet_pending = 1;

			ret = avcodec_send_packet(s->codec_ctx, s->packet);	// supply raw packet data as input to a decoder
			if (ret != AVERROR(EAGAIN))
			{
				av_packet_unref(s->packet);
				s->packet_pending = 0;
			}

			if (ret == AVERROR_EOF)
				return 0;
			if (ret == AVERROR(EAGAIN))
			{
				if (ff_receive_stream_frame_ret(s, &recv_ret))
					return 1;
				if (recv_ret == AVERROR_EOF)
				{
					av_packet_unref(s->packet);
					s->packet_pending = 0;
				}

				return 0;
			}
			if (ret < 0)
			{
				ffmpeg_retval(ret);
				continue;
			}

			if (ff_receive_stream_frame(s))
				return 1;
		}
		else
			av_packet_unref(s->packet);
	}

	if (ret != AVERROR_EOF)
		ffmpeg_retval(ret);

	// Flush the decoder
	ret = avcodec_send_packet(s->codec_ctx, NULL);
	if (ret != AVERROR_EOF && ret < 0)
		ffmpeg_retval(ret);

	return ff_receive_stream_frame(s);
}

void ffstream_close_free(ffstream_t *s)
{
	if (s==NULL)
		return ;

	free_null(&s->frame_info);
	av_frame_free(&s->frame);
	av_packet_free(&s->packet);
	avcodec_close(s->codec_ctx);
	avformat_close_input(&s->fmt_ctx);
	memset(s, 0, sizeof(ffstream_t));

	// Preserve the invalid stream marker after fully clearing the structure
	s->stream_id = -1;
}

double ff_get_timestamp(ffstream_t *s, int64_t timestamp)
{
	AVRational time_base = s->fmt_ctx->streams[s->stream_id]->time_base;
	int64_t start_time = s->fmt_ctx->streams[s->stream_id]->start_time;

	if (timestamp == AV_NOPTS_VALUE)
		return NAN;
	if (start_time != AV_NOPTS_VALUE)
		timestamp -= start_time;

	return (double) timestamp * av_q2d(time_base);
}

double ff_get_frame_timestamp(ffstream_t *s)
{
	return ff_get_timestamp(s, ff_frame_timestamp(s->frame));
}

int64_t ff_make_timestamp(ffstream_t *s, double t)
{
	AVRational time_base = s->fmt_ctx->streams[s->stream_id]->time_base;
	int64_t start_time = s->fmt_ctx->streams[s->stream_id]->start_time;
	int64_t timestamp;

	timestamp = (int64_t) nearbyint(t / av_q2d(time_base));
	if (start_time != AV_NOPTS_VALUE)
		timestamp += start_time;

	return timestamp;
}

/*raster_t ff_frame_to_raster(ffstream_t *s, const int mode)
{
	AVFrame *frame_rgb = av_frame_alloc();
	int alloc_size;
	uint8_t *buffer;
	raster_t im={0};
	struct SwsContext *sws_ctx={0};
	const int rgb_fmt = AV_PIX_FMT_0BGR32;
uint32_t td=0;
get_time_diff(&td);

	sws_ctx = sws_getCachedContext( sws_ctx,
				 s->frame->width,
				 s->frame->height,
				 s->codec_ctx->pix_fmt,
				 s->frame->width,
				 s->frame->height,
				 rgb_fmt,
				 SWS_BILINEAR,
				 NULL,
				 NULL,
				 NULL );

	alloc_size = av_image_get_buffer_size(rgb_fmt, s->codec_ctx->width, s->codec_ctx->height, 1);
	buffer = av_malloc(alloc_size);
//fprintf_rl(stdout, "\n\t sws_getCachedContext() took %d ms\n", get_time_diff(&td));

	av_image_fill_arrays(frame_rgb->data, frame_rgb->linesize, buffer, rgb_fmt, s->codec_ctx->width, s->codec_ctx->height, 1);
//fprintf_rl(stdout, "\t av_image_fill_arrays() took %d ms\n", get_time_diff(&td));

	sws_scale(sws_ctx, s->frame->data, s->frame->linesize, 0, s->frame->height, frame_rgb->data, frame_rgb->linesize);
//fprintf_rl(stdout, "\t sws_scale() took %d ms\n", get_time_diff(&td));

	im.dim = xyi(s->codec_ctx->width, s->codec_ctx->height);
	convert_image_srgb8(&im, frame_rgb->data[0], mode);
//fprintf_rl(stdout, "\t convert_image_srgb8() took %d ms\n", get_time_diff(&td));

	av_free(buffer);
	av_frame_free(&frame_rgb);
	sws_freeContext(sws_ctx);

	return im;
}*/

int ff_pix_fmt_byte_count(int pix_fmt)
{
	const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(pix_fmt);

	if (desc && desc->nb_components > 0)
		return (desc->comp[0].depth + 7) >> 3;

	return 1;
}

int ff_pix_fmt_bit_depth(int pix_fmt)
{
	const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(pix_fmt);

	if (desc && desc->nb_components > 0)
		return desc->comp[0].depth;

	return 8;
}

int ff_pix_fmt_to_buf_fmt(int pix_fmt)
{
	switch (pix_fmt)
	{
		case AV_PIX_FMT_YUV420P:
			return 10;

		case AV_PIX_FMT_YUV420P10LE:
			return 11;

		case AV_PIX_FMT_YUV420P12LE:
			return 12;

		case AV_PIX_FMT_YUVJ420P:
			return 15;
	}

	return -1;
}

int ff_buf_fmt_to_pix_fmt(int buf_fmt)
{
	switch (buf_fmt)
	{
		case 10:
			return AV_PIX_FMT_YUV420P;

		case 11:
			return AV_PIX_FMT_YUV420P10LE;

		case 12:
			return AV_PIX_FMT_YUV420P12LE;

		case 15:
			return AV_PIX_FMT_YUVJ420P;
	}

	return -1;
}

static int ff_pix_fmt_is_planar_yuv(int pix_fmt)
{
	const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(pix_fmt);

	// Check component layout
	if (desc==NULL)
		return 0;
	if ((desc->flags & (AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_PAL | AV_PIX_FMT_FLAG_BITSTREAM | AV_PIX_FMT_FLAG_HWACCEL | AV_PIX_FMT_FLAG_BAYER | AV_PIX_FMT_FLAG_FLOAT | AV_PIX_FMT_FLAG_XYZ)) != 0)
		return 0;
	if ((desc->flags & AV_PIX_FMT_FLAG_PLANAR)==0)
		return 0;
	if (desc->nb_components < 3)
		return 0;
	if (desc->comp[0].plane != 0 || desc->comp[1].plane != 1 || desc->comp[2].plane != 2)
		return 0;

	return 1;
}

static int ff_pix_fmt_is_gray(int pix_fmt)
{
	const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(pix_fmt);

	// Check component layout
	if (desc==NULL)
		return 0;
	if ((desc->flags & (AV_PIX_FMT_FLAG_RGB | AV_PIX_FMT_FLAG_PAL | AV_PIX_FMT_FLAG_BITSTREAM | AV_PIX_FMT_FLAG_HWACCEL | AV_PIX_FMT_FLAG_BAYER | AV_PIX_FMT_FLAG_FLOAT | AV_PIX_FMT_FLAG_XYZ)) != 0)
		return 0;

	return desc->nb_components == 1;
}

static int ff_pix_fmt_is_yuvj(int pix_fmt)
{
	// Check deprecated full range formats
	switch (pix_fmt)
	{
		case AV_PIX_FMT_YUVJ420P:
		case AV_PIX_FMT_YUVJ422P:
		case AV_PIX_FMT_YUVJ444P:
		case AV_PIX_FMT_YUVJ440P:
		case AV_PIX_FMT_YUVJ411P:
			return 1;
	}

	return 0;
}

static double ff_frame_sample(const uint8_t *data, const int linesize, const int x, const int y, const int bpc, const int big_endian)
{
	const uint8_t *p = &data[y*linesize + x*bpc];

	// Read sample bytes
	if (bpc <= 1)
		return p[0];
	if (bpc == 2)
		return big_endian ? ((uint16_t) p[0] << 8) | p[1] : ((uint16_t) p[1] << 8) | p[0];

	return 0.;
}

static void ff_yuv_coeffs_for_colorspace(const enum AVColorSpace colorspace, double *kr, double *kb)
{
	// Set BT.601 defaults
	*kr = 0.299;
	*kb = 0.114;

	// Select colour matrix
	switch (colorspace)
	{
		case AVCOL_SPC_BT709:
			*kr = 0.2126;
			*kb = 0.0722;
			break;

		case AVCOL_SPC_BT2020_NCL:
		case AVCOL_SPC_BT2020_CL:
			*kr = 0.2627;
			*kb = 0.0593;
			break;

		default:
			break;
	}
}

static int ff_color_transfer_is_unknown(const enum AVColorTransferCharacteristic transfer)
{
	// Check unknown transfer values
	switch (transfer)
	{
		case AVCOL_TRC_RESERVED0:
		case AVCOL_TRC_UNSPECIFIED:
		case AVCOL_TRC_RESERVED:
		case AVCOL_TRC_NB:
			return 1;

		default:
			return 0;
	}
}

static enum AVColorTransferCharacteristic ff_frame_color_transfer(ffstream_t *s)
{
	enum AVColorTransferCharacteristic transfer = s->frame->color_trc;

	// Use codec metadata
	if (ff_color_transfer_is_unknown(transfer) && s->codec_ctx)
		transfer = s->codec_ctx->color_trc;

	// Use stream metadata
	if (ff_color_transfer_is_unknown(transfer) && s->fmt_ctx && s->stream_id >= 0 && s->fmt_ctx->streams[s->stream_id])
		transfer = s->fmt_ctx->streams[s->stream_id]->codecpar->color_trc;

	// Default to video transfer
	if (ff_color_transfer_is_unknown(transfer))
		transfer = AVCOL_TRC_BT709;

	return transfer;
}

static double ff_transfer_bt709_to_linear(double v)
{
	// Apply BT.709 EOTF
	if (v < 0.081)
		return v / 4.5;

	return fastpow((v + 0.099) / 1.099, 1. / 0.45);
}

static double ff_transfer_smpte240m_to_linear(double v)
{
	// Apply SMPTE 240M EOTF
	if (v < 0.091286)
		return v / 4.;

	return fastpow((v + 0.1115) / 1.1115, 1. / 0.45);
}

static double ff_transfer_pq_to_linear(double v)
{
	// Set PQ constants
	const double m1 = 2610. / 16384.;
	const double m2 = 2523. / 32.;
	const double c1 = 3424. / 4096.;
	const double c2 = 2413. / 128.;
	const double c3 = 2392. / 128.;
	double n, d;

	// Clamp endpoints
	if (v <= 0.)
		return 0.;
	if (v >= 1.)
		return 1.;

	// Apply PQ EOTF
	n = fastpow(v, 1. / m2);
	d = MAXN(n - c1, 0.) / (c2 - c3*n);

	return fastpow(d, 1. / m1);
}

static double ff_transfer_hlg_to_linear(double v)
{
	// Set HLG constants
	const double a = 0.17883277;
	const double b = 0.28466892;
	const double c = 0.55991073;

	// Apply HLG EOTF
	if (v <= 0.)
		return 0.;
	if (v >= 1.)
		return 1.;
	if (v <= 0.5)
		return (v*v) / 3.;

	return (exp((v - c) / a) + b) / 12.;
}

static double ff_transfer_log_to_linear(double v)
{
	// Apply log EOTF
	if (v <= 0.)
		return 0.;

	return fastpow(10., 2. * (v - 1.));
}

static double ff_transfer_log_sqrt_to_linear(double v)
{
	// Apply log sqrt EOTF
	if (v <= 0.)
		return 0.;

	return fastpow(10., 2.5 * (v - 1.));
}

static double ff_transfer_to_linear(double v, const enum AVColorTransferCharacteristic transfer)
{
	// Normalise code value
	v *= 1. / 255.;

	// Select transfer curve
	switch (transfer)
	{
		case AVCOL_TRC_LINEAR:
			return v;

		case AVCOL_TRC_IEC61966_2_1:
			return slrgb(v);

		case AVCOL_TRC_GAMMA22:
			if (v < 0.)
				return -fastpow(-v, 2.2);
			return fastpow(v, 2.2);

		case AVCOL_TRC_GAMMA28:
			if (v < 0.)
				return -fastpow(-v, 2.8);
			return fastpow(v, 2.8);

		case AVCOL_TRC_SMPTE240M:
			return ff_transfer_smpte240m_to_linear(v);

		case AVCOL_TRC_LOG:
			return ff_transfer_log_to_linear(v);

		case AVCOL_TRC_LOG_SQRT:
			return ff_transfer_log_sqrt_to_linear(v);

		case AVCOL_TRC_SMPTE2084:
			return ff_transfer_pq_to_linear(v);

		case AVCOL_TRC_SMPTE428:
			if (v < 0.)
				return -fastpow(-v, 2.6);
			return fastpow(v, 2.6);

		case AVCOL_TRC_ARIB_STD_B67:
			return ff_transfer_hlg_to_linear(v);

		case AVCOL_TRC_BT709:
		case AVCOL_TRC_SMPTE170M:
		case AVCOL_TRC_BT2020_10:
		case AVCOL_TRC_BT2020_12:
		case AVCOL_TRC_IEC61966_2_4:
		case AVCOL_TRC_BT1361_ECG:
		default:
			return ff_transfer_bt709_to_linear(v);
	}
}

static frgb_t ff_yuv_to_frgb_sample(double y, double u, double v, const double kr, const double kb, const double kg, const double chroma_mul, const enum AVColorTransferCharacteristic transfer)
{
	frgb_t pv;
	double r, g, b;

	// Centre chroma samples
	u = (u - 128.) * chroma_mul;
	v = (v - 128.) * chroma_mul;

	// Apply YUV matrix
	r = y + 2. * (1. - kr) * v;
	g = y - (2. * kb * (1. - kb) / kg) * u - (2. * kr * (1. - kr) / kg) * v;
	b = y + 2. * (1. - kb) * u;

	// Linearise RGB
	pv.r = ff_transfer_to_linear(r, transfer);
	pv.g = ff_transfer_to_linear(g, transfer);
	pv.b = ff_transfer_to_linear(b, transfer);
	pv.a = 1.;

	return pv;
}

raster_t ff_frame_to_buffer(ffstream_t *s)
{
	int ret, pix_fmt, buf_size;
	raster_t im={0};

	pix_fmt = ff_frame_pix_fmt(s);
	im.buf_fmt = ff_pix_fmt_to_buf_fmt(pix_fmt);
	if (im.buf_fmt == -1)
	{
		fprintf_rl(stderr, "Unsupported FFmpeg pixel format %d for ff_frame_to_buffer()\n", pix_fmt);
		return im;
	}

	im.dim = ff_frame_dim(s);
	buf_size = av_image_get_buffer_size(pix_fmt, im.dim.x, im.dim.y, 1);
	if (buf_size < 0)
	{
		ffmpeg_retval(buf_size);
		return im;
	}

	im.buf_size = buf_size;
	im.buf = malloc(im.buf_size);
	if (im.buf==NULL)
	{
		fprintf_rl(stderr, "malloc(%zu) failed in ff_frame_to_buffer()\n", im.buf_size);
		return im;
	}

	ret = av_image_copy_to_buffer(im.buf, buf_size, (const uint8_t * const *) s->frame->data, s->frame->linesize, pix_fmt, im.dim.x, im.dim.y, 1);
	if (ret < 0)
	{
		ffmpeg_retval(ret);
		free_null(&im.buf);
		im.buf_size = 0;
		im.buf_fmt = 0;
	}

	return im;
}

raster_t ff_frame_to_raster(ffstream_t *s, const int mode)
{
	int bpc, bit_depth, pix_fmt, full_range, big_endian, is_gray, cw_shift, ch_shift;
	enum AVColorTransferCharacteristic transfer;
	raster_t im={0};
	xyi_t ip, dim;
	double y, u, v, kr, kb, kg, chroma_mul, sample_mul, y_mul, y_add;
	const AVPixFmtDescriptor *desc;
	const uint8_t *data_y, *data_u, *data_v;
	int linesize_y, linesize_u, linesize_v, row_out, out_i, chroma_x, chroma_y;

	if (mode != IMAGE_USE_FRGB && mode != IMAGE_USE_SQRGB)
		return im;

	// Read pixel format
	pix_fmt = ff_frame_pix_fmt(s);
	desc = av_pix_fmt_desc_get(pix_fmt);
	if (desc==NULL)
		return im;
	is_gray = ff_pix_fmt_is_gray(pix_fmt);
	if (ff_pix_fmt_is_planar_yuv(pix_fmt)==0 && is_gray==0)
	{
		fprintf_rl(stderr, "Unsupported FFmpeg pixel format %d for ff_frame_to_raster()\n", pix_fmt);
		return im;
	}

	// Prepare parameters and pointers
	bpc = ff_pix_fmt_byte_count(pix_fmt);
	bit_depth = ff_pix_fmt_bit_depth(pix_fmt);
	if (bpc < 1 || bpc > 2 || bit_depth < 1 || bit_depth > 16)
	{
		fprintf_rl(stderr, "Unsupported FFmpeg pixel format depth %d for ff_frame_to_raster()\n", bit_depth);
		return im;
	}

	dim = ff_frame_dim(s);
	im = make_raster(NULL, dim, XYI0, mode);
	if ((mode == IMAGE_USE_FRGB && im.f==NULL) || (mode == IMAGE_USE_SQRGB && im.sq==NULL))
		return im;

	// Read frame metadata
	full_range = s->frame->color_range == AVCOL_RANGE_JPEG || ff_pix_fmt_is_yuvj(pix_fmt);
	big_endian = (desc->flags & AV_PIX_FMT_FLAG_BE) != 0;
	transfer = ff_frame_color_transfer(s);

	// Scale raw samples
	if (bit_depth <= 8)
		sample_mul = 1.;
	else if (full_range)
		sample_mul = 255. / (double) ((1 << bit_depth) - 1);
	else
		sample_mul = 1. / (double) (1 << (bit_depth-8));

	y_mul = sample_mul;
	y_add = 0.;

	// Scale limited luma
	if (full_range==0)
	{
		y_mul *= 255. / 219.;
		y_add = -16. * 255. / 219.;
	}

	data_y = s->frame->data[0];
	linesize_y = s->frame->linesize[0];

	// Convert grayscale pixels
	if (is_gray)
	{
		if (mode == IMAGE_USE_FRGB)
		{
			for (ip.y=0; ip.y < im.dim.y; ip.y++)
			{
				row_out = ip.y * im.dim.x;
				for (ip.x=0; ip.x < im.dim.x; ip.x++)
				{
					frgb_t pv;

					out_i = row_out + ip.x;
					y = ff_frame_sample(data_y, linesize_y, ip.x, ip.y, bpc, big_endian) * y_mul + y_add;
					pv.r = ff_transfer_to_linear(y, transfer);
					pv.g = pv.r;
					pv.b = pv.r;
					pv.a = 1.;
					im.f[out_i] = pv;
				}
			}
		}
		else
		{
			for (ip.y=0; ip.y < im.dim.y; ip.y++)
			{
				row_out = ip.y * im.dim.x;
				for (ip.x=0; ip.x < im.dim.x; ip.x++)
				{
					frgb_t pv;

					out_i = row_out + ip.x;
					y = ff_frame_sample(data_y, linesize_y, ip.x, ip.y, bpc, big_endian) * y_mul + y_add;
					pv.r = ff_transfer_to_linear(y, transfer);
					pv.g = pv.r;
					pv.b = pv.r;
					pv.a = 1.;
					im.sq[out_i] = frgb_to_sqrgb(pv);
				}
			}
		}
	}
	else
	{
		// Prepare chroma planes
		data_u = s->frame->data[1];
		data_v = s->frame->data[2];
		linesize_u = s->frame->linesize[1];
		linesize_v = s->frame->linesize[2];
		cw_shift = desc->log2_chroma_w;
		ch_shift = desc->log2_chroma_h;
		chroma_mul = full_range ? 1. : 255. / 224.;
		ff_yuv_coeffs_for_colorspace(s->frame->colorspace, &kr, &kb);
		kg = 1. - kr - kb;

		// Convert YUV pixels
		if (mode == IMAGE_USE_FRGB)
		{
			for (ip.y=0; ip.y < im.dim.y; ip.y++)
			{
				row_out = ip.y * im.dim.x;
				chroma_y = ip.y >> ch_shift;
				for (ip.x=0; ip.x < im.dim.x; ip.x++)
				{
					out_i = row_out + ip.x;
					chroma_x = ip.x >> cw_shift;
					y = ff_frame_sample(data_y, linesize_y, ip.x, ip.y, bpc, big_endian) * y_mul + y_add;
					u = ff_frame_sample(data_u, linesize_u, chroma_x, chroma_y, bpc, big_endian) * sample_mul;
					v = ff_frame_sample(data_v, linesize_v, chroma_x, chroma_y, bpc, big_endian) * sample_mul;

					im.f[out_i] = ff_yuv_to_frgb_sample(y, u, v, kr, kb, kg, chroma_mul, transfer);
				}
			}
		}
		else
		{
			for (ip.y=0; ip.y < im.dim.y; ip.y++)
			{
				row_out = ip.y * im.dim.x;
				chroma_y = ip.y >> ch_shift;
				for (ip.x=0; ip.x < im.dim.x; ip.x++)
				{
					out_i = row_out + ip.x;
					chroma_x = ip.x >> cw_shift;
					y = ff_frame_sample(data_y, linesize_y, ip.x, ip.y, bpc, big_endian) * y_mul + y_add;
					u = ff_frame_sample(data_u, linesize_u, chroma_x, chroma_y, bpc, big_endian) * sample_mul;
					v = ff_frame_sample(data_v, linesize_v, chroma_x, chroma_y, bpc, big_endian) * sample_mul;

					im.sq[out_i] = frgb_to_sqrgb(ff_yuv_to_frgb_sample(y, u, v, kr, kb, kg, chroma_mul, transfer));
				}
			}
		}
	}

	return im;
}

static int ff_seek_timestamp_flags(ffstream_t *s, double ts, int64_t pts, int flags, int flush)
{
	int ret;

	if (isnan(ts)==0)
		pts = ff_make_timestamp(s, ts);

	ret = av_seek_frame(s->fmt_ctx, s->stream_id, pts, flags);
	ffmpeg_retval(ret);

	if (ret >= 0 && flush)
		ff_flush_stream_decoder(s);

	return ret >= 0;
}

void ff_seek_timestamp(ffstream_t *s, double ts, int64_t pts, int flush)
{
	ff_seek_timestamp_flags(s, ts, pts, AVSEEK_FLAG_BACKWARD, flush);
}

void ff_seek_byte(ffstream_t *s, int64_t pos, int flush)
{
	int64_t ret;

	//seek_frame_byte(s->fmt_ctx, s->stream_id, pos, 0);

	//ret = av_seek_frame(s->fmt_ctx, s->stream_id, pos, AVSEEK_FLAG_BYTE);
	ret = avio_seek(s->fmt_ctx->pb, pos, SEEK_SET);
	if (ret < 0)
		ffmpeg_retval((int) ret);

	if (ret >= 0 && flush)
	{
		if (s->packet_pending)
		{
			av_packet_unref(s->packet);
			s->packet_pending = 0;
		}
		avio_flush(s->fmt_ctx->pb);
		avformat_flush(s->fmt_ctx);
		ff_flush_stream_decoder(s);
	}
}

int ff_find_table_frame_id(ffstream_t *s, double t, const int keyframe)
{
	int i, frame_id, seek_id;
	double target_ts;

	if (s->frame_info==NULL || s->frame_count <= 0)
		return -1;

	// check if t isn't after the expected end
	if (t > s->frame_info[s->frame_count-1].ts_end)
		return -1;

	frame_id = -1;
	for (i=0; i < s->frame_count; i++)
		if (t >= s->frame_info[i].ts)
		{
			if (s->frame_info[i].key_frame || keyframe==0)
				frame_id = i;
		}
		else
			break ;

	if (frame_id > -1)
	{
		seek_id = frame_id;
		if (keyframe==0)
			for (i=frame_id; i >= 0; i--)
				if (s->frame_info[i].key_frame)
				{
					seek_id = i;
					break ;
				}

		if (ff_seek_timestamp_flags(s, NAN, s->frame_info[seek_id].pts, AVSEEK_FLAG_BACKWARD, 1)==0)
			return -1;
		//ff_seek_byte(s, s->frame_info[frame_id].pkt_pos, keyframe);

		if (ff_load_stream_packet(s)==0)		// get the sought frame
			return -1;

		if (keyframe==0)
		{
			target_ts = s->frame_info[frame_id].ts;
			while (ff_get_frame_timestamp(s) < target_ts)
				if (ff_load_stream_packet(s)==0)
					return -1;
		}

		if (s->frame_info[frame_id].ts != ff_get_frame_timestamp(s))
			fprintf_rl(stdout, "\tSought PTS %.3f s - Found PTS %.3f sec\n", s->frame_info[frame_id].ts, ff_get_frame_timestamp(s));
	}

	return frame_id;
}

int ff_decode_frame_from_table(ffstream_t *s, double t)
{
	if (s->frame_info==NULL)
		return 0;

	return ff_find_table_frame_id(s, t, 0) > -1;
}

int ff_find_keyframe_for_time(ffstream_t *s, const double t)
{
	AVFrame *f;
	int64_t frame_ts, timestamp, timestamp_o=-1;
	double seek_offset=0.;

	if (s->frame_info)
		if (ff_find_table_frame_id(s, t, 1) > -1)
			return 1;

	if (t < 2.)
	{
		//ff_seek_byte(s, 0, 1);
		if (ff_seek_timestamp_flags(s, 0., 0, AVSEEK_FLAG_BACKWARD, 1)==0)
			return 0;

		if (ff_load_stream_packet(s))		// get the sought frame
			return 1;
		else
			return 0;
	}

	// initial seek
seek_start:
	timestamp = ff_make_timestamp(s, t+seek_offset);	// FIXME timestamp can't be before the start
	if (timestamp_o==-1)
		timestamp_o = timestamp;
	if (ff_seek_timestamp_flags(s, NAN, timestamp, AVSEEK_FLAG_BACKWARD, 1)==0)
		return 0;

	if (ff_load_stream_packet(s))		// get the sought frame
	{
		f = s->frame;
		frame_ts = ff_frame_timestamp(f);
		if (frame_ts != AV_NOPTS_VALUE && frame_ts > timestamp_o)
		{
			seek_offset -= 1.;	// seek one second further back
			if (t+seek_offset <= -2.)
				return 0;

			//fprintf_rl(stdout, "The found keyframe is %g seconds late, seeking %g seconds back\n", ff_get_timestamp(s, s->frame->best_effort_timestamp) - ff_get_timestamp(s, timestamp_o), seek_offset);
			goto seek_start;	// go back to the start of the seeking
		}

		//fprintf_rl(stdout, "%d x %d, format %d, type %d\ttimestamp %5.3f s, number %d%s\n", f->width, f->height, f->format, f->pict_type, ff_get_frame_timestamp(s), f->coded_picture_number, f->key_frame ? ", KEYFRAME" : "");
	}
	else
		return 0;

	return 1;
}

int ff_find_frame_at_time(ffstream_t *s, const double t)	// finds and decode properly the right frame at a given time (can sometimes be one frame late)
{
	AVFrame *f;
	int64_t frame_ts, duration;

	if (ff_find_keyframe_for_time(s, t))		// find the preceding keyframe
	{
		do
		{
			f = s->frame;
			frame_ts = ff_frame_timestamp(f);
			duration = ff_frame_duration_ts(s, f);
			if (frame_ts != AV_NOPTS_VALUE && duration > 0 && ff_get_timestamp(s, frame_ts + duration) > t)	// if the next frame would be after t
				return 1;								// caveat: frame duration can fail to accurately predict the next timestamp
		}
		while (ff_load_stream_packet(s));
	}

	return 0;
}

static int ff_frame_time_bounds(ffstream_t *s, AVFrame *frame, double *ts, double *ts_end)
{
	int64_t frame_ts, duration;

	// Read frame timestamp
	frame_ts = ff_frame_timestamp(frame);
	if (frame_ts == AV_NOPTS_VALUE)
		return 0;

	*ts = ff_get_timestamp(s, frame_ts);
	if (isfinite(*ts)==0)
		return 0;

	// Read frame end timestamp
	duration = ff_frame_duration_ts(s, frame);
	if (duration > 0)
		*ts_end = ff_get_timestamp(s, frame_ts + duration);
	else
		*ts_end = *ts;

	if (*ts_end <= *ts && frame->nb_samples > 0 && s->codec_ctx->sample_rate > 0)
		*ts_end = *ts + (double) frame->nb_samples / (double) s->codec_ctx->sample_rate;

	return 1;
}

static int ff_find_audio_frame_at_time(ffstream_t *s, const double t)
{
	int attempt, decoded;
	double seek_t, backoff=0.05, frame_ts, frame_ts_end;

	// Seek directly to audio packets
	for (attempt=0; attempt < 4; attempt++)
	{
		seek_t = MAXN(0., t - (attempt ? backoff : 0.));
		if (ff_seek_timestamp_flags(s, seek_t, 0, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_ANY, 1)==0)
			return 0;

		// Decode until the target frame
		for (decoded=0; decoded < 256; decoded++)
		{
			if (ff_load_stream_packet(s)==0)
				return 0;
			if (ff_frame_time_bounds(s, s->frame, &frame_ts, &frame_ts_end)==0)
				return 1;

			if (frame_ts <= t && frame_ts_end > t)
				return 1;
			if (frame_ts > t)
				break;
		}

		backoff *= 4.;
	}

	return 0;
}

ffframe_info_t ff_make_frame_info(ffstream_t *s)
{
	ffframe_info_t fi={0};
	int64_t duration;

	if (s->frame==NULL)
		return fi;

	fi.key_frame = ff_frame_key_frame(s->frame);
	fi.pkt_pos = ff_frame_pkt_pos(s);
	fi.pts = ff_frame_timestamp(s->frame);
	fi.ts = ff_get_timestamp(s, fi.pts);
	duration = ff_frame_duration_ts(s, s->frame);
	if (fi.pts != AV_NOPTS_VALUE && duration > 0)
		fi.ts_end = ff_get_timestamp(s, fi.pts + duration);
	else
		fi.ts_end = fi.ts;

	return fi;
}

double ff_get_stream_duration(ffstream_t *s, const char *path, int stream_type)
{
	double duration;
	AVStream *st;

	if (s==NULL)		// allows finding the duration even if s is NULL
	{
		ffstream_t stream={0};
		duration = ff_get_stream_duration(&stream, path, stream_type);
		ffstream_close_free(&stream);
		return duration;
	}

	if (s->fmt_ctx==NULL)
		*s = ff_load_stream_init(path, stream_type, 1);

	// Reject incomplete stream states before accessing FFmpeg data
	if (s->fmt_ctx==NULL || s->fmt_ctx->streams==NULL || s->stream_id < 0 || (unsigned int) s->stream_id >= s->fmt_ctx->nb_streams)
	{
		fprintf_rl(stderr, "Invalid FFmpeg stream state while reading duration for '%s'\n", path ? path : "(unknown)");
		return NAN;
	}

	st = s->fmt_ctx->streams[s->stream_id];

	// Reject missing stream entries before reading their duration
	if (st==NULL)
	{
		fprintf_rl(stderr, "Missing FFmpeg stream %d while reading duration for '%s'\n", s->stream_id, path ? path : "(unknown)");
		return NAN;
	}

	if (st->duration != AV_NOPTS_VALUE)
		return (double) st->duration * av_q2d(st->time_base);

	if (s->fmt_ctx->duration == AV_NOPTS_VALUE)
		return NAN;

	return (double) s->fmt_ctx->duration / (double) AV_TIME_BASE;
}

double ff_get_video_duration(ffstream_t *s, const char *path)
{
	return ff_get_stream_duration(s, path, AVMEDIA_TYPE_VIDEO);
}

double ff_get_audio_duration(ffstream_t *s, const char *path)
{
	return ff_get_stream_duration(s, path, AVMEDIA_TYPE_AUDIO);
}

raster_t ff_load_video_raster(ffstream_t *s, const char *path, const int seek_mode, const double t, const int raster_mode)
{
	int ret=0;
	raster_t im={0};

	// Init
	if (s->fmt_ctx==NULL)
	{
		*s = ff_load_stream_init(path, AVMEDIA_TYPE_VIDEO, 1);
		if (s->stream_id == -1)
			return im;
	}

	if (s->stream_id == -1)
		return im;

	// Load AVFrame
	switch (seek_mode)
	{
		case 0:		// next frame
			ret = ff_load_stream_packet(s);
			break;

		case 1:		// frame at time
			ret = ff_find_frame_at_time(s, t);
			break;

		case 2:		// keyframe before time
			ret = ff_find_keyframe_for_time(s, t);
			break;
	}

	// Convert frame data
	if (ret)
	{
		if (raster_mode & IMAGE_USE_BUF)
			im = ff_frame_to_buffer(s);
		else
			im = ff_frame_to_raster(s, raster_mode);
	}

	return im;
}

// Audio

int ff_load_audio_fl32(ffstream_t *s, const char *path, const int seek_mode, const double t, float **bufp, size_t *buf_as, size_t *buf_pos)
{
	int i, ic, ret=0;
	size_t buf_size;

	// Init
	if (s->fmt_ctx==NULL)
	{
		*s = ff_load_stream_init(path, AVMEDIA_TYPE_AUDIO, 1);
		if (s->stream_id == -1)
			return -1;

		AVDictionaryEntry *t = NULL;
		while ((t = av_dict_get(s->fmt_ctx->metadata, "", t, AV_DICT_IGNORE_SUFFIX)))
			fprintf_rl(stdout, "%s: %s\n", t->key, t->value);
	}

	if (s->stream_id == -1)
		return -1;

	// Load AVFrame
	switch (seek_mode)
	{
		case 0:		// next frame
			ret = ff_load_stream_packet(s);
			break;

		case 1:		// frame at time
			ret = ff_find_audio_frame_at_time(s, t);
			break;
	}

	// Convert frame data
	if (ret)
	{
		int channels = s->frame->ch_layout.nb_channels;
		uint8_t **data = s->frame->extended_data ? s->frame->extended_data : s->frame->data;
		size_t frame_sample_count;

		if (channels <= 0 || s->frame->nb_samples <= 0)
			return 0;

		frame_sample_count = (size_t) s->frame->nb_samples * (size_t) channels;
		buf_size = *buf_pos + frame_sample_count;
		alloc_enough(bufp, buf_size, buf_as, sizeof(float), 1.5);

		// Convert/copy samples
		switch (s->codec_ctx->sample_fmt)
		{
			case AV_SAMPLE_FMT_U8:
				for (size_t is=0; is < frame_sample_count; is++)
					(*bufp)[*buf_pos + is] = ((int) data[0][is] - 128) * 1.f/128.f;
				break;

			case AV_SAMPLE_FMT_U8P:
				for (i=0; i < s->frame->nb_samples; i++)
					for (ic=0; ic < channels; ic++)
						(*bufp)[*buf_pos + i*channels + ic] = ((int) data[ic][i] - 128) * 1.f/128.f;
				break;

			case AV_SAMPLE_FMT_S16:
				for (size_t is=0; is < frame_sample_count; is++)
					(*bufp)[*buf_pos + is] = ((int16_t *)data[0])[is] * 1.f/32768.f;
				break;

			case AV_SAMPLE_FMT_S16P:
				for (i=0; i < s->frame->nb_samples; i++)
					for (ic=0; ic < channels; ic++)
						(*bufp)[*buf_pos + i*channels + ic] = ((int16_t *)data[ic])[i] * 1.f/32768.f;
				break;

			case AV_SAMPLE_FMT_S32:
				for (size_t is=0; is < frame_sample_count; is++)
					(*bufp)[*buf_pos + is] = ((int32_t *)data[0])[is] * 1.f/2147483648.f;
				break;

			case AV_SAMPLE_FMT_S32P:
				for (i=0; i < s->frame->nb_samples; i++)
					for (ic=0; ic < channels; ic++)
						(*bufp)[*buf_pos + i*channels + ic] = ((int32_t *)data[ic])[i] * 1.f/2147483648.f;
				break;

			case AV_SAMPLE_FMT_S64:
				for (size_t is=0; is < frame_sample_count; is++)
					(*bufp)[*buf_pos + is] = ((int64_t *)data[0])[is] * (1. / 9223372036854775808.);
				break;

			case AV_SAMPLE_FMT_S64P:
				for (i=0; i < s->frame->nb_samples; i++)
					for (ic=0; ic < channels; ic++)
						(*bufp)[*buf_pos + i*channels + ic] = ((int64_t *)data[ic])[i] * (1. / 9223372036854775808.);
				break;

			case AV_SAMPLE_FMT_FLT:
				memcpy(&(*bufp)[*buf_pos], data[0], frame_sample_count * sizeof(float));
				break;

			case AV_SAMPLE_FMT_FLTP:
				for (i=0; i < s->frame->nb_samples; i++)
					for (ic=0; ic < channels; ic++)
						(*bufp)[*buf_pos + i*channels + ic] = ((float *)data[ic])[i];
				break;

			case AV_SAMPLE_FMT_DBL:
				for (size_t is=0; is < frame_sample_count; is++)
					(*bufp)[*buf_pos + is] = ((double *)data[0])[is];
				break;

			case AV_SAMPLE_FMT_DBLP:
				for (i=0; i < s->frame->nb_samples; i++)
					for (ic=0; ic < channels; ic++)
						(*bufp)[*buf_pos + i*channels + ic] = ((double *)data[ic])[i];
				break;

			case AV_SAMPLE_FMT_NONE:
			case AV_SAMPLE_FMT_NB:
				fprintf_rl(stderr, "Unsupported FFmpeg sample format %d in ff_load_audio_fl32()\n", s->codec_ctx->sample_fmt);
				return -1;
		}

		*buf_pos = buf_size;
		return frame_sample_count > INT_MAX ? INT_MAX : (int) frame_sample_count;
	}

	return -1;
}

float *ff_load_audio_fl32_full(const char *path, size_t *sample_count, int *channels, int *samplerate)
{
	int ret=0, got_info=0;
	ffstream_t s={0};
	float *buf=NULL;
	size_t buf_as=0, buf_pos=0;

	*sample_count = 0;
	*channels = 0;
	*samplerate = 0;

	while (ret > -1)
	{
		ret = ff_load_audio_fl32(&s, path, 0, NAN, &buf, &buf_as, &buf_pos);

		if (ret > -1)
		{
			*channels = s.frame->ch_layout.nb_channels;
			*samplerate = s.codec_ctx->sample_rate;
			got_info = *channels > 0;
		}
	}

	if (got_info)
		*sample_count = buf_pos / *channels;

	ffstream_close_free(&s);

	return buf;
}

#endif
