int get_scancode_by_key_name(const char *name)
{
	#ifdef RL_SDL
	return SDL_GetScancodeFromKey(SDL_GetKeyFromName(name));
	#endif

	return -1;
}

int get_key_state_by_name(const char *name)
{
	#ifdef RL_SDL
	int scancode;
	
	scancode = get_scancode_by_key_name(name);

	if (scancode > SDL_SCANCODE_UNKNOWN && scancode < SDL_NUM_SCANCODES)
		return mouse.key_state[scancode];
	else
		fprintf_rl(stderr, "Key by the name \"%s\" not found.\n", name);
	#endif

	return -1;
}

int get_kb_shift()  { return mouse.key_state[RL_SCANCODE_LSHIFT] | mouse.key_state[RL_SCANCODE_RSHIFT]; }
int get_kb_ctrl()   { return mouse.key_state[RL_SCANCODE_LCTRL]  | mouse.key_state[RL_SCANCODE_RCTRL]; }
int get_kb_guikey() { return mouse.key_state[RL_SCANCODE_LGUI]   | mouse.key_state[RL_SCANCODE_RGUI]; }
int get_kb_alt()    { return mouse.key_state[RL_SCANCODE_LALT]   | mouse.key_state[RL_SCANCODE_RALT]; }
int get_kb_enter()  { return mouse.key_state[RL_SCANCODE_RETURN] | mouse.key_state[RL_SCANCODE_RETURN2] | mouse.key_state[RL_SCANCODE_KP_ENTER]; }
int get_kb_all_mods() { return get_kb_shift() | get_kb_ctrl() | get_kb_guikey() | get_kb_alt(); }

void flag_update_keyboard_button(int *b, int *quick_b)
{
	if (*b >= 2)
		*b = 1;

	if (*quick_b)
	{
		*b = *quick_b==1 ? 2 : 0;
		*quick_b = 0;
	}
}

void keyboard_pre_event_proc(mouse_t *mouse)
{
	int i;

	for (i=0; i < RL_NUM_SCANCODES; i++)
		flag_update_keyboard_button(&mouse->key_state[i], &mouse->key_quick[i]);
}

void keyboard_button_event(int *b, int *quick_b, int way, int repeat)
{
	if (*b * way <= -2)		// if quick press situation
		*quick_b = way;
	else
		*b = (2 + repeat) * (way==1);
}

void zoom_keyboard_control(zoom_t *zc, int *flag_zoom_key)
{
	xy_t move_vector=XY0;
	const double rep_inc=0.25;
	int s;

	if (zc->mouse->zoom_allowed && get_kb_alt())
	{
		// Toggle or reset zoom with (Shift-)Alt-Z
		if (zc->mouse->key_state[RL_SCANCODE_Z]==2)
			if (get_kb_shift()==0)
				zoom_toggle(zc, flag_zoom_key);
			else
				zoom_reset(zc, flag_zoom_key);

		// Scroll with Alt-WASD
		if ((s=zc->mouse->key_state[RL_SCANCODE_A]) >= 2)	move_vector.x -= s==2 ? 1. : rep_inc;
		if ((s=zc->mouse->key_state[RL_SCANCODE_D]) >= 2)	move_vector.x += s==2 ? 1. : rep_inc;
		if ((s=zc->mouse->key_state[RL_SCANCODE_S]) >= 2)	move_vector.y -= s==2 ? 1. : rep_inc;
		if ((s=zc->mouse->key_state[RL_SCANCODE_W]) >= 2)	move_vector.y += s==2 ? 1. : rep_inc;

		if (is0_xy(move_vector)==0)
		{
			zc->offset_u = add_xy(zc->offset_u, mul_xy(move_vector, set_xy(4./zc->zoomscale)));	// move by steps of 4 units
			calc_screen_limits(zc);
		}

		// Zoom in or out
		if (zc->mouse->key_state[RL_SCANCODE_Q]==2)	zoom_wheel(zc, 1, -1);
		if (zc->mouse->key_state[RL_SCANCODE_E]==2)	zoom_wheel(zc, 1, 1);
	}
}
