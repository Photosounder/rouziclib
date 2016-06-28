#ifdef RL_FFMPEG

static char *const ffmpeg_get_error_text(const int error)
{
	static char error_buffer[255];
	av_strerror(error, error_buffer, sizeof(error_buffer));
	return error_buffer;
}

int ffmpeg_retval(const int ret)
{
	if (ret < 0)
		fprintf_rl(stdout, "ffmpeg error: %s\n", ffmpeg_get_error_text(ret));

	return ret < 0;
}

void load_audio_full_ffmpeg(const char* input_filename)
{
	int i, audio_stream_id = -1;
	AVFormatContext *format_ctx=NULL;	// media container

	av_register_all();	// inits libav

	if (avformat_open_input(&format_ctx, input_filename, NULL, NULL) < 0)
	{
		fprintf_rl(stderr, "Could not open file %s using avformat_open_input()\n", input_filename);
		return ;
	}

	if (avformat_find_stream_info(format_ctx, NULL) < 0)
	{
		fprintf_rl(stderr, "Could not find info for file %s using av_find_stream_info()\n", input_filename);
		return ;
	}

	for (i=0; i < format_ctx->nb_streams; i++)
	{
		if (format_ctx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)	// find the audio stream
		{
			audio_stream_id = i;
			break;
		}
	}

	if (audio_stream_id == -1)
	{
		fprintf_rl(stderr, "Could not find an audio stream for file %s\n", input_filename);
		return ;
	}

	// Find the apropriate codec and open it
	AVCodecContext *codec_context = format_ctx->streams[audio_stream_id]->codec;
	AVCodec *codec = avcodec_find_decoder(codec_context->codec_id);

	if (!avcodec_open2(codec_context, codec, NULL) < 0)
	{
		fprintf_rl(stderr, "Could not find open the needed codec for file %s using avcodec_open()\n", input_filename);
		return ;
	}

fprintf_rl(stdout, "samplerate: %d\tchannels: %d\tcodec: %s\n", codec_context->sample_rate, codec_context->channels, codec->long_name);


	AVPacket packet;
	AVFrame *frame = av_frame_alloc();
	int ret, counter=0, v, ic;
int histo[8][256];
for (ic=0; ic<8; ic++)
memset(histo[ic], 0, 256*sizeof(int));

	while (av_read_frame(format_ctx, &packet)==0)	// get the next frame from the stream
	{
		if (packet.stream_index == audio_stream_id)
		{
			ret = avcodec_send_packet(codec_context, &packet);		// supply raw packet data as input to a decoder
			ffmpeg_retval(ret);

			ret = avcodec_receive_frame(codec_context, frame);		// return decoded output data (in frame) from a decoder
			ffmpeg_retval(ret);

			counter += frame->nb_samples;
//av C:\Users\user\Desktop\_to learn\movies\#3\Ennio Morricone - 2004 - The Good, The Bad And The Ugly [Flac-Cue]\Ennio Morricone - The Good, The Bad And The Ugly.flac
			// raw sample printout
		/*	for (int i=0; i<frame->nb_samples; i++)
				for (int ic=0; ic<frame->channels; ic++)
					fprintf_rl(stdout, "%d%c", ((int16_t *) frame->data[ic])[i], ic < frame->channels-1 ? '\t' : '\n');
		*/
			for (int i=0; i<frame->nb_samples; i++)
				for (int ic=0; ic<frame->channels; ic++)
				{
					if (frame->data[ic])
						v = ((int16_t *) frame->data[ic])[i] + 32768;
					else
						v = ((int16_t *) frame->data[0])[i+ic*frame->nb_samples] + 32768;
					histo[ic][v*20/65536]++;
				}

			//fprintf_rl(stdout, "%7d\n", counter);
		}

		av_packet_unref(&packet);
	}
fprintf_rl(stdout, "samples: %d, duration = %.2f s\n", counter, (float) counter / (float) codec_context->sample_rate);
for (i=0; i<20; i++)
for (ic=0; ic<frame->channels; ic++)
	if (histo[ic][i])
		fprintf_rl(stdout, "%3.3f%%%c", 100. * (double) histo[ic][i] / (double) counter, ic < frame->channels-1 ? '\t' : '\n');
	else
		fprintf_rl(stdout, "    0%%%c", ic < frame->channels-1 ? '\t' : '\n');

	av_frame_free(&frame);
	avformat_close_input(&format_ctx);
fprintf_rl(stdout, "Done.\n");
}
#endif
