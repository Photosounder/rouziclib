void wahe_bench_point(wahe_module_t *ctx, const char *label, int depth)
{
	return;
	static double ref_time = 0.;
	static int bench_depth = 0;
	static buffer_t buf = {0};

	// Set ref time if we're starting at the bottom level
	if (bench_depth == 0)
		ref_time = get_time_hr();

	// Decrement depth
	if (depth < 0)
		bench_depth--;

	// Indent printing according to depth
	for (int i=0; i < bench_depth; i++)
		bufprintf(&buf, "\t");
	bufprintf(&buf, "Bench %s: %.3f ms\n", label, 1e3*(get_time_hr() - ref_time));

	// Increment depth
	if (depth > 0)
		bench_depth++;

	// Print buffer to output if it's the end
	if (bench_depth == 0)
	{
		if (get_time_hr() - ref_time > 0.6/60.)
			fprintf_rl(stdout, "%s\n", buf.buf);
		clear_buf(&buf);
	}
}

int wasmtime_linker_get_memory(wahe_module_t *ctx)
{
	wasmtime_extern_t item;

	// Look for memory
	if (!wasmtime_linker_get(ctx->linker, ctx->context, "", 0, "memory", strlen("memory"), &item))
	{
		fprintf_rl(stderr, "Error: memory not found in wasmtime_linker_get()\n");
		return 0;
	}

	if (item.kind != WASMTIME_EXTERN_MEMORY)
	{
		fprintf_rl(stderr, "Error: memory found in wasmtime_linker_get() is not a memory\n");
		return 0;
	}

	ctx->memory = item.of.memory;

	// Get pointer to start of module's linear memory
	ctx->memory_ptr = wasmtime_memory_data(ctx->context, &ctx->memory);

	return 1;

}

int wasmtime_linker_get_func(wahe_module_t *ctx, const char *func_name, wasmtime_func_t *func, int verbosity)
{
	wasmtime_extern_t func_ext;

	if (!wasmtime_linker_get(ctx->linker, ctx->context, "", 0, func_name, strlen(func_name), &func_ext))
	{
		if (verbosity == -1)
			fprintf_rl(stderr, "Error: function %s() not found in wasmtime_linker_get()\n", func_name);
		return 0;
	}

	if (func_ext.kind != WASMTIME_EXTERN_FUNC)
	{
		if (verbosity == -1)
			fprintf_rl(stderr, "Error: symbol %s found in wasmtime_linker_get() is not a function\n", func_name);
		return 0;
	}

	*func = func_ext.of.func;

	if (verbosity == 1)
		fprintf_rl(stdout, "Function %s found\n", func_name);

	return 1;

}

wasmtime_val_t wasmtime_val_set_address(wahe_module_t *ctx, size_t address)
{
	wasmtime_val_t val;

	val.kind = ctx->address_type;

	if (ctx->address_type == WASMTIME_I32)
		val.of.i32 = address;
	else
		val.of.i64 = address;

	return val;
}

size_t wasmtime_val_get_address(wasmtime_val_t val)
{
	if (val.kind == WASMTIME_I32)
		return val.of.i32;
	else
		return val.of.i64;
}

size_t call_module_malloc(wahe_module_t *ctx, size_t size)
{
	wasmtime_error_t *error;
	wasm_trap_t *trap = NULL;
	wasmtime_val_t ret[1], param[1];

	// Set params
	param[0] = wasmtime_val_set_address(ctx, size);

	// Call the function
	wahe_bench_point(ctx, "calling malloc()", 1);
	error = wasmtime_func_call(ctx->context, &ctx->malloc_func, param, 1, ret, 1, &trap);
	wahe_bench_point(ctx, "malloc() returned", 0);
	if (error || trap)
	{
		fprintf_rl(stderr, "call_module_malloc(ctx, %zu) failed\n", size);
		fprint_wasmtime_error(error, trap);
		return 0;
	}

	// Check result type
	if (ret[0].kind != ctx->address_type)
	{
		fprintf_rl(stderr, "call_module_malloc() expected a type %s result\n", ctx->address_type==WASMTIME_I32 ? "int32_t" : "int64_t");
		return 0;
	}

	// Update memory pointer
	wahe_bench_point(ctx, "Updating with wasmtime_linker_get_memory()", 0);
	wasmtime_linker_get_memory(ctx);
	wahe_bench_point(ctx, "call_module_malloc() end", -1);

	// Return result
	return wasmtime_val_get_address(ret[0]);
}

