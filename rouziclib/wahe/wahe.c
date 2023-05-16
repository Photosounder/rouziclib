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
	error = wasmtime_func_call(ctx->context, &ctx->malloc_func, param, 1, ret, 1, &trap);
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
	wasmtime_linker_get_memory(ctx);

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
	error = wasmtime_func_call(ctx->context, &ctx->free_func, param, 1, NULL, 0, &trap);
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

int call_module_draw(wahe_module_t *ctx, xyi_t recommended_resolution)
{
	wasmtime_error_t *error;
	wasm_trap_t *trap = NULL;
	wasmtime_val_t ret[1], param[2];

	// Write message to send
	if (ctx->draw_msg_addr == 0)
		ctx->draw_msg_addr = call_module_malloc(ctx, 50);
	
	sprintf(&ctx->memory_ptr[ctx->draw_msg_addr], "Recommended resolution %dx%d", recommended_resolution.x, recommended_resolution.y);

	// Allocate space for return message pointer
	if (ctx->draw_ret_msg_addr_ptr == 0)
		ctx->draw_ret_msg_addr_ptr = call_module_malloc(ctx, ctx->address_type == WASMTIME_I32 ? sizeof(int32_t) : sizeof(int64_t));

	// Set params
	param[0] = wasmtime_val_set_address(ctx, ctx->draw_msg_addr);
	param[1] = wasmtime_val_set_address(ctx, ctx->draw_ret_msg_addr_ptr);

	// Call the function
	error = wasmtime_func_call(ctx->context, &ctx->draw_func, param, 2, ret, 1, &trap);

	// Update memory pointer
	wasmtime_linker_get_memory(ctx);

	// Handle function call problems
	if (error || trap)
	{
		fprintf_rl(stderr, "call_module_draw() failed\n");
		fprint_wasmtime_error(error, trap);
		return -1;
	}

	// Make the pointer to the return message
	size_t draw_ret_msg_addr;
	if (ctx->address_type == WASMTIME_I32)
		draw_ret_msg_addr = *((int32_t *) &ctx->memory_ptr[ctx->draw_ret_msg_addr_ptr]);
	else
		draw_ret_msg_addr = *((int64_t *) &ctx->memory_ptr[ctx->draw_ret_msg_addr_ptr]);
	char *message = &ctx->memory_ptr[draw_ret_msg_addr];

	// Parse the return message
	int ret_mode = get_raster_mode(ctx->fb);
	for (const char *line = message; line; line = strstr_after(line, "\n"))
	{
		char a[32];

		if (sscanf(line, "Pixel format: %[^\n]", a) == 1)
			ret_mode = wahe_pixel_format_to_raster_mode(a);

		sscanf(line, "Framebuffer location: %zu bytes at %zi", &ctx->raster_size, &ctx->raster_address);
		sscanf(line, "Framebuffer resolution %dx%d", &ctx->fb.dim.x, &ctx->fb.dim.y);
	}

	// Update host-side raster for module framebuffer
	ctx->fb = make_raster(&ctx->memory_ptr[ctx->raster_address], ctx->fb.dim, ctx->fb.dim, ret_mode);

	// The return value indicates whether or not the framebuffer was updated
	return ret[0].of.i32;
}

void call_module_user_input(wahe_module_t *ctx, size_t message_offset, size_t data_offset, size_t data_len)
{
	wasmtime_error_t *error;
	wasm_trap_t *trap = NULL;
	wasmtime_val_t param[3];

	// Set params
	param[0] = wasmtime_val_set_address(ctx, message_offset);
	param[1] = wasmtime_val_set_address(ctx, data_offset);
	param[2] = wasmtime_val_set_address(ctx, data_len);

	// Call the function
	error = wasmtime_func_call(ctx->context, &ctx->user_input_func, param, 3, NULL, 0, &trap);
	if (error || trap)
	{
		fprintf_rl(stderr, "call_module_user_input() failed\n");
		fprint_wasmtime_error(error, trap);
		return;
	}
}

size_t module_sprintf_alloc(wahe_module_t *ctx, const char* format, ...)
{
	int len;
	va_list args, args_copy;

	va_start(args, format);

	// Get length of string to print
	va_copy(args_copy, args);
	len = vsnprintf(NULL, 0, format, args_copy);
	va_end(args_copy);

	// Alloc message in the module's linear memory
	size_t offset = call_module_malloc(ctx, len+1);

	// Print
	vsnprintf(&ctx->memory_ptr[offset], len+1, format, args);

	va_end(args);

	return offset;
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

void wahe_module_init(wahe_module_t *ctx, const char *path)
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
	wasmtime_func_t print_func;

	error = wasmtime_linker_define_func(ctx->linker, "env", 3, "wahe_print", strlen("wahe_print"), func_type, wahe_print, ctx, NULL);
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
	wasmtime_linker_get_func(ctx, "module_user_input", &ctx->user_input_func, 1);

	// Get pointer to linear memory
	wasmtime_linker_get_memory(ctx);

	// Set the type of module address offsets (currently always 32-bit)
	ctx->address_type = WASMTIME_I32;

	// Init module's textedit used for transmitting text input
	textedit_init(&ctx->input_te, 1);
}

// Gets called from the module
wasm_trap_t *wahe_print(void *env, wasmtime_caller_t *caller, const wasmtime_val_t *args, size_t nargs, wasmtime_val_t *results, size_t nresults)
{
	wahe_module_t *ctx = env;

	uint32_t string_offset = args[0].of.i32;
	char *string = &ctx->memory_ptr[string_offset];
	fprintf_rl(stdout, "*=*WASM*=*   %s\n", string);
	return NULL;
}
