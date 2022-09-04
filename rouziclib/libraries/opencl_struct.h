#ifdef RL_OPENCL

#define clew_STATIC
#include "orig/clew.h"

#ifdef RL_OPENCL_GL
	// My minimal version of GLEW doesn't compile well on Linux, installing the package libglew-dev will have to do
	#if defined(RL_BUILTIN_GLEW) && defined(__linux__)
		#undef RL_BUILTIN_GLEW
	#endif

	#if defined(RL_BUILTIN_GLEW)
		#define GLEW_STATIC
		#include "glew_minimal.h"
	#else
		#ifdef _MSC_VER
			#include <GL/glew.h>
		#elif defined(__linux__)
			#include <GL/glew.h>
			#include <GL/glxew.h>
		#endif
	#endif

	#ifdef _MSC_VER
		#pragma comment (lib, "opengl32.lib")

		#ifndef RL_BUILTIN_GLEW
			#pragma comment (lib, "glew32.lib")
		#endif
	#endif

	#ifdef __APPLE__
		#include <OpenCL/cl_gl.h>
		#include <OpenCL/cl_gl_ext.h>
	#endif
#endif

typedef struct
{
	cl_device_id	 device_id;
	cl_context	 context;
	cl_command_queue command_queue;
	cl_program	 program;
	cl_kernel	 kernel;
	cl_event	 ev;		// clEnqueueNDRangeKernel event
	cl_ulong queue_time, start_time, end_time;
} clctx_t;

#endif