void call_module_free(wahe_module_t *ctx, size_t address)
{
	wasmtime_error_t *error;
	wasm_trap_t *trap = NULL;
	wasmtime_val_t param[1];

	// Set params
	param[0] = wasmtime_val_set_address(ctx, address);

	// Call the function
	wahe_bench_point(ctx, "calling free()", 1);
	error = wasmtime_func_call(ctx->context, &ctx->free_func, param, 1, NULL, 0, &trap);
	wahe_bench_point(ctx, "free() returned", -1);
	if (error || trap)
	{
		fprintf_rl(stderr, "call_module_free(ctx, %zu) failed\n", address);
		fprint_wasmtime_error(error, trap);
		return;
	}
}

int wahe_pixel_format_to_raster_mode(const char *name)
{
	if (strcmp(name, "RGBA UQ1.15 linear") == 0)
		return IMAGE_USE_LRGB;

	if (strcmp(name, "RGBA float linear") == 0)
		return IMAGE_USE_FRGB;

	if (strcmp(name, "RGBA 8 sRGB") == 0)
		return IMAGE_USE_SRGB;

	if (strcmp(name, "RGB 10-12-10 sqrt") == 0)
		return IMAGE_USE_SQRGB;

	return IMAGE_USE_BUF;
}

char *call_module_message_input(wahe_module_t *ctx, size_t message_addr)
{
	wasmtime_error_t *error;
	wasm_trap_t *trap = NULL;
	wasmtime_val_t ret[1], param[1];

	if (ctx->input_func.store_id == 0)
		return NULL;

	// Set params
	param[0] = wasmtime_val_set_address(ctx, message_addr);

	// Call the function
	wahe_bench_point(ctx, "calling module_message_input()", 1);
	error = wasmtime_func_call(ctx->context, &ctx->input_func, param, 1, ret, 1, &trap);
	wahe_bench_point(ctx, "module_message_input() returned", -1);
	if (error || trap)
	{
		fprintf_rl(stderr, "call_module_message_input() failed\n");
		fprint_wasmtime_error(error, trap);
		return NULL;
	}

	// Save return pointer
	ctx->input_ret_msg_addr = wasmtime_val_get_address(ret[0]);

	// Update memory pointer
	wasmtime_linker_get_memory(ctx);

	// Return pointer to return message
	if (ctx->input_ret_msg_addr)
		return (char *) &ctx->memory_ptr[ctx->input_ret_msg_addr];
	else
		return NULL;
}

int call_module_draw(wahe_module_t *ctx, xyi_t recommended_resolution)
{
	wasmtime_error_t *error;
	wasm_trap_t *trap = NULL;
	wasmtime_val_t ret[1], param[1];

	if (ctx->draw_func.store_id == 0)
		return -1;

	// Write message to send
	/*call_module_free(ctx, ctx->draw_msg_addr);
	ctx->draw_msg_addr = module_sprintf_alloc(ctx, "Recommended resolution %dx%d", recommended_resolution.x, recommended_resolution.y);*/

	// Set params
	param[0] = wasmtime_val_set_address(ctx, ctx->draw_msg_addr);

	// Call the function
	wahe_bench_point(ctx, "calling module_draw()", 1);
	error = wasmtime_func_call(ctx->context, &ctx->draw_func, param, 1, ret, 1, &trap);
	wahe_bench_point(ctx, "module_draw() returned", -1);
	if (error || trap)
	{
		fprintf_rl(stderr, "call_module_draw() failed\n");
		fprint_wasmtime_error(error, trap);
		return -1;
	}

	// Update memory pointer
	wasmtime_linker_get_memory(ctx);

	// Parse the return message
	if (wasmtime_val_get_address(ret[0]))
	{
		int ret_mode = get_raster_mode(ctx->fb);

		// Get the message address and make pointer to the message
		ctx->draw_ret_msg_addr = wasmtime_val_get_address(ret[0]);
		char *message = &ctx->memory_ptr[ctx->draw_ret_msg_addr];

		// Parse each line of the message
		for (const char *line = message; line; line = strstr_after(line, "\n"))
		{
			char a[32];

			if (sscanf(line, "Pixel format: %[^\n]", a) == 1)
				ret_mode = wahe_pixel_format_to_raster_mode(a);

			sscanf(line, "Framebuffer location: %zu bytes at %zi", &ctx->raster_size, &ctx->raster_address);
			sscanf(line, "Framebuffer resolution %dx%d", &ctx->fb.dim.x, &ctx->fb.dim.y);
		}

		// Update the host-side raster for the module framebuffer
		ctx->fb = make_raster(&ctx->memory_ptr[ctx->raster_address], ctx->fb.dim, ctx->fb.dim, ret_mode);
	}

	// The return value indicates whether or not the framebuffer was updated
	return wasmtime_val_get_address(ret[0]) != 0;
}

