#ifdef RL_FFMPEG

// Define mix modes
#define AUDIO_PLAYER_MIX_SILENT		0
#define AUDIO_PLAYER_MIX_MONO		1
#define AUDIO_PLAYER_MIX_STEREO		2
#define AUDIO_PLAYER_MIX_DOWNMIX	3

static void audio_player_clear_frames_no_lock(audio_player_data_t *data)
{
	int i;

	// Clear decoded frames
	if (data->frame==NULL)
		return ;

	for (i=0; i < data->frame_as; i++)
	{
		free(data->frame[i].buffer);
		memset(&data->frame[i], 0, sizeof(audframe_t));
	}

	data->ifr = -1;
	data->is = -1;
}

static int audio_player_frame_sample_pos(audframe_t *frame, double t, double *sample_posp)
{
	double sample_pos;

	// Check frame validity
	if (frame==NULL || frame->used==0 || frame->buffer==NULL || frame->samplerate <= 0. || frame->sample_count <= 0 || frame->channels <= 0)
		return 0;

	// Calculate sample position
	sample_pos = (t - frame->info.ts) * frame->samplerate;
	if (sample_pos < -1e-6 || sample_pos >= (double) frame->sample_count - 1e-6)
		return 0;

	*sample_posp = rangelimit(sample_pos, 0., (double) frame->sample_count - 1.);

	return 1;
}

static int audio_player_find_frame_no_lock(audio_player_data_t *data, double t, double *sample_posp)
{
	int ifr;

	// Try current frame
	if (data->ifr >= 0 && data->ifr < data->frame_as)
		if (audio_player_frame_sample_pos(&data->frame[data->ifr], t, sample_posp))
			return data->ifr;

	// Search buffered frames
	for (ifr=0; ifr < data->frame_as; ifr++)
		if (audio_player_frame_sample_pos(&data->frame[ifr], t, sample_posp))
			return ifr;

	return -1;
}

static int audio_player_layout_channel_index(const AVChannelLayout *layout, int channels, enum AVChannel channel)
{
	int index;
	AVChannelLayout default_layout={0};

	// Search stream layout
	if (layout && layout->nb_channels > 0)
	{
		index = av_channel_layout_index_from_channel(layout, channel);
		if (layout->order != AV_CHANNEL_ORDER_UNSPEC)
			return index >= 0 ? index : -1;
	}

	// Search default layout
	if (channels > 0)
	{
		av_channel_layout_default(&default_layout, channels);
		index = av_channel_layout_index_from_channel(&default_layout, channel);
		av_channel_layout_uninit(&default_layout);
		if (index >= 0)
			return index;
	}

	return -1;
}

