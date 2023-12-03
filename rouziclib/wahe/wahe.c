const char *wahe_eo_name[] =
{
	"module_func",
	"image_display",
	"kb_mouse",
	"thread_input_msg"
};

const char *wahe_func_name[] =
{
	"(none)",
	"malloc",
	"realloc",
	"free",

	"input",
	"proc_cmd",
	"draw",
	"proc_image",
	"proc_sound"
};

_Thread_local wahe_module_t *wahe_cur_ctx = NULL;
_Thread_local wahe_thread_t *wahe_cur_thread = NULL;

void wahe_bench_point(const char *label, int depth)
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

	// Nothing to do if the module is NULL (this means host memory is being used)
	if (ctx == NULL)
		return 1;

	// Nothing to do if the module is native
	if (ctx->native)
		return 1;

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

void wahe_get_module_func(wahe_module_t *ctx, const char *func_name, enum wahe_func_id func_id, int verbosity)
{
	if (ctx->native)
	{
		ctx->dl_func[func_id] = dynlib_find_symbol(ctx->native, func_name);

		if (ctx->dl_func[func_id] == NULL && verbosity == -1)
			fprintf_rl(stderr, "Error in module %s: function %s() not found\n", ctx->module_name, func_name);

		if (ctx->dl_func[func_id] && verbosity == 1)
			fprintf_rl(stdout, "Module %s: %s() found\n", ctx->module_name, func_name);
	}
	else
	{
		wasmtime_extern_t func_ext;

		// Get the symbol
		if (!wasmtime_linker_get(ctx->linker, ctx->context, "", 0, func_name, strlen(func_name), &func_ext))
		{
			if (verbosity == -1)
				fprintf_rl(stderr, "Error in module %s: function %s() not found in wasmtime_linker_get()\n", ctx->module_name, func_name);
			return;
		}

		// Check the symbol is a function
		if (func_ext.kind != WASMTIME_EXTERN_FUNC)
		{
			if (verbosity == -1)
				fprintf_rl(stderr, "Error in module %s: symbol %s found in wasmtime_linker_get() is not a function\n", ctx->module_name, func_name);
			return;
		}

		// Store function symbol data
		ctx->func[func_id] = func_ext.of.func;

		if (verbosity == 1)
			fprintf_rl(stdout, "Module %s: %s() found\n", ctx->module_name, func_name);
	}
}

void wahe_init_all_module_symbols(wahe_module_t *ctx)
{
	wahe_get_module_func(ctx, "module_malloc",        WAHE_FUNC_MALLOC, -1);
	wahe_get_module_func(ctx, "module_realloc",       WAHE_FUNC_REALLOC, -1);
	wahe_get_module_func(ctx, "module_free",          WAHE_FUNC_FREE, -1);
	wahe_get_module_func(ctx, "module_message_input", WAHE_FUNC_INPUT, 1);
	wahe_get_module_func(ctx, "module_proc_cmd",      WAHE_FUNC_PROC_CMD, 1);
	wahe_get_module_func(ctx, "module_draw",          WAHE_FUNC_DRAW, 1);
	wahe_get_module_func(ctx, "module_proc_image",    WAHE_FUNC_PROC_IMAGE, 1);
	wahe_get_module_func(ctx, "module_proc_sound",    WAHE_FUNC_PROC_SOUND, 1);
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

	// Native call
	if (ctx->native)
	{
		void *(*func)(size_t) = ctx->dl_func[WAHE_FUNC_MALLOC];
		return (size_t) func(size);
	}

	// Set params
	param[0] = wasmtime_val_set_address(ctx, size);

	// Call the function
	wahe_bench_point("calling malloc()", 1);
	rl_mutex_lock(&ctx->mutex);
	error = wasmtime_func_call(ctx->context, &ctx->func[WAHE_FUNC_MALLOC], param, 1, ret, 1, &trap);
	rl_mutex_unlock(&ctx->mutex);
	wahe_bench_point("malloc() returned", 0);
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
	wahe_bench_point("Updating with wasmtime_linker_get_memory()", 0);
	wasmtime_linker_get_memory(ctx);
	wahe_bench_point("call_module_malloc() end", -1);

	// Return result
	return wasmtime_val_get_address(ret[0]);
}