size_t module_vsprintf_alloc(wahe_module_t *ctx, const char *format, va_list args)
{
	int len;

	// Get length of string to print
	len = vstrlenf(format, args);

	// Alloc message in the module's linear memory
	size_t addr = call_module_malloc(ctx, len+1);

	// Print
	vsnprintf(&ctx->memory_ptr[addr], len+1, format, args);

	return addr;
}

size_t module_sprintf_alloc(wahe_module_t *ctx, const char *format, ...)
{
	va_list args;

	va_start(args, format);
	size_t addr = module_vsprintf_alloc(ctx, format, args);
	va_end(args);

	return addr;
}

char *wahe_send_input(wahe_module_t *ctx, const char *format, ...)
{
	va_list args;

	// Print to message allocated in the module's memory
	va_start(args, format);
	size_t message_addr = module_vsprintf_alloc(ctx, format, args);
	va_end(args);

	// Send the message then free it
	char *return_msg = call_module_message_input(ctx, message_addr);
	call_module_free(ctx, message_addr);

	return return_msg;
}

int is_wasmtime_func_found(wasmtime_func_t func)
{
	return func.store_id != 0;
}

void fprint_wasmtime_error(wasmtime_error_t *error, wasm_trap_t *trap)
{
	wasm_byte_vec_t error_message;

	if (error)
	{
		wasmtime_error_message(error, &error_message);
		wasmtime_error_delete(error);
		fprintf_rl(stderr, "wasmtime_error_message(): \"%.*s\"\n", (int) error_message.size, error_message.data);
		wasm_byte_vec_delete(&error_message);
	}

	if (trap)
	{
		wasm_trap_message(trap, &error_message);
		wasm_trap_delete(trap);
		fprintf_rl(stderr, "wasm_trap_message(): \"%.*s\"\n", (int) error_message.size, error_message.data);
		wasm_byte_vec_delete(&error_message);
	}
}

