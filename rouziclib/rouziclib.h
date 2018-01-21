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


 * with MinGW's GCC make sure to use -lwinmm

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
#include <float.h>
#include <limits.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#ifdef _WIN32
#define _WINSOCKAPI_	// prevents Winsock.h / Winsock2.h conflict
#include <windows.h>
#endif

// Structs and includes needed elsewhere
#include "general/structs.h"
#include "general/xyz_struct.h"
#include "geometry/rect_struct.h"		// needs xyz
#include "fileio/dir_struct.h"
#include "gui/controls_struct.h"
#include "gui/inputprocessing_struct.h"
#include "gui/focus_struct.h"
#include "libraries/opencl_struct.h"		// unused unless RL_OPENCL is defined

#include "graphics/graphics_struct.h"		// needs opencl, xyz
#include "graphics/blending_struct.h"		// needs graphics
#include "general/mouse_struct.h"		// needs rect, xyz, controls
#include "gui/zoom_struct.h"			// needs rect, xyz, mouse, graphics
#include "vector/vector_struct.h"		// needs xyz
#include "vector_type/vector_type_struct.h"	// needs vector


#include "general/macros.h"
#include "general/xyz.h"
#include "general/time.h"
#include "general/estimates.h"
#include "general/mouse.h"
#include "general/noop.h"

#include "memory/alloc.h"
#include "memory/circular_buffer.h"

#include "geometry/rect.h"
#include "geometry/intersection.h"
#include "geometry/rotation.h"
#include "geometry/fit.h"
#include "geometry/distance.h"
#include "math/functions.h"
#include "math/rand.h"
#include "math/dsp.h"
#include "math/dct.h"
#include "math/matrix.h"
#include "math/physics.h"
#include "fixedpoint/fp.h"			// used unless RL_EXCL_APPROX is defined
#include "fastfloat/fastfloat.h"		// used unless RL_EXCL_APPROX is defined

#include "graphics/graphics.h"
#include "graphics/srgb.h"
#include "graphics/colour.h"
#include "graphics/blending.h"
#include "graphics/blit.h"
#include "graphics/drawline.h"
#include "graphics/drawrect.h"
#include "graphics/drawprimitives.h"
#include "graphics/drawqueue.h"
#include "graphics/processing.h"
#include "vector/vector.h"
#include "vector/polyline.h"

#include "text/unicode_data.h"			// needs RL_INCL_UNICODE_DATA to be defined
#include "text/unicode.h"
#include "text/unicode_bidi.h"
#include "text/unicode_arabic.h"
#include "text/parse.h"
#include "text/format.h"
#include "text/string.h"
#include "text/edit.h"
#include "text/undo.h"
#include "vector_type/vector_type.h"

#include "gui/zoom.h"
#include "gui/focus.h"
#include "gui/positioning.h"
#include "gui/layout.h"
#include "gui/drawelements.h"
#include "gui/visualisations.h"
#include "gui/inputprocessing.h"
#include "gui/knob_functions.h"
#include "gui/controls.h"
#include "gui/control_array.h"

#include "fileio/open.h"
#include "fileio/image.h"
#include "fileio/path.h"
#include "fileio/dir.h"
#include "fileio/file_management.h"
#include "fileio/process.h"

// used unless RL_EXCL_NETWORK is defined
#include "network/network.h"
#include "network/http.h"	// may need RL_ZLIB

#include "libraries/opencl.h"		// unused unless RL_OPENCL is defined
#include "libraries/opencv.h"		// unused unless RL_OPENCV is defined
#include "libraries/sdl.h"		// unused unless RL_SDL is defined
#include "libraries/devil.h"		// unused unless RL_DEVIL is defined
#include "libraries/clfft.h"		// unused unless RL_CLFFT is defined
#include "libraries/ffmpeg.h"		// unused unless RL_FFMPEG is defined
#include "libraries/libsndfile.h"	// unused unless RL_LIBSNDFILE is defined
#include "libraries/libraw.h"		// unused unless RL_LIBRAW is defined
#include "libraries/libjpeg.h"		// unused unless RL_LIBJPEG is defined
#include "libraries/zlib.h"		// unused unless RL_ZLIB is defined
#include "libraries/mpfr.h"		// unused unless RL_MPFR is defined
#include "libraries/fftpack.h"		// used unless RL_EXCL_FFTPACK is defined
#include "libraries/tinycthread.h"	// unused unless RL_TINYCTHREAD is defined
#include "libraries/threading.h"	// unused unless RL_TINYCTHREAD is defined


#ifdef __cplusplus
}
#endif
#endif