static void audio_player_set_channel_map(audframe_t *frame, AVFrame *avframe)
{
	int channels = frame->channels;

	// Clear channel map
	frame->mix_mode = AUDIO_PLAYER_MIX_SILENT;
	frame->mix_l = frame->mix_r = -1;
	frame->ch_fl = frame->ch_fr = frame->ch_fc = frame->ch_lfe = -1;
	frame->ch_bl = frame->ch_br = frame->ch_bc = frame->ch_sl = frame->ch_sr = -1;
	frame->ch_stl = frame->ch_str = -1;

	// Map named channels
	frame->ch_fl = audio_player_layout_channel_index(&avframe->ch_layout, channels, AV_CHAN_FRONT_LEFT);
	frame->ch_fr = audio_player_layout_channel_index(&avframe->ch_layout, channels, AV_CHAN_FRONT_RIGHT);
	frame->ch_fc = audio_player_layout_channel_index(&avframe->ch_layout, channels, AV_CHAN_FRONT_CENTER);
	frame->ch_lfe = audio_player_layout_channel_index(&avframe->ch_layout, channels, AV_CHAN_LOW_FREQUENCY);
	frame->ch_bl = audio_player_layout_channel_index(&avframe->ch_layout, channels, AV_CHAN_BACK_LEFT);
	frame->ch_br = audio_player_layout_channel_index(&avframe->ch_layout, channels, AV_CHAN_BACK_RIGHT);
	frame->ch_bc = audio_player_layout_channel_index(&avframe->ch_layout, channels, AV_CHAN_BACK_CENTER);
	frame->ch_sl = audio_player_layout_channel_index(&avframe->ch_layout, channels, AV_CHAN_SIDE_LEFT);
	frame->ch_sr = audio_player_layout_channel_index(&avframe->ch_layout, channels, AV_CHAN_SIDE_RIGHT);
	frame->ch_stl = audio_player_layout_channel_index(&avframe->ch_layout, channels, AV_CHAN_STEREO_LEFT);
	frame->ch_str = audio_player_layout_channel_index(&avframe->ch_layout, channels, AV_CHAN_STEREO_RIGHT);

	// Fallback to stream order
	if (channels == 1 && frame->ch_fc < 0)
		frame->ch_fc = 0;
	if (channels >= 2)
	{
		if (frame->ch_fl < 0)
			frame->ch_fl = 0;
		if (frame->ch_fr < 0)
			frame->ch_fr = 1;
	}

	// Select mono mix
	if (channels == 1)
	{
		frame->mix_mode = AUDIO_PLAYER_MIX_MONO;
		frame->mix_l = frame->ch_fc;
		frame->mix_r = frame->ch_fc;
		return ;
	}

	// Select stereo mix
	if (frame->ch_stl >= 0 && frame->ch_str >= 0)
	{
		frame->mix_mode = AUDIO_PLAYER_MIX_STEREO;
		frame->mix_l = frame->ch_stl;
		frame->mix_r = frame->ch_str;
		return ;
	}

	// Select ordered stereo mix
	if (channels == 2)
	{
		frame->mix_mode = AUDIO_PLAYER_MIX_STEREO;
		frame->mix_l = frame->ch_fl;
		frame->mix_r = frame->ch_fr;
		return ;
	}

	// Select surround downmix
	if (channels > 2)
	{
		frame->mix_mode = AUDIO_PLAYER_MIX_DOWNMIX;
		frame->mix_l = frame->ch_fl >= 0 ? frame->ch_fl : 0;
		frame->mix_r = frame->ch_fr >= 0 ? frame->ch_fr : MINN(1, channels-1);
	}
}

static int audio_player_add_downmix_channel(audframe_t *frame, int is, int channel, double left_weight, double right_weight, double *left, double *right, double *left_weight_sum, double *right_weight_sum)
{
	float sample;

	// Add mapped channel
	if (channel < 0 || channel >= frame->channels)
		return 0;

	sample = frame->buffer[is*frame->channels + channel];
	*left += (double) sample * left_weight;
	*right += (double) sample * right_weight;
	*left_weight_sum += left_weight;
	*right_weight_sum += right_weight;

	return 1;
}

static void audio_player_downmix_frame_sample(audframe_t *frame, int is, float *left, float *right)
{
	int mixed=0;
	double l=0., r=0., left_weight_sum=0., right_weight_sum=0.;
	const double surround_gain = M_SQRT1_2;
	const double center_gain = M_SQRT1_2;
	const double back_center_gain = 0.5;

	// Downmix surround channels
	mixed |= audio_player_add_downmix_channel(frame, is, frame->ch_fl, 1., 0., &l, &r, &left_weight_sum, &right_weight_sum);
	mixed |= audio_player_add_downmix_channel(frame, is, frame->ch_fr, 0., 1., &l, &r, &left_weight_sum, &right_weight_sum);
	mixed |= audio_player_add_downmix_channel(frame, is, frame->ch_fc, center_gain, center_gain, &l, &r, &left_weight_sum, &right_weight_sum);
	mixed |= audio_player_add_downmix_channel(frame, is, frame->ch_bl, surround_gain, 0., &l, &r, &left_weight_sum, &right_weight_sum);
	mixed |= audio_player_add_downmix_channel(frame, is, frame->ch_br, 0., surround_gain, &l, &r, &left_weight_sum, &right_weight_sum);
	mixed |= audio_player_add_downmix_channel(frame, is, frame->ch_sl, surround_gain, 0., &l, &r, &left_weight_sum, &right_weight_sum);
	mixed |= audio_player_add_downmix_channel(frame, is, frame->ch_sr, 0., surround_gain, &l, &r, &left_weight_sum, &right_weight_sum);
	mixed |= audio_player_add_downmix_channel(frame, is, frame->ch_bc, back_center_gain, back_center_gain, &l, &r, &left_weight_sum, &right_weight_sum);

	// Fallback to first channels
	if (mixed==0)
	{
		*left = frame->buffer[is*frame->channels + frame->mix_l];
		*right = frame->buffer[is*frame->channels + frame->mix_r];
		return ;
	}

	// Normalise downmix
	if (left_weight_sum > 1.)
		l /= left_weight_sum;
	if (right_weight_sum > 1.)
		r /= right_weight_sum;

	*left = l;
	*right = r;
}