void wahe_module_init(wahe_group_t *parent_group, int module_index, wahe_module_t *ctx, const char *path)
{
	wasmtime_error_t *error;
	wasm_functype_t *func_type;

	memset(ctx, 0, sizeof(wahe_module_t));

	// WASM initialisation
	wasm_config_t *config = wasm_config_new();
	//wasmtime_config_debug_info_set(config, true);
	ctx->engine = wasm_engine_new_with_config(config);
	ctx->store = wasmtime_store_new(ctx->engine, NULL, NULL);
	ctx->context = wasmtime_store_context(ctx->store);

	// Create a linker with WASI functions defined
	ctx->linker = wasmtime_linker_new(ctx->engine);
	error = wasmtime_linker_define_wasi(ctx->linker);
	if (error)
	{
		fprintf_rl(stderr, "Error linking WASI in wasmtime_linker_define_wasi()\n");
		fprint_wasmtime_error(error, NULL);
		return;
	}

	// Load WASM file
	buffer_t wasm_buf = buf_load_raw_file(path);

	// Compile WASM
	error = wasmtime_module_new(ctx->engine, wasm_buf.buf, wasm_buf.len, &ctx->module);
	if (error)
	{
		fprintf_rl(stderr, "Error compiling the module in wasmtime_module_new()\n");
		fprint_wasmtime_error(error, NULL);
		return;
	}

	free_buf(&wasm_buf);

	// WASI initialisation
	ctx->wasi_config = wasi_config_new();
	wasi_config_inherit_stdout(ctx->wasi_config);
	wasi_config_inherit_stderr(ctx->wasi_config);
	error = wasmtime_context_set_wasi(ctx->context, ctx->wasi_config);
	if (error)
	{
		fprintf_rl(stderr, "Error initialising WASI in wasmtime_context_set_wasi()\n");
		fprint_wasmtime_error(error, NULL);
		return;
	}

	// Initialise callbacks (host functions called from WASM module)
	func_type = wasm_functype_new_1_0(wasm_valtype_new_i32());
	error = wasmtime_linker_define_func(ctx->linker, "env", strlen("env"), "wahe_print", strlen("wahe_print"), func_type, wahe_print, ctx, NULL);
	if (error)
	{
		fprintf_rl(stderr, "Error defining callback in wasmtime_linker_define_func()\n");
		fprint_wasmtime_error(error, NULL);
		return;
	}
	wasm_functype_delete(func_type);

	func_type = wasm_functype_new_1_1(wasm_valtype_new_i32(), wasm_valtype_new_i32());
	error = wasmtime_linker_define_func(ctx->linker, "env", strlen("env"), "wahe_run_command", strlen("wahe_run_command"), func_type, wahe_run_command, ctx, NULL);
	if (error)
	{
		fprintf_rl(stderr, "Error defining callback in wasmtime_linker_define_func()\n");
		fprint_wasmtime_error(error, NULL);
		return;
	}
	wasm_functype_delete(func_type);

	// Instantiate the module
	error = wasmtime_linker_module(ctx->linker, ctx->context, "", 0, ctx->module);
	if (error)
	{
		fprintf_rl(stderr, "Error instantiating module in wasmtime_linker_module()\n");
		fprint_wasmtime_error(error, NULL);
		return;
	}

	// Find functions from the WASM module
	wasmtime_linker_get_func(ctx, "malloc", &ctx->malloc_func, -1);
	wasmtime_linker_get_func(ctx, "realloc", &ctx->realloc_func, -1);
	wasmtime_linker_get_func(ctx, "free", &ctx->free_func, -1);
	wasmtime_linker_get_func(ctx, "module_draw", &ctx->draw_func, 1);
	wasmtime_linker_get_func(ctx, "module_message_input", &ctx->input_func, 1);

	// Get pointer to linear memory
	wasmtime_linker_get_memory(ctx);

	// Set the type of module addresses (currently always 32-bit)
	ctx->address_type = WASMTIME_I32;

	ctx->parent_group = parent_group;

	// Init module's textedit used for transmitting text input
	textedit_init(&ctx->input_te, 1);

	// Send an Init message to the module
	wahe_send_input(ctx, "Init");

	// Send module index to the module
	if (parent_group)
		wahe_send_input(ctx, "Module index %d", module_index);
}

void wahe_copy_between_memories(wahe_group_t *group, int src_module, size_t src_addr, size_t copy_size, int dst_module, size_t dst_addr)
{
	if (src_module < 0 || src_module >= group->module_count || dst_module < 0 || dst_module >= group->module_count)
		return;

	wahe_module_t *src_ctx = &group->module[src_module];
	wahe_module_t *dst_ctx = &group->module[dst_module];

	// Update memory pointers
	wasmtime_linker_get_memory(src_ctx);
	wasmtime_linker_get_memory(dst_ctx);

	// TODO Check boundaries

	// Copy
	memcpy(&dst_ctx->memory_ptr[dst_addr], &src_ctx->memory_ptr[src_addr], copy_size);
}

