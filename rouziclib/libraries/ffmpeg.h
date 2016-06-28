#ifdef RL_FFMPEG

#ifdef _MSC_VER
#pragma comment (lib, "avcodec.lib")
#pragma comment (lib, "avformat.lib")
#pragma comment (lib, "avutil.lib")
#endif

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>

extern void load_audio_full_ffmpeg(const char* input_filename);

#endif