static void audio_player_read_frame_sample(audframe_t *frame, double sample_pos, float *left, float *right)
{
	int is0, is1, base0, base1;
	double frac;
	float left0, right0, left1, right1;

	// Select interpolation samples
	is0 = (int) floor(sample_pos);
	is1 = is0 + 1;
	frac = sample_pos - (double) is0;

	if (is1 >= frame->sample_count)
	{
		is1 = is0;
		frac = 0.;
	}

	// Interpolate source sample
	base0 = is0 * frame->channels;
	base1 = is1 * frame->channels;
	switch (frame->mix_mode)
	{
		case AUDIO_PLAYER_MIX_MONO:
			left0 = right0 = frame->buffer[base0 + frame->mix_l];
			left1 = right1 = frame->buffer[base1 + frame->mix_l];
			break ;

		case AUDIO_PLAYER_MIX_STEREO:
			left0 = frame->buffer[base0 + frame->mix_l];
			right0 = frame->buffer[base0 + frame->mix_r];
			left1 = frame->buffer[base1 + frame->mix_l];
			right1 = frame->buffer[base1 + frame->mix_r];
			break ;

		case AUDIO_PLAYER_MIX_DOWNMIX:
			audio_player_downmix_frame_sample(frame, is0, &left0, &right0);
			audio_player_downmix_frame_sample(frame, is1, &left1, &right1);
			break ;

		default:
			left0 = right0 = left1 = right1 = 0.f;
			break ;
	}

	*left = mix(left0, left1, frac);
	*right = mix(right0, right1, frac);
}

static double audio_player_seek_preroll()
{
	double preroll;

	// Calculate buffering margin
	preroll = audiosys.sec_per_buf * 2. + 0.05;
	return rangelimit(preroll, 0.05, 0.25);
}

