#ifdef _WIN32
#include <mmeapi.h>

typedef struct
{
	HMIDIIN handle;
	MIDIHDR header;
} midiin_dev_t;

extern int init_midi_input_device(int dev_id, midiin_dev_t *dev, buffer_t *err_log, void *callback_func, void *callback_data);

#endif
