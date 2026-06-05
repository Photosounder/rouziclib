#ifdef RL_FFMPEG

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

static float audio_player_frame_channel_sample(audframe_t *frame, int is, int channel)
{
	// Read mono sample
	if (frame->channels <= 1)
		return frame->buffer[is];

	// Read selected channel
	channel = MINN(channel, frame->channels-1);
	return frame->buffer[is*frame->channels + channel];
}

static void audio_player_read_frame_sample(audframe_t *frame, double sample_pos, float *left, float *right)
{
	int is0, is1;
	double frac;

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
	*left = mix(audio_player_frame_channel_sample(frame, is0, 0), audio_player_frame_channel_sample(frame, is1, 0), frac);
	*right = mix(audio_player_frame_channel_sample(frame, is0, 1), audio_player_frame_channel_sample(frame, is1, 1), frac);
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
	int start_thread=1, stop_thread=1, jump=0;

	// Close the old stream and open the new one
	if (path && data->path)
	if (strcmp(path, data->path)==0)
	{
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

	if (stop_thread)
	{
		data->thread_on = 0;
		audiosys_bus_unregister(data);
		audio_player_thread_exit(data);
		free_null(&data->path);
	}

	if (start_thread)
	{
		// Initialise new data
		data->path = make_string_copy(path);
		data->duration = ff_get_audio_duration(NULL, data->path);
		data->ts_req = 0.;
		data->ifr = -1;
		data->is = -1;
		data->thread_on = 1;
		rl_thread_create(&data->thread_handle, audio_player_load_thread, data);
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
