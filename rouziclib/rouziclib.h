/* Make a .h file in your project containing an include to this file and 
 * add the optional defines you like in it, like this:

#ifndef H_PRL
#define H_PRL
#ifdef __cplusplus
extern "C" {
#endif

#define LBD	12
#define GAUSSLIMIT 0.0002

#include <rouziclib/rouziclib.h>

#ifdef __cplusplus
}
#endif
#endif

****************/

#ifndef H_ROUZICLIB
#define H_ROUZICLIB
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>


#include "general/structs.h"
#include "general/macros.h"
#include "general/time.h"
#include "math/functions.h"
#include "general/xyz.h"

#include "libraries/mpfr.h"
#include "libraries/opencl.h"	// unused unless RL_OPENCL is defined

#include "memory/circular_buffer.h"

#include "graphics/graphics.h"
#include "graphics/srgb.h"
#include "graphics/colour.h"
#include "graphics/blending.h"
#include "graphics/blit.h"
#include "graphics/drawline.h"
#include "graphics/drawprimitives.h"
#include "graphics/drawqueue.h"
#include "gui/drawelements.h"
#include "vector/vector.h"

#include "geometry/intersection.h"
#include "geometry/rotation.h"
#include "math/rand.h"
#include "math/dsp.h"
#include "math/matrix.h"

#include "fixedpoint/fp.h"
#include "fastfloat/fastfloat.h"

#include "text/unicode_data.h"
#include "text/unicode.h"
#include "text/parse.h"
#include "text/format.h"
#include "vector_type/vector_type.h"

#include "fileio/open.h"
#include "fileio/image.h"
#include "fileio/dir.h"

#include "libraries/opencv.h"	// unused unless RL_OPENCV is defined
#include "libraries/sdl.h"	// unused unless RL_SDL is defined
#include "libraries/devil.h"	// unused unless RL_DEVIL is defined
#include "libraries/clfft.h"	// unused unless RL_CLFFT is defined
#include "libraries/ffmpeg.h"	// unused unless RL_FFMPEG is defined


#ifdef __cplusplus
}
#endif
#endif