int audio_player_load_thread(audio_player_data_t *data)
{
	int ip=-1, must_seek=0, ip0=-1, ip1=-1, prev_ip;
	double ts_req=0., ts0=NAN, ts0_end=0., ts1=NAN, speed=1.;
	ffframe_info_t info;
	int ret=0, sample_count;
	float *buf=NULL;
	size_t buf_as=0, buf_pos=0;

	while (data->thread_on)
	{
loop_start:
		rl_mutex_lock(&data->mutex);

		// Read playback state
		speed = data->speed;
		ts_req = data->ts_req;

		// Seek on discontinuities
		if (speed < 0. || ts_req < ts0 || (ts_req > ts1 && isnan(ts1)==0) || (ts_req > 0. && isnan(ts1)))
		{
			ip = -1;
			ts0 = NAN;
			ts0_end = 0.;
			ts1 = NAN;
			must_seek = 1;
			ip0 = -1;
			ip1 = -1;
			audio_player_clear_frames_no_lock(data);
		}

		rl_mutex_unlock(&data->mutex);

		// Pause the loading if the frame to be replaced next is still needed
		if (ts_req >= ts0 && ts_req < ts0_end && must_seek==0 && data->thread_on && circ_index(ip+1, data->frame_as)==ip0)
		{
			sleep_ms(1);
			goto loop_start;
		}

		ret = 0;
		sample_count = 0;

		if (llabs(double_diff_ulp(speed, 1.)) < 100)
		{
			// Load the frame, either by seeking or sequentially
			buf_pos = 0;
			ret = ff_load_audio_fl32(data->stream, data->path, must_seek, ts_req, &buf, &buf_as, &buf_pos);
			if (ret >= 0)
				sample_count = data->stream->frame->nb_samples;
		}

		if (ret <= 0)
			sleep_ms(20);		// if the frame was empty just wait
		else
		{
			info = ff_make_frame_info(data->stream);

			rl_mutex_lock(&data->mutex);

			// Add the frame to the tables
			prev_ip = ip1;
			ip = circ_index(ip + 1, data->frame_as);

			// if new frame replaces old frame[ip0]
			if (ip==ip0)
			{
				ip0 = circ_index(ip + 1, data->frame_as);
				ts0 = data->frame[ip0].info.ts;
				ts0_end = data->frame[ip0].info.ts_end;
			}

			if (isnan(ts0))			// init ts0
			{
				ip0 = ip;
				ts0 = MINN(ts_req, info.ts);	// in case info.ts is higher than ts_req
				ts0_end = info.ts_end;
			}
			ts1 = info.ts_end;
			ip1 = ip;

			data->frame[ip].used = 1;
			data->frame[ip].sample_count = sample_count;
			data->frame[ip].channels = data->stream->frame->ch_layout.nb_channels;
			data->frame[ip].samplerate = data->stream->codec_ctx->sample_rate;
			audio_player_set_channel_map(&data->frame[ip], data->stream->frame);
			alloc_enough_and_copy(&data->frame[ip].buffer, buf, data->frame[ip].len = ret, &data->frame[ip].as, sizeof(float), 1.);
			data->frame[ip].info = info;

			if (must_seek==0 && prev_ip > -1)
				data->frame[ip].info.ts = data->frame[prev_ip].info.ts_end;
			data->frame[ip].info.ts_end = data->frame[ip].info.ts + (double) sample_count / data->frame[ip].samplerate;

			rl_mutex_unlock(&data->mutex);
		}

		must_seek = 0;
	}

	ffstream_close_free(data->stream);

	return 0;
}

void audio_player_thread_exit(audio_player_data_t *data)
{
	rl_mutex_lock(&data->mutex);

	audio_player_clear_frames_no_lock(data);

	rl_mutex_unlock(&data->mutex);
}

void audio_player_main(audio_player_data_t *data, char *path, double ts_req, double speed, double volume)
{
	int start_thread=1, stop_thread=1, same_path=0, jump=0;

	// Close the old stream and open the new one
	if (path && data->path)
	if (strcmp(path, data->path)==0)
	{
		same_path = 1;
		start_thread = 0;
		stop_thread = 0;
	}

	if (path==NULL)
		start_thread = 0;

	// Init the data
	if (data->init==0)
	{
		data->init = 1;
		rl_mutex_init(&data->mutex);
		data->frame = calloc(data->frame_as = 600, sizeof(audframe_t));
		data->stream = calloc(1, sizeof(ffstream_t));
	}

	// Restart expired stream
	if (same_path && data->thread_on==0)
		start_thread = 1;

	if (stop_thread)
	{
		data->thread_on = 0;
		audiosys_bus_unregister(data);
		audio_player_thread_exit(data);
		free_null(&data->path);
	}

	if (start_thread)
	{
		// Finish stopped thread
		rl_thread_join_and_null(&data->thread_handle);

		// Initialise new path
		if (same_path==0)
		{
			data->path = make_string_copy(path);
			data->duration = ff_get_audio_duration(NULL, data->path);
		}

		// Initialise playback state
		audio_player_clear_frames_no_lock(data);
		data->ts_req = rangelimit(ts_req - audio_player_seek_preroll(), 0., data->duration);
		data->speed = speed;
		data->volume = volume;
		data->ifr = -1;
		data->is = -1;
		data->thread_on = 1;
		if (rl_thread_create(&data->thread_handle, audio_player_load_thread, data)==0)
			data->thread_on = 0;
	}

	// Update values
	rl_mutex_lock(&data->mutex);

	if (fabs(data->ts_cb - ts_req) > 2. * audiosys.sec_per_buf + 0.05)	// FIXME calculate whether to jump or not better
		jump = 1;

	if (jump)
		fprintf_rl(stdout, "jump to ts %.4f\n", ts_req);

	// Set the time offset
	if (jump || start_thread)
	{
		data->ts_cb = ts_req;
		data->ifr = -1;
		data->is = -1;
	}
	//data->time_offset = get_time_hr() - ts_req;

	data->ts_req = rangelimit(ts_req - audio_player_seek_preroll(), 0., data->duration);
	data->speed = speed;
	data->volume = volume;

	rl_mutex_unlock(&data->mutex);

	// Register the callback
	if (data->thread_on)
		audiosys_bus_register(audio_player_callback, data, 0, 0.);
}

