#ifdef RL_OPENCL

#ifdef _MSC_VER
#pragma comment (lib, "C:\\Program Files\\NVIDIA GPU Computing Toolkit\\CUDA\\v10.0\\lib\\x64\\OpenCL.lib")
#endif

#ifdef _MSC_VER
#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "glew32.lib")
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

#endif

typedef struct
{
	cl_device_id	 device_id;
	cl_context	 context;
	cl_command_queue command_queue;
} clctx_t;

#endif
