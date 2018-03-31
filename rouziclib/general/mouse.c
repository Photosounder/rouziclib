mouse_t init_mouse()
{
	mouse_t mouse;

	memset(&mouse, 0, sizeof(mouse_t));
	mouse.ctrl_id = calloc(1, sizeof(mouse_ctrl_id_t));

	mouse.b.lmb = mouse.b.mmb = mouse.b.rmb = -1;

	return mouse;
}

void mouse_pre_event_proc(mouse_t *mouse)
{
	mouse->d = XY0;
	mouse->prev_u = mouse->u_stored;
	mouse->b.wheel = 0;
	mouse->ctrl_id->hover = mouse->ctrl_id->hover_new;
	memset(&mouse->ctrl_id->hover_new, 0, sizeof(ctrl_id_t));
	memset(&mouse->ctrl_id->current, 0, sizeof(ctrl_id_t));

	flag_update(mouse->b.lmb);
	flag_update(mouse->b.mmb);
	flag_update(mouse->b.rmb);
	flag_update(mouse->window_focus_flag);
	flag_update(mouse->window_minimised_flag);
	flag_update(mouse->mouse_focus_flag);
}

// see libraries/sdl.c for mouse_event_proc()

void mouse_post_event_proc(mouse_t *mouse, zoom_t *zc)
{
	if (mouse->b.mmb > 0)					// when to reset the zoom (when MMB is held down)
		zoom_key_released(zc, &mouse->zoom_flag, 2);

	if (mouse->zoom_flag && mouse->window_focus_flag == -2)	// if we left the window while zoom is on
	{
		#ifdef RL_SDL
		zc->zoom_key_time = SDL_GetTicks();
		#endif
		zoom_key_released(zc, &mouse->zoom_flag, 1);
	}

	if (mouse->zoom_flag)
		mouse->u = zc->offset_u;
	mouse->u_stored = mouse->u;			// store mouse->u before it gets changed by temporary zoom changes

	if (mouse->b.lmb==2 || mouse->b.rmb==2)		// store the origin of a click
		mouse->b.orig = mouse->u;

	if (mouse->window_focus_flag < 0)		// if focus was regained through a click
		mouse->b.orig = xy(FLT_MAX, FLT_MAX);	// set the click location far out so it doesn't click on any control

	if (mouse->b.lmb==2)				// on click unselected any text editor
		cur_textedit = NULL;
}
