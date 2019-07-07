mouse_t init_mouse()
{
	mouse_t mouse;

	memset(&mouse, 0, sizeof(mouse_t));
	mouse.ctrl_id = calloc(1, sizeof(mouse_ctrl_id_t));

	mouse.b.lmb = mouse.b.mmb = mouse.b.rmb = -1;
	mouse.zoom_allowed = 1;

	return mouse;
}

void flag_update_mouse_button(int *mb, int *quick_mb)
{
	flag_update(*mb);

	if (*quick_mb)
	{
		*mb = *quick_mb * 2;
		*quick_mb = 0;
	}
}

void mouse_pre_event_proc(mouse_t *mouse)
{
	mouse->d = XY0;
	mouse->prev_u = mouse->u_stored;
	mouse->b.wheel = 0;
	if (mouse->b.lmb <= 0)						// if LMB is being pressed the hovered ID stays the same as before no matter what
		mouse->ctrl_id->hover = mouse->ctrl_id->hover_new;
	memset(&mouse->ctrl_id->hover_new, 0, sizeof(ctrl_id_t));
	memset(&mouse->ctrl_id->current, 0, sizeof(ctrl_id_t));
	mouse->ctrl_id->hover_box_matched = 0;

	flag_update_mouse_button(&mouse->b.lmb, &mouse->b.quick_lmb);
	flag_update_mouse_button(&mouse->b.mmb, &mouse->b.quick_mmb);
	flag_update_mouse_button(&mouse->b.rmb, &mouse->b.quick_rmb);
	flag_update(mouse->window_focus_flag);
	flag_update(mouse->window_minimised_flag);
	flag_update(mouse->mouse_focus_flag);
	mouse->d = XY0;
}

void mouse_button_event(int *mb, int *quick_mb, int way)
{
	if (*mb * way == -2)		// if quick press situation
		*quick_mb = way;
	else
		*mb = 2 * way;
}

void mouse_button_update(int *mb, int *quick_mb, int new_state, int button_index, mouse_t *mouse)	// button index is 0 for lmb, 1 for mmb, 2 for rmb
{
	if (abs(*mb)==2)	// we don't need to update the mouse button state from global polling if the event did it fine
		return;

	if (new_state == 0 && *mb >= 0)		// if the button is being released
		*mb = -2;

	if (new_state && *mb <= 0)		// if the button is pressed
		if (mouse->mouse_focus_flag >= 0 && mouse->window_focus_flag >= 0)	// make sure the click is in the window
			*mb = 2;
}

// see libraries/sdl.c for sdl_mouse_event_proc()

void mouse_post_event_proc(mouse_t *mouse, zoom_t *zc)
{
	zc->just_reset = 0;

	#ifdef RL_SDL
	sdl_update_mouse(fb.window, mouse);
	#endif

	zoom_keyboard_control(zc, &mouse->zoom_flag);

	if (mouse->b.mmb == -2 && mouse->zoom_allowed)
		zoom_key_released(zc, &mouse->zoom_flag, 1);
	
	if (mouse->b.mmb == 2 && mouse->zoom_allowed)
		zc->zoom_key_time = get_time_ms();

	mouse->u = to_world_coord_xy(*zc, mouse->a);
	xy_t du = mul_xy(neg_y(mouse->d), set_xy(zc->iscrscale));

	if (mouse->zoom_flag && mouse->zoom_scroll_freeze==0)	// if we're scrolling the zoom
	{
		zc->offset_u = add_xy(zc->offset_u, mul_xy(du, set_xy(2.)));	// the factor of 2 is arbitrary, doesn't have to be integer
		calc_screen_limits(zc);
	}

	// post-input logic
	mouse->warp_prev = mouse->warp;
	if (mouse->warp_if_move==0)
	{
		mouse->showcursor = 0;
		mouse->warp = 0;
	}

	if (mouse->b.mmb > 0 && mouse->zoom_allowed)		// when to reset the zoom (when MMB is held down)
		zoom_key_released(zc, &mouse->zoom_flag, 2);

	if (mouse->zoom_flag && mouse->window_focus_flag == -2)	// if we left the window while zoom is on
	{
		zc->zoom_key_time = get_time_ms();
		zoom_key_released(zc, &mouse->zoom_flag, 1);
	}

	if (mouse->zoom_flag)
		mouse->u = zc->offset_u;
	mouse->u_stored = mouse->u;			// store mouse->u before it gets changed by temporary zoom changes

	if (mouse->b.clicks==1 && (mouse->b.lmb==2 || mouse->b.rmb==2))		// store the origin of a click
	{
		mouse->b.orig = mouse->u;

		if (mouse->mouse_focus_flag != 1)	// if the mouse focus is regained at the same time as the click this means drag and drop
			mouse->b.orig = set_xy(FLT_MAX); // set the click location far out so it doesn't click on any control
	}

	if (mouse->window_focus_flag == 2)		// if focus was regained through a click
		mouse->b.orig = set_xy(FLT_MAX);	// set the click location far out so it doesn't click on any control

	if (mouse->b.lmb==2)				// on click unselect any text editor
		cur_textedit = NULL;
	
	reset_insert_rect_array();			// nothing to do with the mouse but this is about the right place to put it
}

void mousecursor_logic_and_draw()
{
	if (mouse.zoom_flag)
	{
		#ifdef RL_SDL
		SDL_SetRelativeMouseMode(1);
		#endif

		if (mouse.showcursor == 1)
			mouse.showcursor = 0;
	}
	else
	{
		#ifdef RL_SDL
		SDL_SetRelativeMouseMode(mouse.warp);

		if (mouse.warp_prev && mouse.warp==0)		// set the mouse position after exiting warp mode
			sdl_set_mouse_pos_world(mouse.b.orig);
		#endif
	}

	//if (mouse.window_focus_flag < 0)
	//	mouse.showcursor = 0;

	#ifdef RL_SDL
	SDL_ShowCursor(mouse.showcursor==1);
	#endif
	draw_mousecursor(mouse.u);
}
