#ifdef RL_OPENCL

#ifdef _MSC_VER
#pragma comment (lib, "C:\\Program Files\\NVIDIA GPU Computing Toolkit\\CUDA\\v7.5\\lib\\x64\\OpenCL.lib")
#endif

#ifdef _MSC_VER
#pragma comment (lib, "opengl32.lib")
#pragma comment (lib, "glew32.lib")
#endif

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#include <OpenCL/cl_gl.h>
#include <OpenGL/OpenGL.h>
#else
#include <CL/cl.h>
#include <CL/cl_gl.h>
#endif

typedef struct
{
	cl_device_id	 device_id;
	cl_context	 context;
	cl_command_queue command_queue;
} clctx_t;

extern const char *get_cl_error_string(cl_int err);
extern void check_compilation_log(clctx_t *c, cl_program program);
extern cl_int init_cl_context_from_gl(clctx_t *c, cl_platform_id platform);
extern cl_int init_cl_context(clctx_t *c, const int from_gl);
extern void deinit_clctx(clctx_t *c);
extern cl_int build_cl_program(clctx_t *c, cl_program *program, const char *src);
extern cl_int build_cl_program_from_file(clctx_t *c, cl_program *program, char *cl_src_path);
extern cl_int create_cl_kernel(clctx_t *c, cl_program program, cl_kernel *kernel, const char *name);
extern cl_int zero_cl_mem(clctx_t *c, cl_mem buffer, size_t size);
extern cl_int init_raster_cl(void *fb, const clctx_t *clctx);
extern uint64_t get_clmem_device_address(clctx_t *clctx, cl_mem buf);

#define CL_ERR_RET(name, ret)	if (ret != CL_SUCCESS) { fprintf_rl(stderr, "%s failed (%s)\n", name, get_cl_error_string(ret));	return ret; }
#define CL_ERR_NORET(name, ret)	if (ret != CL_SUCCESS) { fprintf_rl(stderr, "%s failed (%s)\n", name, get_cl_error_string(ret));	return ; }

#endif