size_t call_module_realloc(wahe_module_t *ctx, size_t address, size_t size)
{
	wasmtime_error_t *error;
	wasm_trap_t *trap = NULL;
	wasmtime_val_t ret[1], param[2];

	// Native call
	if (ctx->native)
	{
		void *(*func)(size_t,size_t) = ctx->dl_func[WAHE_FUNC_REALLOC];
		return (size_t) func(address, size);
	}

	// Set params
	param[0] = wasmtime_val_set_address(ctx, address);
	param[1] = wasmtime_val_set_address(ctx, size);

	// Call the function
	wahe_bench_point("calling realloc()", 1);
	rl_mutex_lock(&ctx->mutex);
	error = wasmtime_func_call(ctx->context, &ctx->func[WAHE_FUNC_REALLOC], param, 2, ret, 1, &trap);
	rl_mutex_unlock(&ctx->mutex);
	wahe_bench_point("realloc() returned", 0);
	if (error || trap)
	{
		fprintf_rl(stderr, "call_module_realloc(ctx, %zu) failed\n", size);
		fprint_wasmtime_error(error, trap);
		return 0;
	}

	// Check result type
	if (ret[0].kind != ctx->address_type)
	{
		fprintf_rl(stderr, "call_module_realloc() expected a type %s result\n", ctx->address_type==WASMTIME_I32 ? "int32_t" : "int64_t");
		return 0;
	}

	// Check NULL result
	if (wasmtime_val_get_address(ret[0]) == 0)
	{
		fprintf_rl(stderr, "call_module_realloc(%s, 0x%x, %d) returned NULL\n", ctx->module_name, address, size);
		return 0;
	}

	// Update memory pointer
	wahe_bench_point("Updating with wasmtime_linker_get_memory()", 0);
	wasmtime_linker_get_memory(ctx);
	wahe_bench_point("call_module_realloc() end", -1);

	// Return result
	return wasmtime_val_get_address(ret[0]);
}

void call_module_free(wahe_module_t *ctx, size_t address)
{
	wasmtime_error_t *error;
	wasm_trap_t *trap = NULL;
	wasmtime_val_t param[1];

	// Native call
	if (ctx->native)
	{
		void (*func)(void *) = ctx->dl_func[WAHE_FUNC_FREE];
		func((void *) address);
		return;
	}

	// Set params
	param[0] = wasmtime_val_set_address(ctx, address);

	// Call the function
	wahe_bench_point("calling free()", 1);
	rl_mutex_lock(&ctx->mutex);
	error = wasmtime_func_call(ctx->context, &ctx->func[WAHE_FUNC_FREE], param, 1, NULL, 0, &trap);
	rl_mutex_unlock(&ctx->mutex);
	wahe_bench_point("free() returned", -1);
	if (error || trap)
	{
		fprintf_rl(stderr, "call_module_free(ctx, %zu) failed\n", address);
		fprint_wasmtime_error(error, trap);
		return;
	}
}

