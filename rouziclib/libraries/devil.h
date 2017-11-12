#ifdef RL_DEVIL

#ifdef _MSC_VER
#pragma comment (lib, "DevIL.lib")
#pragma comment (lib, "ILU.lib")
#pragma comment (lib, "ILUT.lib")
#endif

#include <IL/il.h>
#include <IL/ilu.h>

typedef struct
{
	raster_t *im;
	char *in_path;
	int mode;
	void *clctx;
} load_image_libdevil_thread_data_t;

extern raster_t load_image_libdevil_from_memory(ILubyte *raw_data, ILuint size, const int mode, const void *clctx);
extern raster_t load_image_libdevil(const char *in_path, const int mode, const void *clctx);

#endif