void audio_player_callback(float *stream, audiosys_t *sys, int bus_index, audio_player_data_t *data)
{
	int i, ifr;
	float left, right;
	double t, ibl, vol_t, sample_pos;
	int debug=1;

	// Deinit
	if (stream==NULL)
	{
		data->thread_on = 0;

		if (bus_index == -1)	// this signals a blocking deinitialisation
			rl_thread_join_and_null(&data->thread_handle);

		data->vol1 = NAN;
		return;
	}

	// Only play if speed == 1 and thread_on
	if (fabs(data->speed - 1.) >= 2e-14 || data->thread_on==0)
	{
		data->vol1 = NAN;
		return;
	}

	rl_mutex_lock(&data->mutex);

	// Prepare volume interpolation
	data->vol0 = data->vol1;
	if (isnan(data->vol0))
		data->vol0 = data->volume;
	data->vol1 = data->volume;
	ibl = 1. / (double) sys->buffer_len;

	// Go through each sample
	for (t=data->ts_cb, i=0; i < sys->buffer_len; i++, t+=sys->sec_per_sample)
	{
		// Find source sample
		data->ifr = audio_player_find_frame_no_lock(data, t, &sample_pos);
		data->is = data->ifr==-1 ? -1 : (int) floor(sample_pos + 1e-6);

		// Interpolate volume
		vol_t = mix(data->vol0, data->vol1, (double) i * ibl);
		//if (data->vol0 != data->vol1 && ((i & 0xFF) == 0 || i == sys->buffer_len -1)) fprintf_rl(stdout, "mix(%.6f , %.6f, %.6f) = %.6f (%4d/%4d)\n", data->vol0, data->vol1, (double) i * ibl, vol_t, i, sys->buffer_len);

		// Copy interpolated sample
		if (data->ifr != -1)
		{
			audio_player_read_frame_sample(&data->frame[data->ifr], sample_pos, &left, &right);
			stream[i*2  ] += left * vol_t;
			stream[i*2+1] += right * vol_t;
		}
		else if (debug)
		{
			double min_ts=1e9, max_ts=-1.;
			int used_count=0;

			debug = 0;
			for (ifr=0; ifr < data->frame_as; ifr++)
			{
				if (data->frame[ifr].used==0)
					continue;

				used_count++;
				min_ts = MINN(min_ts, data->frame[ifr].info.ts);
				max_ts = MAXN(max_ts, data->frame[ifr].info.ts_end);
			}
			if (used_count > 0)
				fprintf_rl(stdout, "No sample for ts %.4f (ts available: %.4f to %.4f)\n", t, min_ts, max_ts);
		}
	}
	data->ts_cb += sys->sec_per_buf;

	rl_mutex_unlock(&data->mutex);
}

#endif
