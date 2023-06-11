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
	wasmtime_func_t malloc_func, realloc_func, free_func, draw_func, input_func;

	textedit_t input_te;

	// Input-specific
	size_t input_msg_addr, input_ret_msg_addr;

	// Draw-specific
	raster_t fb;
	size_t raster_address, raster_size, draw_msg_addr, draw_ret_msg_addr;
	rect_t fb_rect;

	void *parent_group;	// wahe_group_t *
} wahe_module_t;

enum wahe_func_id
{
	WAHE_FUNC_INPUT=1,
	WAHE_FUNC_DRAW
};

typedef struct
{
	int type;
	int module_id;
	enum wahe_func_id func_id;
} wahe_connection_end_t;

typedef struct
{
	wahe_connection_end_t src, dst;
	int exec_order_index;
} wahe_connection_t;

typedef struct
{
	wahe_module_t *module;
	size_t module_count, module_as;

	wahe_connection_t *connection;
	size_t conn_count, conn_as;

	wahe_connection_end_t *exec_order;
	size_t exec_order_count, exec_order_as;
} wahe_group_t;

extern int wasmtime_linker_get_memory(wahe_module_t *ctx);
extern int wasmtime_linker_get_func(wahe_module_t *ctx, const char *func_name, wasmtime_func_t *func, int verbosity);
extern wasmtime_val_t wasmtime_val_set_address(wahe_module_t *ctx, size_t address);
extern size_t wasmtime_val_get_address(wasmtime_val_t val);
extern size_t call_module_malloc(wahe_module_t *ctx, size_t size);
extern void call_module_free(wahe_module_t *ctx, size_t address);
extern int wahe_pixel_format_to_raster_mode(const char *name);
extern char *call_module_message_input(wahe_module_t *ctx, size_t message_addr);
extern int call_module_draw(wahe_module_t *ctx, xyi_t recommended_resolution);

extern size_t module_vsprintf_alloc(wahe_module_t *ctx, const char *format, va_list args);
extern size_t module_sprintf_alloc(wahe_module_t *ctx, const char* format, ...);
extern char *wahe_send_input(wahe_module_t *ctx, const char *format, ...);
extern int is_wasmtime_func_found(wasmtime_func_t func);
extern void fprint_wasmtime_error(wasmtime_error_t *error, wasm_trap_t *trap);
extern void wahe_module_init(wahe_group_t *parent_group, int module_index, wahe_module_t *ctx, const char *path);

extern wasm_trap_t *wahe_print(void *env, wasmtime_caller_t *caller, const wasmtime_val_t *arg, size_t arg_count, wasmtime_val_t *result, size_t result_count);
extern wasm_trap_t *wahe_run_command(void *env, wasmtime_caller_t *caller, const wasmtime_val_t *arg, size_t arg_count, wasmtime_val_t *result, size_t result_count);
