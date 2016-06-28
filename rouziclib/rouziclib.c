/* Make a .c file in your project containing an include to the header file you must 
 * create in your project and an include to this file, rouziclib.c, like this:

#include "rl.h"

#include <rouziclib/rouziclib.c>

****************/

// C files
	
#include "general/time.c"

#include "memory/circular_buffer.c"

#include "graphics/srgb.c"
#include "graphics/colour.c"
#include "graphics/blending.c"
#include "graphics/blit.c"
#include "graphics/drawline.c"
#include "graphics/drawprimitives.c"
#include "graphics/drawqueue.c"
#include "gui/drawelements.c"
#include "vector/vector.c"

#include "geometry/intersection.c"
#include "geometry/rotation.c"
#include "math/functions.c"
#include "math/rand.c"
#include "math/dsp.c"
#include "math/matrix.c"
#include "fixedpoint/fp.c"
#include "fastfloat/fastfloat.c"

#include "text/unicode_data.c"
#include "text/unicode.c"
#include "text/parse.c"
#include "text/format.c"
#include "vector_type/vector_type.c"

#include "fileio/open.c"
#include "fileio/image.c"
#include "fileio/dir.c"

#include "libraries/opencv.c"
#include "libraries/opencl.c"
#include "libraries/sdl.c"
#include "libraries/devil.c"
#include "libraries/clfft.c"
#include "libraries/ffmpeg.c"
