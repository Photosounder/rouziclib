#ifdef RL_MINIAUDIO

#define MA_NO_DECODING
#define MA_NO_ENCODING
#define MA_NO_GENERATION
#include "orig/miniaudio.h"

extern int miniaudio_init_wasapi_loopback(void *data_callback, void *user_data);

#endif
