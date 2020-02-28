#define DR_FLAC_IMPLEMENTATION
#define drflac__fopen(path)	fopen_utf8(path, "rb")
#include "orig/dr_flac.h"

float *load_sound_flac_file(const char *path, size_t *sample_count, int *channels, int *samplerate)
{
	uint64_t total_sample_count=0;
	float *snd = drflac_open_file_and_read_pcm_frames_f32(path, channels, samplerate, &total_sample_count, NULL);

	*sample_count = total_sample_count / *channels;

	return snd;
}
