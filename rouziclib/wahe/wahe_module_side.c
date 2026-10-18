void wahe_rl_input_init(void)
{
	// Initialize GUI input independently of framebuffer allocation
	if (mouse.ctrl_id)
		return;
	init_mouse();
	mouse.mouse_focus_flag = mouse.window_focus_flag = 1;
	zc = init_zoom(&mouse, drawing_thickness);
}

void wahe_rl_input_deinit(void)
{
	// Release input bookkeeping without owning active editors or framebuffers
	if (mouse.ctrl_id)
	{
		free(mouse.ctrl_id->stack);
		free(mouse.ctrl_id);
	}
	memset(&mouse, 0, sizeof(mouse));
	memset(&zc, 0, sizeof(zc));
	cur_textedit = prev_textedit = next_textedit = NULL;
}

void wahe_rl_frame_begin(framebuffer_t *target)
{
	// Refresh the target transform without resetting the view and process pending input
	wahe_rl_input_init();
	fb = target;
	calc_screen_limits(&zc);
	textedit_add(cur_textedit, NULL);
	mouse_post_event_proc(&mouse, &zc);
}

void wahe_rl_frame_end(void)
{
	// Advance input edges and clear accumulated motion after the frame consumes them
	mouse_pre_event_proc(&mouse);
	keyboard_pre_event_proc(&mouse);
	mouse.warp_prev = mouse.warp;
}

int wahe_rl_get_scancode_by_key_name(const char *name)
{
	// Resolve printable keys without requiring a native keyboard backend
	if (name == NULL || name[0] == '\0')
		return -1;
	if (name[1] == '\0')
	{
		int c = tolower((unsigned char) name[0]);
		if (c >= 'a' && c <= 'z') return RL_SCANCODE_A + c - 'a';
		if (c >= '1' && c <= '9') return RL_SCANCODE_1 + c - '1';
		if (c == '0') return RL_SCANCODE_0;
		if (c == ' ') return RL_SCANCODE_SPACE;
	}

	// Resolve function keys and standard navigation and modifier names
	int function = 0, end = 0;
	if (name[0] == 'f' || name[0] == 'F')
		sscanf(name + 1, "%d%n", &function, &end);
	if (end && !name[end + 1])
	{
		if (function >= 1 && function <= 12) return RL_SCANCODE_F1 + function - 1;
		if (function >= 13 && function <= 24) return RL_SCANCODE_F13 + function - 13;
	}
	static const struct { const char *name; int scancode; } keys[] = {
		{"Return", RL_SCANCODE_RETURN}, {"Enter", RL_SCANCODE_RETURN},
		{"Escape", RL_SCANCODE_ESCAPE}, {"Backspace", RL_SCANCODE_BACKSPACE},
		{"Tab", RL_SCANCODE_TAB}, {"Space", RL_SCANCODE_SPACE},
		{"Delete", RL_SCANCODE_DELETE}, {"Insert", RL_SCANCODE_INSERT},
		{"Home", RL_SCANCODE_HOME}, {"End", RL_SCANCODE_END},
		{"PageUp", RL_SCANCODE_PAGEUP}, {"PageDown", RL_SCANCODE_PAGEDOWN},
		{"Left", RL_SCANCODE_LEFT}, {"Right", RL_SCANCODE_RIGHT},
		{"Up", RL_SCANCODE_UP}, {"Down", RL_SCANCODE_DOWN},
		{"Left Ctrl", RL_SCANCODE_LCTRL}, {"Right Ctrl", RL_SCANCODE_RCTRL},
		{"Left Shift", RL_SCANCODE_LSHIFT}, {"Right Shift", RL_SCANCODE_RSHIFT},
		{"Left Alt", RL_SCANCODE_LALT}, {"Right Alt", RL_SCANCODE_RALT},
		{"Left GUI", RL_SCANCODE_LGUI}, {"Right GUI", RL_SCANCODE_RGUI}
	};
	for (size_t i = 0; i < sizeof(keys) / sizeof(*keys); i++)
		if (!stricmp(name, keys[i].name))
			return keys[i].scancode;
	return -1;
}

