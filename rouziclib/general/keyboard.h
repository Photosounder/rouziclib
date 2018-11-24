// key names at https://wiki.libsdl.org/SDL_Keycode

extern int get_scancode_by_key_name(const char *name);
extern int get_key_state_by_name(const char *name);
extern void keyboard_pre_event_proc(mouse_t *mouse);
extern void keyboard_button_event(int *b, int *quick_b, int way, int repeat);
extern void zoom_keyboard_control(zoom_t *zc, int *flag_zoom_key);

#define get_kb_shift()	(mouse.key_state[RL_SCANCODE_LSHIFT] | mouse.key_state[RL_SCANCODE_RSHIFT])
#define get_kb_ctrl()	(mouse.key_state[RL_SCANCODE_LCTRL] | mouse.key_state[RL_SCANCODE_RCTRL])
#define get_kb_guikey()	(mouse.key_state[RL_SCANCODE_LGUI] | mouse.key_state[RL_SCANCODE_RGUI])
#define get_kb_alt()	(mouse.key_state[RL_SCANCODE_LALT] | mouse.key_state[RL_SCANCODE_RALT])
#define get_kb_all_mods()	(get_kb_shift() | get_kb_ctrl() | get_kb_guikey() | get_kb_alt())
#define get_kb_enter()	(mouse.key_state[RL_SCANCODE_RETURN] | mouse.key_state[RL_SCANCODE_RETURN2] | mouse.key_state[RL_SCANCODE_KP_ENTER])
