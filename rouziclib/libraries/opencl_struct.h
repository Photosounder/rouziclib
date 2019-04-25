#ifdef RL_OPENCL

#include "libraries/orig/clew.h"

#ifdef RL_OPENCL_GL
	#ifdef RL_BUILTIN_GLEW
		#define GLEW_STATIC
		#include "libraries/glew_minimal.h"
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

typedef struct
{
	cl_device_id	 device_id;
	cl_context	 context;
	cl_command_queue command_queue;
} clctx_t;

#endif