char *call_module_func(wahe_module_t *ctx, size_t message_addr, enum wahe_func_id func_id, int call_from_eo)
{
	wasmtime_error_t *error;
	wasm_trap_t *trap = NULL;
	wasmtime_val_t ret[1], param[1];
	size_t ret_msg_addr_s = 0, *ret_msg_addr = &ret_msg_addr_s;
	int current_module;

	wahe_thread_t *thread = wahe_cur_thread;
	current_module = thread->current_module = ctx->module_id;

	if (call_from_eo)
	{
		// Find where to store the return message address
		wahe_exec_order_t *eo = &thread->exec_order[thread->current_eo];
		ret_msg_addr = &eo->ret_msg_addr;
	}

	// Native call
	if (ctx->native)
	{
		char *(*func)(char *) = ctx->dl_func[func_id];
		swap_ptr(&wahe_cur_ctx, &ctx);
		*ret_msg_addr = (size_t) func((char *) message_addr);
		swap_ptr(&wahe_cur_ctx, &ctx);
		return (char *) *ret_msg_addr;
	}

	if (ctx->func[func_id].store_id == 0)
		return NULL;

	// Set params
	param[0] = wasmtime_val_set_address(ctx, message_addr);

	// Call the function
	wahe_bench_point("calling call_module_func()", 1);
	int prev_func = thread->current_func;
	thread->current_func = func_id;
	rl_mutex_lock(&ctx->mutex);
	error = wasmtime_func_call(ctx->context, &ctx->func[func_id], param, 1, ret, 1, &trap);
	rl_mutex_unlock(&ctx->mutex);
	thread->current_func = prev_func;
	wahe_bench_point("call_module_func() returned", -1);
	if (error || trap)
	{
		fprintf_rl(stderr, "call_module_func() failed\n");
		fprint_wasmtime_error(error, trap);
		return NULL;
	}

	// Restore current_module
	thread->current_module = current_module;

	// Store the return message address
	*ret_msg_addr = wasmtime_val_get_address(ret[0]);

	// Update memory pointer
	wasmtime_linker_get_memory(ctx);

	// Return pointer to return message
	if (*ret_msg_addr)
		return (char *) &ctx->memory_ptr[*ret_msg_addr];
	else
		return NULL;
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

int wahe_message_to_raster(wahe_module_t *ctx, size_t msg_addr, raster_t *r)
{
	size_t raster_size = 0, raster_address = 0;

	if (msg_addr == 0)
		return 0;

	int ret_mode = get_raster_mode(*r);

	// Pointer to the message
	char *message = &ctx->memory_ptr[msg_addr];

	// Parse each line of the message
	for (const char *line = message; line; line = strstr_after(line, "\n"))
	{
		char a[32];

		if (sscanf(line, "Pixel format: %31[^\n]", a) == 1)
			ret_mode = wahe_pixel_format_to_raster_mode(a);

		sscanf(line, "Framebuffer location: %zi bytes at %zi", &raster_size, &raster_address);
		sscanf(line, "Framebuffer resolution %dx%d", &r->dim.x, &r->dim.y);
	}

	if (raster_address == 0)
		return 0;

	// Update the host-side raster for the module framebuffer
	*r = make_raster(&ctx->memory_ptr[raster_address], r->dim, r->dim, ret_mode);
	cl_unref_raster(r);

	if (ret_mode == IMAGE_USE_BUF)
		r->buf_size = raster_size;

	return 1;
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
	char *return_msg = call_module_func(ctx, message_addr, WAHE_FUNC_INPUT, 0);
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
	memset(ctx, 0, sizeof(wahe_module_t));
	rl_mutex_init(&ctx->mutex);
	ctx->module_name = sprintf_alloc("%s", get_filename_from_path(path));

	// Native module
	if (ctx->native = dynlib_open(path))
	{
		// Find functions from the native module
		wahe_init_all_module_symbols(ctx);
	}
	// WASM module
	else
	{
		wasmtime_error_t *error;
		wasm_functype_t *func_type;

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
		func_type = wasm_functype_new_1_1(wasm_valtype_new_i32(), wasm_valtype_new_i32());
		error = wasmtime_linker_define_func(ctx->linker, "env", strlen("env"), "wahe_run_command", strlen("wahe_run_command"), func_type, wahe_run_command, parent_group, NULL);
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
		wahe_init_all_module_symbols(ctx);

		// Get pointer to linear memory
		wasmtime_linker_get_memory(ctx);

		// Set the type of module addresses (currently always 32-bit)
		ctx->address_type = WASMTIME_I32;
	}

	// Store module index so we can know which index a given module has
	ctx->parent_group = parent_group;
	ctx->module_id = module_index;

	// Send module ID to the module, it's intended to keep changing in the future on the host side without changes in modules, so modules should just store the ID in a char[61]
	if (parent_group)
		wahe_send_input(ctx, "Module ID 0x%" PRIx64 "->%d", parent_group, module_index);

	// Send pointer to wahe_run_command() if the module is not WASM
	if (ctx->native)
		wahe_send_input(ctx, "wahe_run_command() = %#zx", wahe_run_command_native);

	// Send an Init message to the module
	wahe_send_input(ctx, "Init");

	// Send an Init message to the module
	char *cmd_reg_msg = wahe_send_input(ctx, "Command registration");
	// TODO register commands

	// Init module's textedit used for transmitting text input
	textedit_init(&ctx->input_te, 1);
	ctx->input_te.edit_mode = te_mode_full;
}

wahe_module_t *wahe_get_module_by_id_string(const char *id_string)
{
	size_t group_addr;
	int module_index;

	// Module ID is made of the group's address in host memory plus the module's index
	if (sscanf(id_string, "%zx->%d", &group_addr, &module_index) == 2)
	{
		wahe_group_t *group = (wahe_group_t *) group_addr;
		if (module_index >= group->module_count)
			return NULL;

		wahe_module_t *ctx = &group->module[module_index];

		if (ctx->parent_group != group)
			fprintf_rl(stderr, "Module ID \"%s\" points to a bogus group address.\n", id_string);

		return ctx;
	}

	return NULL;
}

void wahe_copy_between_memories(wahe_module_t *src_module, size_t src_addr, size_t copy_size, wahe_module_t *dst_module, size_t dst_addr)
{
	// Update memory pointers
	wasmtime_linker_get_memory(src_module);
	wasmtime_linker_get_memory(dst_module);

	// TODO Check boundaries

	// Copy
	memcpy(dst_module ? &dst_module->memory_ptr[dst_addr] : (void *) dst_addr, src_module ? &src_module->memory_ptr[src_addr] : (void *) src_addr, copy_size);
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

void wahe_make_keyboard_mouse_messages(wahe_thread_t *thread, int module_id, int display_id, int conn_id)
{
	int i;
	buffer_t buf = {0};
	const char *state_name[] = { "up", "", "", "", "down", "repeat" };
	wahe_group_t *group = thread->parent_group;
	wahe_module_t *ctx = &group->module[module_id];

	// Set textedit if framebuffer is clicked which indicates that the module is active in the interface
	ctrl_button_state_t *butt_state = proc_mouse_rect_ctrl_lrmb(group->image[display_id].fb_rect);
	if (butt_state[0].down || butt_state[1].down)
		cur_textedit = &ctx->input_te;

	// Determine if the control that represents the display is active
	int mouse_active = butt_state[0].orig || butt_state[0].over || butt_state[1].orig || butt_state[1].over;
	int kb_active = (cur_textedit == &ctx->input_te);

	// Send text input
	if (ctx->input_te.string[0])
	{
		bufprintf(&buf, "Text input (0@) ");

		// Convert to 0@ format
		size_t len = strlen(ctx->input_te.string);
		for (int i=0; i < len; i++)
		{
			uint8_t c = ctx->input_te.string[i];
			bufprintf(&buf, "%c%c", '0' + (c >> 5), '@' + (c & 0x1F));
		}
		bufprintf(&buf, "\n");

		// Clear textedit
		textedit_clear_then_set_new_text(&ctx->input_te, NULL);
	}

	// Go through all keys looking for newly pressed or released keys
	if (kb_active)
	for (i = RL_SCANCODE_A; i < RL_NUM_SCANCODES; i++)
	{
		if (abs(mouse.key_state[i]) >= 2)
		{
			bufprintf(&buf, "Key %s: %d", state_name[2 + mouse.key_state[i]], i);

		#ifdef RL_SDL
			bufprintf(&buf, " / \"%s\" / \"%s\"", SDL_GetScancodeName(i), SDL_GetKeyName(SDL_GetKeyFromScancode(i)));
		#endif
			bufprintf(&buf, "\n");
		}
	}

	// Make mouse messages depending on the target display
	if (mouse_active)
	{
		xy_t r_scale, r_offset;
		rect_range_and_dim_to_scale_offset_inv(group->image[display_id].fb_rect, group->image[display_id].fb.dim, &r_scale, &r_offset, 0);
		xy_t pix_pos = mad_xy(mouse.u, r_scale, r_offset);

if (mouse.b.lmb != -1 || mouse.b.rmb != -1)
		bufprintf(&buf, "Mouse position (pixels) %.16g %.16g\n", pix_pos.x, pix_pos.y);
	}
	else if (group->image[display_id].mouse_active)
		bufprintf(&buf, "Mouse position (pixels) NAN NAN\n");

	// Mouse buttons
	for (i=0; i < 3; i++)
	{
		int b;
		const char *b_name[] = { "left", "middle", "right" };

		switch (i)
		{
				case 0: b = mouse.b.lmb;
			break;	case 1: b = mouse.b.mmb;
			break;	case 2: b = mouse.b.rmb;
		}

		if (abs(b) == 2)
			bufprintf(&buf, "Mouse %s button %s\n", b_name[i], state_name[2 + b]);
	}

	// Copy message from host memory to module memory
	size_t *addr = &thread->exec_order[ thread->connection[conn_id].dst_eo ].dst_msg_addr;
	call_module_free(&group->module[module_id], *addr);
	*addr = 0;

	if (buf.buf)
	{
		*addr = call_module_malloc(&group->module[module_id], buf.len + 1);
		memcpy(&group->module[module_id].memory_ptr[*addr], buf.buf, buf.len + 1);
		free_buf(&buf);
	}

	// Remember the active statuses
	group->image[display_id].mouse_active = mouse_active;
	group->image[display_id].kb_active = kb_active;
}

wahe_shared_buffer_t *wahe_add_or_find_shared_buffer(wahe_group_t *group, const char *name)
{
	int i;
	uint64_t name_hash = get_string_hash(name);

	rl_mutex_lock(&group->shared_buffer_mutex);

	// Look for an existing buffer
	for (i=0; i < group->shared_buffer_count; i++)
		if (group->shared_buffer[i].name_hash == name_hash)
		{
			rl_mutex_unlock(&group->shared_buffer_mutex);
			return &group->shared_buffer[i];
		}

	// Add the buffer
	alloc_enough(&group->shared_buffer, group->shared_buffer_count+=1, &group->shared_buffer_as, sizeof(wahe_shared_buffer_t), 1.5);
	wahe_shared_buffer_t *sb = &group->shared_buffer[group->shared_buffer_count-1];
	sb->name = make_string_copy(name);
	sb->name_hash = name_hash;
	rl_mutex_init(&sb->mutex);

	rl_mutex_unlock(&group->shared_buffer_mutex);

	return sb;
}

// Get called from the module
size_t wahe_run_command_core(wahe_module_t *ctx, char *message)
{
	wahe_group_t *group = NULL;
	size_t return_msg_addr = 0;
	int n, start, end;

	if (message == NULL)
		return 0;

	wahe_thread_t *thread = wahe_cur_thread;
	if (ctx)
		group = ctx->parent_group;

	// Execute command processors for this execution order
	if (thread && thread->current_eo >= 0 && thread->exec_order)
	{
		wahe_exec_order_t *eo = &thread->exec_order[thread->current_eo];
		if (eo && eo->cmd_proc_id && thread->current_cmd_proc_id < eo->cmd_proc_count)
		{
			int dst_module_id = eo->cmd_proc_id[thread->current_cmd_proc_id];
			wahe_module_t *dst_module = &group->module[dst_module_id];

			// Copy message to cmd processing module
			size_t len = strlen(message) + 1;
			size_t dst_addr = call_module_malloc(dst_module, len);
			memcpy(&dst_module->memory_ptr[dst_addr], message, len);

			// Call cmd processing function
			thread->current_cmd_proc_id++;
			size_t return_msg_addr_dst = (size_t) call_module_func(dst_module, dst_addr, WAHE_FUNC_PROC_CMD, 0);
			thread->current_module = ctx->module_id;
			if (return_msg_addr_dst)
				return_msg_addr_dst -= (size_t) dst_module->memory_ptr;
			call_module_free(dst_module, dst_addr);

			if (thread->current_cmd_proc_id == eo->cmd_proc_count)
				thread->current_cmd_proc_id = 0;

			// Copy and return the return message
			if (return_msg_addr_dst)
			{
				size_t ret_len = strlen(&dst_module->memory_ptr[return_msg_addr_dst]) + 1;
				return_msg_addr = call_module_malloc(ctx, ret_len);
				wahe_copy_between_memories(dst_module, return_msg_addr_dst, ret_len, &group->module[eo->module_id], return_msg_addr);
				call_module_free(dst_module, return_msg_addr_dst);
				return return_msg_addr;
			}
			else
				return 0;
		}
	}

	// Parse each line of the message
	for (const char *line = message; line; line = strstr_after(line, "\n"))
	{
		int done = 0;

		// Identify calling module and its group
		start = end = 0;
		sscanf(line, "From module ID %n%*[^\n]%n", &start, &end);
		if (end)
		{
			ctx = wahe_get_module_by_id_string(&line[start]);
			if (ctx)
			{
				group = ctx->parent_group;
				done = 1;
			}
		}

		// Skip parsing the line if the module hasn't been identified yet
		if (group == NULL || ctx == NULL)
		{
			fprintf_rl(stderr, "Module calling wahe_run_command() unidentified, run the command 'From module ID <module_id>' first.\n");
			goto loop_end;
		}

		// Run thread
		n = start = end = 0;
		sscanf(line, "Run thread %n%*[^\n]%n\n%n", &start, &end, &n);
		if (end)
		{
			wahe_thread_t *thread_to_run = NULL;
			char *name = make_string_copy_len(&line[start], end-start);

			for (int i=0; i < group->thread_count; i++)
				if (group->thread[i].thread_name && strcmp(group->thread[i].thread_name, name) == 0)
					thread_to_run = &group->thread[i];

			if (thread_to_run)
			{
				// Point to the thread_input_msg
				const char *input_msg = NULL;
				if (n)
					input_msg = &line[n];

				// Execute thread and get the last message
				char *end_msg = wahe_execute_thread(thread_to_run, input_msg);

				// Copy the last message from the thread to give it to the caller
				if (end_msg)
					return_msg_addr = module_sprintf_alloc(ctx, "%s", end_msg);
			}
			else
				fprintf(stderr, "The 'Run thread' command from %s:%s could not be executed because the thread named '%s' couldn't be found.\n", ctx->module_name, thread ? wahe_func_name[thread->current_func] : "(?)", name);
			free(name);

			return return_msg_addr;
		}

		// Copy buffer between memories
		char src_module_id[61], dst_module_id[61];
		size_t src_addr, copy_size, dst_addr;
		if (sscanf(line, "Copy %zi bytes at %zi (module %60[^)]) to %zi (module %60[^)])", &copy_size, &src_addr, src_module_id, &dst_addr, dst_module_id) == 5)
		{
			wahe_module_t *src_ctx = wahe_get_module_by_id_string(src_module_id);
			wahe_module_t *dst_ctx = wahe_get_module_by_id_string(dst_module_id);

			if (src_ctx && dst_ctx)
			{
				wahe_copy_between_memories(src_ctx, src_addr, copy_size, dst_ctx, dst_addr);
				done = 1;
			}
		}

		// Copy buffer between memories with allocation of destination
		if (sscanf(line, "Copy %zi bytes at %zi (module %60[^)]) to module %60[^)]", &copy_size, &src_addr, src_module_id, dst_module_id) == 4)
		{
			wahe_module_t *dst_ctx = wahe_get_module_by_id_string(dst_module_id);

			if (dst_ctx)
			{
				dst_addr = call_module_malloc(dst_ctx, copy_size);
				wahe_copy_between_memories(wahe_get_module_by_id_string(src_module_id), src_addr, copy_size, dst_ctx, dst_addr);
				return_msg_addr = module_sprintf_alloc(ctx, "Destination %#zx", (void *) dst_addr);
				done = 1;
			}
		}

		// Sync module buffer to shared buffer
		start = end = 0;
		size_t offset = 0;
		sscanf(line, "Sync %zi bytes at %zi (module %60[^)]) to shared buffer %n%*s%n (offset %zu)", &copy_size, &src_addr, src_module_id, &start, &end, &offset);
		if (end)
		{
			char *name = make_string_copy_len(&line[start], end-start);
			wahe_shared_buffer_t *sb = wahe_add_or_find_shared_buffer(group, name);
			free(name);
			wahe_module_t *src_ctx = wahe_get_module_by_id_string(src_module_id);

			rl_mutex_lock(&sb->mutex);
			buf_alloc_enough(&sb->buffer, offset+copy_size);
			sb->buffer.len = sb->buffer.as;
			wahe_copy_between_memories(src_ctx, src_addr, copy_size, NULL, (size_t) &sb->buffer.buf[offset]);
			rl_mutex_unlock(&sb->mutex);

			done = 1;
		}

		// Sync shared buffer to module buffer
		size_t orig_size;
		dst_module_id[0] = '\0';
		sscanf(line, "Sync shared buffer %n%*s%n to %zi bytes at %zi (module %60[^)])", &start, &end, &orig_size, &dst_addr, dst_module_id);
		if (dst_module_id[0])
		{
			char *name = make_string_copy_len(&line[start], end-start);
			wahe_shared_buffer_t *sb = wahe_add_or_find_shared_buffer(group, name);
			free(name);
			wahe_module_t *dst_ctx = wahe_get_module_by_id_string(dst_module_id);

			rl_mutex_lock(&sb->mutex);
			if (sb->buffer.len)
			{
				if (orig_size < sb->buffer.len)
					dst_addr = call_module_realloc(dst_ctx, dst_addr, sb->buffer.len);

				wahe_copy_between_memories(NULL, (size_t) sb->buffer.buf, sb->buffer.len, dst_ctx, dst_addr);
			}
			rl_mutex_unlock(&sb->mutex);

			return_msg_addr = module_sprintf_alloc(ctx, "Buffer location: %zi bytes at %#zx", sb->buffer.len, (void *) dst_addr);
			done = 1;
		}

		// Load raw file
		start = end = 0;
		sscanf(line, "Load raw file at path %n%*[^\n]%n", &start, &end);
		if (end)
		{
			size_t data_addr, data_size = 0;
			char *path = make_string_copy_len(&line[start], end-start);
			data_addr = wahe_load_raw_file(ctx, path, &data_size);
			free(path);
			return_msg_addr = module_sprintf_alloc(ctx, "Data location: %zi bytes at %#zx", data_size, (void *) data_addr);
			done = 1;
		}

		// Return raw time
		n = 0;
		sscanf(line, "Get raw time%n", &n);
		if (n)
		{
			return_msg_addr = module_sprintf_alloc(ctx, "Raw time %.17g seconds", get_time_hr());
			done = 1;
		}

		// Benchmark return
		n = 0;
		sscanf(line, "Benchmark%n", &n);
		if (n)
		{
			wahe_bench_point("-Benchmark command-", 0);
			done = 1;
		}

		// Print to host
		n = 0;
		sscanf(line, "Print%n", &n);
		if (n && (line[n] == ' ' || line[n] == '\n'))
		{
			if (get_string_linecount(&line[n+1], 0) > 1)
				fprintf_rl(stdout, "\n=== from %s:%s ===\n%s\n    ===    ===    \n\n",
						ctx->module_name, thread ? wahe_func_name[thread->current_func] : "(?)", &line[n+1]);
			else
				fprintf_rl(stdout, "(from %s:%s)   %s\n",
						ctx->module_name, thread ? wahe_func_name[thread->current_func] : "(?)", &line[n+1]);
			return 0;
		}

loop_end:
		// Print line if it wasn't interpreted
		if (done == 0)
		{
			int line_len = strlen(line);
			if (strstr(line, "\n"))
				line_len = strstr(line, "\n") - line;
			fprintf_rl(stderr, "Command from %s:%s not interpreted: %.*s\n",
					ctx ? ctx->module_name : "(?)", 
					thread ? wahe_func_name[thread->current_func] : "(?)",
					line_len, line);
		}
	}

	return return_msg_addr;
}

char *wahe_run_command_native(char *message)
{
	return (char *) wahe_run_command_core(wahe_cur_ctx, message);
}

wasm_trap_t *wahe_run_command(void *env, wasmtime_caller_t *caller, const wasmtime_val_t *arg, size_t arg_count, wasmtime_val_t *result, size_t result_count)
{
	wahe_group_t *group = env;
	wahe_module_t *ctx = &group->module[wahe_cur_thread->current_module];
	size_t return_msg_addr = 0;
	int n;

	// Update memory pointer
	wasmtime_linker_get_memory(ctx);

	// Parse message
	if (wasmtime_val_get_address(arg[0]))
	{
		// Get the message address and make pointer to the message
		char *message = &ctx->memory_ptr[wasmtime_val_get_address(arg[0])];
		if (wasmtime_val_get_address(arg[0]) == 0)
			message = NULL;

		// Run the command
		return_msg_addr = wahe_run_command_core(ctx, message);
	}

	// Return message
	result[0] = wasmtime_val_set_address(ctx, return_msg_addr);

	return NULL;
}
