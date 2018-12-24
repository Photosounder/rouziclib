#ifdef RL_OPENCL

#include "libraries/clew.h"

#ifdef RL_OPENCL_GL
	#ifdef RL_BUILTIN_GLEW
		#define GLEW_STATIC
		#include "libraries/glew.h"
	#else
		#include <GL/glew.h>
	#endif
	
	#ifdef _MSC_VER
		#pragma comment (lib, "opengl32.lib")

		#ifndef RL_BUILTIN_GLEW
			#pragma comment (lib, "glew32.lib")
		#endif
	#endif
#endif

/*#ifdef _MSC_VER
	#pragma comment (lib, "C:\\Program Files\\NVIDIA GPU Computing Toolkit\\CUDA\\v10.0\\lib\\x64\\OpenCL.lib")
#endif

#ifdef __EMSCRIPTEN__
	#include <CL/opencl.h>
	#include <CL/cl_gl.h>
#else

	#ifdef __APPLE__
		#include <OpenCL/opencl.h>
		#include <OpenCL/cl_gl.h>
		#include <OpenGL/OpenGL.h>
	#else
		#include <CL/cl.h>
		#include <CL/cl_gl.h>
	#endif

#endif*/

typedef struct
{
	cl_device_id	 device_id;
	cl_context	 context;
	cl_command_queue command_queue;
} clctx_t;

#endif
