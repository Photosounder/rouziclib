// in libraries/opencl_struct.h:
// clctx_t, clew.h

#ifdef RL_OPENCL

extern const char *get_cl_error_string(cl_int err);
extern void check_compilation_log(clctx_t *c, cl_program program);
extern cl_int init_cl_context_from_gl(clctx_t *c, cl_platform_id platform);
extern cl_int init_cl_context(clctx_t *c, const int from_gl);
extern void deinit_clctx(clctx_t *c, int deinit_kernel);
extern cl_int build_cl_program(clctx_t *c, cl_program *program, const char *src);
extern cl_int create_cl_kernel(clctx_t *c, cl_program program, cl_kernel *kernel, const char *name);
extern cl_int zero_cl_mem(clctx_t *c, cl_mem buffer, size_t size);
extern void init_framebuffer_cl(const clctx_t *clctx);
extern void cl_make_srgb_tex();
extern cl_int init_fb_cl();

#define CL_ERR_RET(name, ret)	if (ret != CL_SUCCESS) { fprintf_rl(stderr, "%s failed (err %d: %s)\n", name, ret, get_cl_error_string(ret));	return ret; }
#define CL_ERR_NORET(name, ret)	if (ret != CL_SUCCESS) { fprintf_rl(stderr, "%s failed (err %d: %s)\n", name, ret, get_cl_error_string(ret));	return ; }

#endif

extern int check_opencl();
extern void dialog_cl_gl_interop_options(rect_t area, int parent_on, int *detached);
