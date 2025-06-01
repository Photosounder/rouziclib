#ifdef RL_SOUND_FILE

#define DR_FLAC_IMPLEMENTATION
#define DR_FLAC_NO_STDIO
#include "orig/dr_flac.h"

#endif

float *load_sound_flac_file(const char *path, size_t *sample_count, int *channels, int *samplerate)
{
#ifdef RL_SOUND_FILE
	uint64_t total_sample_count=0;

	buffer_t data = buf_load_raw_file(path);
	float *snd = drflac_open_memory_and_read_pcm_frames_f32(data.buf, data.len, channels, samplerate, &total_sample_count, NULL);
	free_buf(&data);

	*sample_count = total_sample_count;

	return snd;
#else
	*sample_count = 0;
	*channels = 0;
	*samplerate = 0;

	fprintf_rl(stderr, "Define RL_SOUND_FILE in order to be able to use load_sound_flac_file()\n");

	return NULL;
#endif
}
