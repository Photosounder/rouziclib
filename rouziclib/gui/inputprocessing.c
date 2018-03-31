int equal_ctrl_id(ctrl_id_t a, ctrl_id_t b)	// returns 1 if the controls are identical
{
	if (equal_rect(a.box, b.box) == 0)
		return 0;

	if (a.id != b.id)
		return 0;

	return 1;
}

int check_ctrl_id(rect_t box, mouse_t mouse)	// returns 0 if the current control should be ignored
{
	mouse.ctrl_id->current.box = box;

	if (equal_rect(mouse.ctrl_id->hover.box, box))			// if this control has the same box as the previously hovered control
		mouse.ctrl_id->current.id++;				// increment the ID to differentiate between stacked same-box controls

	if (mouse.b.lmb > 0)						// if LMB is being pressed
		mouse.ctrl_id->hover_new = mouse.ctrl_id->hover;	// the hovered ID stays the same as before no matter what
	else if (check_point_within_box(mouse.u, box))			// otherwise if the mouse hovers the control
		mouse.ctrl_id->hover_new = mouse.ctrl_id->current;	// the hovered ID becomes the ID of the current control

	if (equal_ctrl_id(mouse.ctrl_id->current, mouse.ctrl_id->hover))
		return 1;
	return 0;
}

void proc_mouse_rect_ctrl_button(int mb, int clicks, ctrl_button_state_t *state, int orig_point_within_box)
{
	state->over = 1;

	if (mb != -1)					// if there is clicking going on
		if (orig_point_within_box)		// check the click originated in the same box
		{
			if (mb > 0)
				state->down = 1;

			if (mb == 2)
				state->once = 1;

			if (mb == -2)
				state->uponce = 1;

			if (mb == -2 && clicks == 2)
				state->doubleclick = 1;
		}
		else
			state->over = 0;		// if the click originated somewhere else we don't even care if the mouse is now over the box
}

ctrl_button_state_t *proc_mouse_rect_ctrl_lrmb(rect_t box, mouse_t mouse)
{
	static ctrl_button_state_t state[2];
	int orig_point_within_box;

	memset(state, 0, sizeof(state));

	if (check_ctrl_id(box, mouse)==0)
		return state;

	if (check_point_within_box(mouse.u, box))				// check if mouse is over box
	{
		orig_point_within_box = check_point_within_box(mouse.b.orig, box);
		proc_mouse_rect_ctrl_button(mouse.b.lmb, mouse.b.clicks, &state[0], orig_point_within_box);
		proc_mouse_rect_ctrl_button(mouse.b.rmb, mouse.b.clicks, &state[1], orig_point_within_box);
	}

	return state;
}

ctrl_button_state_t proc_mouse_rect_ctrl(rect_t box, mouse_t mouse)
{
	return proc_mouse_rect_ctrl_lrmb(box, mouse)[0];			// return the state for the lmb
}

ctrl_knob_state_t proc_mouse_knob_ctrl(rect_t box, mouse_t mouse)
{
	ctrl_knob_state_t state;

	memset(&state, 0, sizeof(state));

	if (check_ctrl_id(box, mouse)==0)
		return state;
	
	if (check_point_within_box(mouse.b.orig, box))	// check the click originated in the same box
	{
		if (mouse.b.clicks == 2)
		{
			if (mouse.b.lmb == 1)
				state.doubleclick = 1;
		}
		else
		{
			if (mouse.b.lmb == 1)
			{
				state.vert_delta = (mouse.u.y - mouse.prev_u.y) / get_rect_dim(box).y;

				if (mouse.mod_key[mouse_mod_shift])
				{
					state.vert_delta *= 1./32.;

					if (mouse.mod_key[mouse_mod_ctrl])
						state.vert_delta *= 1./32.;
				}
			}

			if (mouse.b.lmb > 0)
				state.down = 1;

			if (mouse.b.lmb == -2)
				state.uponce = 1;
		}
	}

	return state;
}

int proc_mouse_draggable_ctrl(ctrl_drag_state_t *state, rect_t box, mouse_t mouse)
{
	int ret=0, ctrl_id_check;

	state->offset = XY0;

	ctrl_id_check = check_ctrl_id(box, mouse);
	
	if (mouse.b.lmb < 0)				// if LMB is released
		state->down = 0;			// the control is deselected

	if (mouse.b.lmb==2 && ctrl_id_check)		// if LMB was just clicked and the control was hovered
		state->down = 1;			// the control is now selected

	if (mouse.b.lmb==1 && state->down)		// if the control is being dragged
	{
		state->offset = mul_xy(state->freedom, sub_xy(mouse.u, mouse.prev_u) );
		state->pos = add_xy(state->pos, state->offset);
		ret = 1;
	}

	return ret;
}

int proc_mouse_xy_ctrl(rect_t box, mouse_t mouse, xy_t *pos, int *lmb, int *rmb)
{
	int change = 0;

	*pos = XY0;
	*lmb = 0;
	*rmb = 0;

	if (check_ctrl_id(box, mouse)==0)
		return 0;

	*pos = mouse.u;
	*lmb = mouse.b.lmb;
	*rmb = mouse.b.rmb;

	if (*lmb != -1 || *rmb != -1)
		change = 1;

	return change;
}