const char *wahe_rl_parse_inputs(const char *line, int *received_input)
{
	// Ignore unrelated commands without allocating GUI state for queries
	if (line == NULL)
		return NULL;
	const char *next = strchr(line, '\n');
	if (next) next++;
	int prefix = 0;
	sscanf(line, "Mouse%n", &prefix);
	if (!prefix) sscanf(line, "Key%n", &prefix);
	if (!prefix) sscanf(line, "Text input%n", &prefix);
	if (!prefix) sscanf(line, "Clipboard contents%n", &prefix);
	if (!prefix) return next;
	wahe_rl_input_init();

	// Consume counted UTF-8 payloads without interpreting their lines as commands
	size_t line_count = 0;
	int n = 0;
	sscanf(line, "Text input %zu lines%n", &line_count, &n);
	if (!n) sscanf(line, "Clipboard contents %zu lines%n", &line_count, &n);
	if (n)
	{
		buffer_t text = {0};
		const char *payload = next;
		for (size_t i = 0; i < line_count && payload; i++)
		{
			// Preserve payload line breaks but exclude the final framing newline
			const char *end = strchr(payload, '\n');
			if (i) bufprintf(&text, "\n");
			bufwrite(&text, (const uint8_t *) payload, end ? (size_t) (end - payload) : strlen(payload));
			payload = end ? end + 1 : NULL;
		}
		textedit_add(cur_textedit, text.buf ? (const char *) text.buf : "");
		free_buf(&text);
		if (received_input) *received_input = 1;
		return payload;
	}

	// Preserve quick press and release transitions through consecutive frame edges
	int scancode;
	char state[7];
	if (sscanf(line, "Key %6[^:]: %d", state, &scancode) == 2)
	{
		if (scancode > 0 && scancode < RL_NUM_SCANCODES)
		{
			int way = !strcmp(state, "up") ? -1 : (!strcmp(state, "down") || !strcmp(state, "repeat"));
			if (way)
				keyboard_button_event(&mouse.key_state[scancode], &mouse.key_quick[scancode], way, !strcmp(state, "repeat"));
		}
		if (received_input) *received_input = 1;
		return next;
	}

	// Clear held keys and persistent capture when keyboard focus is lost
	n = 0;
	sscanf(line, "Keyboard focus lost%n", &n);
	if (n)
	{
		memset(mouse.key_state, -1, sizeof(mouse.key_state));
		memset(mouse.key_quick, 0, sizeof(mouse.key_quick));
		mouse.window_focus_flag = -2;
		mouse.warp = mouse.warp_persistent = 0;
		cur_textedit = NULL;
		if (received_input) *received_input = 1;
		return next;
	}

	// Accumulate absolute motion while preferring the explicit relative stream when supplied
	xy_t pos;
	if (sscanf(line, "Mouse position (pixels) %lg %lg", &pos.x, &pos.y) == 2)
	{
		if (!isfinite(pos.x) || !isfinite(pos.y))
		{
			mouse.mouse_focus_flag = -2;
			mouse.a = set_xy(1e30);
			mouse.position_received = 0;
		}
		else
		{
			if (mouse.position_received && mouse.mouse_focus_flag >= 0)
				mouse.d_absolute = add_xy(mouse.d_absolute, sub_xy(pos, mouse.a));
			mouse.a = pos;
			mouse.position_received = 1;
			mouse.mouse_focus_flag = mouse.window_focus_flag = 1;
			if (zc.scrscale > 0.) mouse.u = wc_xy(pos);
		}
		mouse.d = mouse.d_relative_received ? mouse.d_relative : mouse.d_absolute;
		if (received_input) *received_input = 1;
		return next;
	}
	if (sscanf(line, "Mouse delta %lg %lg", &pos.x, &pos.y) == 2)
	{
		mouse.d_relative = add_xy(mouse.d_relative, pos);
		mouse.d_relative_received = 1;
		mouse.d = mouse.d_relative;
		if (received_input) *received_input = 1;
		return next;
	}

	// Keep explicit capture separate from temporary knob cursor hiding
	n = 0;
	sscanf(line, "Mouse capture%n", &n);
	if (n)
	{
		mouse.warp = mouse.warp_persistent = 1;
		mouse.b.orig = zc.offset_u;
		if (received_input) *received_input = 1;
		return next;
	}
	n = 0;
	sscanf(line, "Mouse release%n", &n);
	if (n)
	{
		mouse.warp = mouse.warp_persistent = 0;
		mouse.b.orig = mouse.u = zc.offset_u;
		if (received_input) *received_input = 1;
		return next;
	}

	// Apply button transitions through the regular mouse event helper
	char button[7];
	if (sscanf(line, "Mouse %6s button %6s", button, state) == 2)
	{
		int way = !strcmp(state, "down") ? 1 : (!strcmp(state, "up") ? -1 : 0);
		if (way)
		{
			if (!strcmp(button, "left")) mouse_button_event(&mouse.b.lmb, &mouse.b.lmf, way);
			if (!strcmp(button, "middle")) mouse_button_event(&mouse.b.mmb, &mouse.b.mmf, way);
			if (!strcmp(button, "right")) mouse_button_event(&mouse.b.rmb, &mouse.b.rmf, way);
		}
		if (received_input) *received_input = 1;
		return next;
	}

	// Accumulate wheel movement and apply only the newly received zoom increment
	char direction[5];
	double increments;
	if (sscanf(line, "Mouse scroll %4s %lg", direction, &increments) == 2)
	{
		int wheel = nearbyint(increments) * (!strcmp(direction, "up") ? 1 : -1);
		mouse.b.wheel += wheel;
		if (zc.scrscale > 0.) zoom_wheel(&zc, mouse.zoom_flag, wheel);
		if (received_input) *received_input = 1;
	}
	return next;
}

void wahe_rl_module_init(xyi_t fb_dim, const int mode)
{
	// Allocate the drawing target before calculating its initial transform
	if (fb->w)
		return;
	wahe_rl_input_init();
	fb->pixel_scale = 1;
	fb->r = make_raster(NULL, fb_dim, fb_dim, mode);
	fb->w = fb_dim.x;
	fb->h = fb_dim.y;
	calc_screen_limits(&zc);
	mouse.a = set_xy(1e30);

	// Load the standard vector font for GUI controls
	#ifdef RL_INCL_VECTOR_TYPE_FILEBALL
	vector_font_load_from_header();
	#else
	make_fallback_font(font = init_font());
	#endif
}
