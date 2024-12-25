int get_scancode_by_key_name(const char *name)
{
	#ifdef RL_SDL
	#if RL_SDL == 2
	return SDL_GetScancodeFromKey(SDL_GetKeyFromName(name));
	#else
	return SDL_GetScancodeFromKey(SDL_GetKeyFromName(name), NULL);
	#endif
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

int get_key_state_fusion(int sc1, int sc2, int sc3)
{
	int key[3];

	// Load the key values
	key[0] = mouse.key_state[sc1];
	key[1] = mouse.key_state[sc2];
	key[2] = mouse.key_state[sc3];

	// Sort them (lowest ends up first)
	qsort(key, 3, sizeof(int), cmp_int);

	// Apply state fusion logic
	if (key[2] == 3 || key[2] == 1)			// if one of the keys is held down
		return key[2];				// return its state

	if (key[2] == 2)				// if a key is newly down
		if (key[1] != 1 && key[0] != 1)		// and other keys aren't held down
			return 2;			// return newly down
		else					// if other keys are already held down
			return 1;			// return down

	return key[0];		// either -2 or -1
}

int get_kb_shift()  { return get_key_state_fusion(RL_SCANCODE_LSHIFT, RL_SCANCODE_RSHIFT, RL_SCANCODE_UNKNOWN); }
int get_kb_ctrl()   { return get_key_state_fusion(RL_SCANCODE_LCTRL, RL_SCANCODE_RCTRL, RL_SCANCODE_UNKNOWN); }
int get_kb_guikey() { return get_key_state_fusion(RL_SCANCODE_LGUI, RL_SCANCODE_RGUI, RL_SCANCODE_UNKNOWN); }
int get_kb_alt()    { return get_key_state_fusion(RL_SCANCODE_LALT, RL_SCANCODE_RALT, RL_SCANCODE_UNKNOWN); }
int get_kb_enter()  { return get_key_state_fusion(RL_SCANCODE_RETURN, RL_SCANCODE_RETURN2, RL_SCANCODE_KP_ENTER); }
int get_kb_all_mods() { return (get_kb_shift()!=-1) + (get_kb_ctrl()!=-1) + (get_kb_guikey()!=-1) + (get_kb_alt()!=-1); }

void flag_update_keyboard_button(int8_t *b, int8_t *quick_b)
{
	flag_update(*b);

	if (*quick_b)
	{
		*b = *quick_b * 2;
		*quick_b = 0;
	}
}

void keyboard_pre_event_proc(mouse_t *mouse)
{
	int i;

	for (i=0; i < RL_NUM_SCANCODES; i++)
		flag_update_keyboard_button(&mouse->key_state[i], &mouse->key_quick[i]);
}

void keyboard_button_event(int8_t *b, int8_t *quick_b, int way, int repeat)
{
	if (*b * way == -2)		// if quick press situation
		*quick_b = way;
	else
		*b = (2 + repeat) * way;
}
