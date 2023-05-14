#include <wasmtime.h>

#ifdef _MSC_VER
#pragma comment (lib, "wasmtime.dll.lib")
#endif

typedef struct
{
	wasm_engine_t *engine;
	wasmtime_store_t *store;
	wasmtime_context_t *context;
	wasmtime_module_t *module;
	wasi_config_t *wasi_config;
	wasmtime_linker_t *linker;
	wasmtime_memory_t memory;
	uint8_t *memory_ptr;
	wasmtime_valkind_t address_type;
	wasmtime_func_t malloc_func, realloc_func, free_func, draw_func, user_input_func;

	textedit_t input_te;

	// Draw-specific
	raster_t fb;
	size_t raster_address, draw_msg_addr, draw_ret_msg_addr_ptr;
	rect_t fb_rect;
} wahe_module_t;

extern int wasmtime_linker_get_memory(wahe_module_t *ctx);
extern int wasmtime_linker_get_func(wahe_module_t *ctx, const char *func_name, wasmtime_func_t *func, int verbosity);
extern wasmtime_val_t wasmtime_val_set_address(wahe_module_t *ctx, size_t address);
extern size_t wasmtime_val_get_address(wasmtime_val_t val);
extern size_t call_module_malloc(wahe_module_t *ctx, size_t size);
extern void call_module_free(wahe_module_t *ctx, size_t address);
extern int wahe_pixel_format_to_raster_mode(const char *name);
extern int call_module_draw(wahe_module_t *ctx, xyi_t recommended_resolution);
extern void call_module_user_input(wahe_module_t *ctx, size_t message_offset, size_t data_offset, size_t data_len);

extern size_t module_sprintf_alloc(wahe_module_t *ctx, const char* format, ...);
extern int is_wasmtime_func_found(wasmtime_func_t func);
extern void fprint_wasmtime_error(wasmtime_error_t *error, wasm_trap_t *trap);
extern void wahe_module_init(wahe_module_t *ctx, const char *path);

extern wasm_trap_t *wahe_print(void *env, wasmtime_caller_t *caller, const wasmtime_val_t *args, size_t nargs, wasmtime_val_t *results, size_t nresults);
