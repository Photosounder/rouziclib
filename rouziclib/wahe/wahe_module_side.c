void wahe_rl_parse_inputs(const char *line, int *received_input)
{
	int i, n;

	// Generic user input
	// Mouse position
	xy_t pos;
	if (sscanf(line, "Mouse position (pixels) %lg %lg", &pos.x, &pos.y) == 2)
	{
		*received_input = 1;

		if (isnan_xy(pos))
		{
			pos = set_xy(1e30);
			*received_input = 0;
		}

		mouse.d = sub_xy(pos, mouse.a);
		mouse.a = pos;
		mouse.u = wc_xy(pos);
	}

	//if (sscanf(line, "Mouse delta %lg %lg", &pos.x, &pos.y) == 2)
	//	mouse.d = pos;

	// Mouse buttons
	char mb_name[7], mb_state_name[7];
	if (sscanf(line, "Mouse %6s button %6s", mb_name, mb_state_name) == 2)
	{
		int new_state = (strcmp(mb_state_name, "down")==0) - (strcmp(mb_state_name, "up")==0);

		if (strcmp(mb_name, "left") == 0)
			if (new_state * mouse.b.lmb <= 0)
				mouse_button_event(&mouse.b.lmb, &mouse.b.lmf, new_state);

		if (strcmp(mb_name, "middle") == 0)
			if (new_state * mouse.b.mmb <= 0)
				mouse_button_event(&mouse.b.mmb, &mouse.b.mmf, new_state);

		if (strcmp(mb_name, "right") == 0)
			if (new_state * mouse.b.rmb <= 0)
				mouse_button_event(&mouse.b.rmb, &mouse.b.rmf, new_state);

		*received_input = 1;
	}

	// Text input
	n = 0;
	int n2 = 0;
	sscanf(line, "Text input (0@) %n%*s%n", &n, &n2);
	if (n2)
	{
		size_t len = (n2 - n) / 2;
		char *text_input = calloc(len + 1, sizeof(char));

		for (i=0 ; i < len; i++)
			text_input[i] = (line[n+i*2] - '0' << 5) + line[n+i*2+1] - '@';

		textedit_add(cur_textedit, text_input);

		free(text_input);
		*received_input = 1;
	}

	// Key presses
	int scancode;
	char key_state_name[7];
	if (sscanf(line, "Key %6[^:]: %d", key_state_name, &scancode) == 2)
	{
		if (scancode > 0 && scancode < RL_NUM_SCANCODES)
		{
			if (strcmp(key_state_name, "up") == 0)     mouse.key_state[scancode] = -2;
			if (strcmp(key_state_name, "down") == 0)   mouse.key_state[scancode] = 2;
			if (strcmp(key_state_name, "repeat") == 0) mouse.key_state[scancode] = 3;
		}

		*received_input = 1;
	}
}

void wahe_rl_module_init(xyi_t fb_dim, const int mode)
{
	fb->pixel_scale = 1;
	zc = init_zoom(&mouse, drawing_thickness);
	calc_screen_limits(&zc);
	mouse = init_mouse();
	mouse.mouse_focus_flag = 1;
	mouse.window_focus_flag = 1;
	mouse.zoom_allowed = 0;
	mouse.a = set_xy(1e30);

#ifdef RL_INCL_VECTOR_TYPE_FILEBALL
	vector_font_load_from_header();
#else
	make_fallback_font(font);
#endif

	// Prepare framebuffer
	fb->r.dim = fb_dim;
	fb->r = make_raster(NULL, fb->r.dim, fb->r.dim, mode);
	fb->w = fb->r.dim.x;
	fb->h = fb->r.dim.y;
	calc_screen_limits(&zc);
}