size_t wahe_load_raw_file(wahe_module_t *ctx, const char *path, size_t *size)
{
	FILE *in_file;
	uint8_t *data;
	size_t fsize, data_addr;

	if (size)
		*size = 0;

	// Open file handle
	in_file = fopen_utf8(path, "rb");
	if (in_file == NULL)
	{
		fprintf_rl(stderr, "File '%s' not found.\n", path);
		return 0;
	}

	// Get file size
	fseek(in_file, 0, SEEK_END);
	fsize = ftell(in_file);
	rewind(in_file);

	// Alloc data buffer
	data_addr = call_module_malloc(ctx, fsize+1);
	data = &ctx->memory_ptr[data_addr];

	// Read all the data at once
	fread(data, 1, fsize, in_file);
	fclose(in_file);

	if (size)
		*size = fsize;

	return data_addr;
}

// Get called from the module
wasm_trap_t *wahe_print(void *env, wasmtime_caller_t *caller, const wasmtime_val_t *arg, size_t arg_count, wasmtime_val_t *resuls, size_t result_count)
{
	wahe_module_t *ctx = env;

	wahe_bench_point(ctx, "WASM called wahe_print()", 1);

	// Update memory pointer
	wasmtime_linker_get_memory(ctx);

	// Find string and print it
	uint32_t string_addr = wasmtime_val_get_address(arg[0]);
	char *string = &ctx->memory_ptr[string_addr];
	fprintf_rl(stdout, "*=*WASM*=*   %s\n", string);

	wahe_bench_point(ctx, "wahe_print() returning", -1);

	return NULL;
}

wasm_trap_t *wahe_run_command(void *env, wasmtime_caller_t *caller, const wasmtime_val_t *arg, size_t arg_count, wasmtime_val_t *result, size_t result_count)
{
	wahe_module_t *ctx = env;
	size_t return_msg_addr = 0;
	int n;

	// Update memory pointer
	wasmtime_linker_get_memory(ctx);

	// Parse message
	if (wasmtime_val_get_address(arg[0]))
	{
		// Get the message address and make pointer to the message
		char *message = &ctx->memory_ptr[wasmtime_val_get_address(arg[0])];

		// Parse each line of the message
		for (const char *line = message; line; line = strstr_after(line, "\n"))
		{
			// Copy buffer between memories
			int src_module, dst_module;
			size_t src_addr, copy_size, dst_addr;
			if (sscanf(line, "Copy %zu bytes at %zi (module %d) to %zi (module %d)", &copy_size, &src_addr, &src_module, &dst_addr, &dst_module) == 5)
			{
				wahe_copy_between_memories(ctx->parent_group, src_module, src_addr, copy_size, dst_module, dst_addr);
			}

			// Copy buffer between memories with allocation of destination
			if (sscanf(line, "Copy %zu bytes at %zi (module %d) to module %d", &copy_size, &src_addr, &src_module, &dst_module) == 4)
			{
				wahe_group_t *group = ctx->parent_group;
				if (dst_module >= 0 && dst_module < group->module_count)
				{
					dst_addr = call_module_malloc(&group->module[dst_module], copy_size);
					wahe_copy_between_memories(ctx->parent_group, src_module, src_addr, copy_size, dst_module, dst_addr);
					return_msg_addr = module_sprintf_alloc(ctx, "Destination %p", (void *) dst_addr);
				}
			}

			// Load raw file
			int path_start = 0, path_end = 0;
			sscanf(line, "Load raw file at path %n%*[^\n]%n", &path_start, &path_end);
			if (path_end)
			{
				size_t data_addr, data_size = 0;
				char *path = make_string_copy_len(&line[path_start], path_end-path_start);
				data_addr = wahe_load_raw_file(ctx, path, &data_size);
				free(path);
				return_msg_addr = module_sprintf_alloc(ctx, "Data location: %zu bytes at %p", data_size, (void *) data_addr);
			}

			// Return raw time
			n = 0;
			sscanf(line, "Get raw time%n", &n);
			if (n)
				return_msg_addr = module_sprintf_alloc(ctx, "Raw time %.16g seconds", get_time_hr());

			// Benchmark return
			n = 0;
			sscanf(line, "Benchmark%n", &n);
			if (n)
				wahe_bench_point(ctx, "-Benchmark command-", 0);
		}
	}

	// Return message
	result[0] = wasmtime_val_set_address(ctx, return_msg_addr);

	return NULL;
}
