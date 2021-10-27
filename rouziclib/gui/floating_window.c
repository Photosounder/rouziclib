void flwindow_init_defaults(flwindow_t *w)
{
	if (w->init == 0)
	{
		w->bg_col = make_grey(0.);
		w->bar_col = make_grey(0.0054);
		w->border_col = make_grey(0.006);
		w->bar_height = 0.5;
		w->close_hover_col = make_grey(0.06);
		w->close_down_col = make_grey(0.12);
		w->title_col = make_grey(0.5);
		w->close_x_col = make_grey(0.5);
		w->parent_fit_offset = xy(0., 1.);	// the offset for fitting the window into the parent_area
		w->bg_opacity = 1.;
		w->shadow_strength = 0.85;
		w->draw_bg_always = 0;

		// The pinning preset offset and scale
		w->pinned_offset_preset = xy(-1e9, 1e9);
		w->pinned_sm_preset = 1.;
	}
}

void flwindow_init_pinned(flwindow_t *w)
{
	if (w->init == 0)
		w->pinned = 1;
}

void draw_dialog_window_fromlayout(flwindow_t *w, int *diag_on, rect_t *parent_area, gui_layout_t *layout, const int id)
{
	int ret, close_on=0;
	layout_elem_t *cur_elem=NULL;
	rect_t area, area_os, bar_area, bar_area_os, corner_area_os, title_area, diag_area, close_area, close_x_area, pin_area;
	ctrl_button_state_t close_butt_state={0};

	if (ctrl_fromlayout_resizing(layout, id, 1))
		return ;
	cur_elem = &layout->elem[id];

	// Fit an undetached window into its parent area if it exists
	if (parent_area)
		if (*diag_on==0 || w->init==0)
			fit_sublayout_into_area(*parent_area, layout, id, w->parent_fit_offset, 1);

	w->init = 1;

	//**** Background, title bar and resizing logic ****

	if (w->pinned && w->pinned_sm !=0.)
		pinned_os_to_world_os(w->pinned_offset, w->pinned_sm, &layout->offset, &layout->sm);

	area = gui_layout_elem_comp_area(layout, id);							// full window area
	bar_area = get_subdiv_area(area, xy(1., w->bar_height / get_rect_dim(area).y), xy(0.5, 1.));	// title bar area
	bar_area_os = offset_scale_rect(bar_area, layout->offset, layout->sm);

	ctrl_button_invis(offset_scale_rect(area, layout->offset, layout->sm), NULL);			// background clicks absorber

	// Title bar draggable control
	w->bar_drag.pos = get_rect_centre(bar_area_os);
	w->bar_drag.freedom = XY1;
	ctrl_drag_set_dim(&w->bar_drag, get_rect_dim(bar_area_os));
	ctrl_draggable(&w->bar_drag);
	layout->offset = add_xy(layout->offset, w->bar_drag.offset);
	if (w->bar_drag.down && parent_area)			// detach the title bar is being dragged
		*diag_on = 1;

	// Prevent the bar from being out of the screen when pinned
	if (w->pinned)
	{
		bar_area_os = offset_scale_rect(bar_area, layout->offset, layout->sm);
		rect_t bao_moved = bar_area_os;

		if (keep_box_inside_area(&bao_moved, zc.corners))
		{
			xy_t offset = sub_xy(get_rect_centre(bao_moved), get_rect_centre(bar_area_os));
			layout->offset = add_xy(layout->offset, offset);
		}

		if (parent_area)
			*diag_on = 1;		// this means the dialog is detached
	}

	area_os = offset_scale_rect(area, layout->offset, layout->sm);

	// Corner resizing draggable control
	if (w->hide_corner==0)
	{
		corner_area_os = make_rect_off( rect_p10(area_os), set_xy(get_rect_dim(bar_area_os).y * 6./12.), xy(1., 0.) );
		if (w->corner_drag.down==0)
			w->corner_drag.pos = get_rect_centre(corner_area_os);
		w->corner_drag.freedom = XY1;
		ctrl_drag_set_dim(&w->corner_drag, get_rect_dim(corner_area_os));
		if (ctrl_draggable(&w->corner_drag))
		{
			xy_t corner_pos, min_corner_pos, new_dim;
			double min_height;
			min_height = zc.iscrscale * 12. * get_rect_dim(area).y / w->bar_height;	// min bar height of 12 px
			min_corner_pos = add_xy(rect_p01(area_os), xy(min_height * div_x_by_y_xy(get_rect_dim(area)), -min_height));

			corner_pos = rect_p10(make_rect_off(w->corner_drag.pos, get_rect_dim(corner_area_os), xy(0.5, 0.5)));
			area_os.p1.x = MAXN(corner_pos.x, min_corner_pos.x);
			area_os.p0.y = MINN(corner_pos.y, min_corner_pos.y);

			// Make area_os have the same area but with the original aspect ratio
			new_dim = make_dim_from_area_and_aspect_ratio(mul_x_by_y_xy(get_rect_dim(area_os)), get_rect_dim(area));
			area_os = make_rect_off( rect_p01(area_os), new_dim, xy(0., 1.) );
			//area_os = fit_rect_in_area(get_rect_dim(area), area_os, xy(0., 1.));
			layout->offset = fit_into_area(area_os, area, 0., &layout->sm);

			// Detach if detachable
			if (parent_area)
				*diag_on = 1;
		}
	}
	//---- Background, title bar and resizing logic ----

	// Areas calculation
	bar_area_os = offset_scale_rect(bar_area, layout->offset, layout->sm);
	diag_area = get_subdiv_area(area_os, xy(1., 1. - w->bar_height / get_rect_dim(area).y), xy(0.5, 0.));	// diag area
	close_area = get_subdiv_area(bar_area_os, xy(div_y_by_x_xy(get_rect_dim(bar_area)), 1.), xy(1., 0.5));	// close button area
	close_x_area = get_subdiv_area(close_area, set_xy(6./12.), set_xy(0.5));				// shrink
	pin_area = get_subdiv_area(bar_area_os, xy(div_y_by_x_xy(get_rect_dim(bar_area)), 1.), xy(0., 0.5));	// pin area
	title_area = rect_add_margin(bar_area_os, xy(-1.5*get_rect_dim(bar_area_os).y, 0.));
	title_area = get_subdiv_area(title_area, xy(12./12., 8./12.), xy(0.5, 0.5));				// title area

	// Detachment logic
	if (parent_area==NULL)
	{
		if (diag_on)
			close_on = 1;
	}
	else
	{
		if (diag_on)
			if (*diag_on)
				close_on = 1;
	}

	// close button
	if (close_on)
	{
		if (ctrl_button_invis(close_area, &close_butt_state))
		{
			*diag_on = 0;
			w->pinned = 0;
		}
	}

	// Drawing
	int draw_bg = 1;
	if (diag_on)
		if (*diag_on == 0 && w->draw_bg_always == 0)
			draw_bg = 0;

	if (draw_bg)
	{
		if (w->shadow_strength)
			draw_black_rect(sc_rect(rect_move(area_os, mul_xy(xy(2., -3.), set_xy(get_rect_dim(area_os).y / 108.)))), hypot(zc.scrscale*get_rect_dim(area_os).y / 24., drawing_thickness), w->shadow_strength);	// shadow
		draw_black_rect(sc_rect(area_os), drawing_thickness, w->bg_opacity);		// black out the background
	}

	draw_rect_full(sc_rect(diag_area), drawing_thickness, w->bg_col, blend_add, 1.);	// diag rectangle
	draw_rect_full(sc_rect(bar_area_os), drawing_thickness, w->bar_col, blend_add, 1.);	// title bar rectangle
	draw_rect(sc_rect(area_os), drawing_thickness, w->border_col, blend_add, 1.);		// thin window border

	if (close_butt_state.over)
		draw_rect_full(sc_rect(close_area), drawing_thickness, close_butt_state.down ? w->close_down_col : w->close_hover_col, blend_add, 1.);	// close button hover rect

	// Pin control
	if (w->hide_pin==0)
	{
		world_os_to_pinned_os(layout->offset, layout->sm, &w->pinned_offset, &w->pinned_sm);

		if (ctrl_checkbox_pin(&w->pinned, pin_area, w->title_col).doubleclick)
		{
			w->pinned = 1;
			w->pinned_offset = w->pinned_offset_preset;
			w->pinned_sm = w->pinned_sm_preset;
		}
	}

	draw_label(cur_elem->label, title_area, w->title_col, ALIG_CENTRE);				// title bar text
	if (close_on)
		draw_label("\342\250\211", close_x_area, w->close_x_col, ALIG_CENTRE);			// close button

	// Resizing corner
	if (w->hide_corner==0)
	{
		corner_area_os = make_rect_off( rect_p10(area_os), set_xy(get_rect_dim(bar_area_os).y * 6./12.), xy(1., 0.) );

		int hover = check_point_within_box(mouse.u, corner_area_os) || w->corner_drag.down;

		corner_area_os = get_subdiv_area(corner_area_os, set_xy(8./12.), set_xy(0.5));
		draw_line_thin(sc_xy(corner_area_os.p0), sc_xy(corner_area_os.p1), drawing_thickness, GUI_COL_DEF, blend_add, hover ? 0.5 : 0.25);
		corner_area_os = get_subdiv_area(corner_area_os, set_xy(6./12.), xy(1., 0.));
		draw_line_thin(sc_xy(corner_area_os.p0), sc_xy(corner_area_os.p1), drawing_thickness, GUI_COL_DEF, blend_add, hover ? 0.5 : 0.25);
	}

	ctrl_fromlayout_resizing(layout, id, 2);
}

// Window manager

void window_run(window_manager_entry_t *w)
{
	rect_t a = w->parent_area;
	int *on = w->wind_on;
	void *f = w->window_func;
	void **p = w->ptr_array;

	if (w->already_ran)
		return ;

	switch (w->ptr_count)
	{
			case 0:	((void (*)(rect_t,int*))f)(a, on);
		break;	case 1:	((void (*)(rect_t,int*,void*))f)(a, on, p[0]);
		break;	case 2:	((void (*)(rect_t,int*,void*,void*))f)(a, on, p[0], p[1]);
		break;	case 3:	((void (*)(rect_t,int*,void*,void*,void*))f)(a, on, p[0], p[1], p[2]);
		break;	case 4:	((void (*)(rect_t,int*,void*,void*,void*,void*))f)(a, on, p[0], p[1], p[2], p[3]);
		break;	case 5:	((void (*)(rect_t,int*,void*,void*,void*,void*,void*))f)(a, on, p[0], p[1], p[2], p[3], p[4]);
		break;	case 6:	((void (*)(rect_t,int*,void*,void*,void*,void*,void*,void*))f)(a, on, p[0], p[1], p[2], p[3], p[4], p[5]);
		break;	default: fprintf_rl(stderr, "In window_run(): Unsupported number of pointers (%d)\n", w->ptr_count);
	}

	w->already_ran = 1;
}

int window_register(int priority, void *window_func, rect_t parent_area, int *wind_on, int num_args, ...)
{
	int i, ia;
	va_list ap;
	window_manager_entry_t *entry=NULL, entry_s={0};

	// Don't bother actually registering if this runs from the manager, just run the window function
	if (wind_man.manager_is_calling)
	{
		entry = &entry_s;
		entry->window_func = window_func;
		entry->ptr_count = num_args;
		entry->ptr_array = calloc(entry->ptr_count, sizeof(void *));
		i = -1;

		goto skip_add2;		// skip to filling the entry and run the function
	}

	// Check if the window function is already registered
	for (i=0; i < wind_man.window_count; i++)
		if (wind_man.window[i].window_func == window_func)
		{
			entry = &wind_man.window[i];
			goto skip_add2;
		}

	// Look for a free slot in the registry
	for (i=0; i < wind_man.window_count; i++)
		if (wind_man.window[i].window_func == NULL)
			goto skip_add1;

	// Add new window at the end of the registry
	i = wind_man.window_count;
	alloc_enough(&wind_man.window, wind_man.window_count+=1, &wind_man.window_as, sizeof(window_manager_entry_t), 2.);

skip_add1:
	entry = &wind_man.window[i];
	entry->window_func = window_func;
	entry->ptr_count = num_args;
	entry->ptr_array = calloc(entry->ptr_count, sizeof(void *));

	// Set window orders
	wind_man.min_order = entry->order = wind_man.min_order - 1;

	if (priority==1)
		window_move_to_top(window_func);

skip_add2:
	// Copy pointers to array
	va_start(ap, num_args);
	for (ia=0; ia < num_args; ia++)
		entry->ptr_array[ia] = va_arg(ap, void *);
	va_end(ap);

	entry->parent_area = parent_area;
	entry->wind_on = wind_on;
	entry->dereg = 0;

	// Run here if window has a close button and is "closed"
	if (wind_on)
		if (*wind_on == 0)
			window_run(entry);

	if (wind_man.manager_is_calling)
		free(entry->ptr_array);

	return i;
}

int cmp_window_man_order(const window_manager_entry_t **a, const window_manager_entry_t **b)
{
	return (*a)->order - (*b)->order;
}

void window_man_sort(int reg_count)
{
	int i, j;

	// Sort
	alloc_enough((void **) &wind_man.wsor, reg_count, &wind_man.wsor_as, sizeof(window_manager_entry_t *), 2.);
	for (j=0, i=0; i < wind_man.window_count; i++)
		if (wind_man.window[i].window_func)
		{
			wind_man.wsor[j] = &wind_man.window[i];
			j++;
		}

	qsort(wind_man.wsor, reg_count, sizeof(window_manager_entry_t *), cmp_window_man_order);

	// Set new order
	for (i=0; i < reg_count; i++)
		wind_man.wsor[i]->order = i;
	wind_man.min_order = 0;
	wind_man.max_order = reg_count-1;
}

void window_manager()
{
	int i, j, prev_hover_ided = mouse.ctrl_id->hover_ided, reg_count=0;

	// Clear deregistered windows
	for (i=0; i < wind_man.window_count; i++)
		if (wind_man.window[i].dereg)
		{
			free(wind_man.window[i].ptr_array);
			memset(&wind_man.window[i], 0, sizeof(window_manager_entry_t));
		}
		else if (wind_man.window[i].window_func)
			reg_count++;

	// Sort registered windows
	window_man_sort(reg_count);

	// Run all windows in sorted order
	wind_man.manager_is_calling = 1;
	for (i=0; i < reg_count; i++)
		if (wind_man.wsor[i]->window_func)
		{
			window_run(wind_man.wsor[i]);

			// Put this window on top if one of its controls has been clicked
			if (prev_hover_ided != mouse.ctrl_id->hover_ided && (mouse.b.lmb>=1 || mouse.b.rmb>=1))
				window_move_to_top(wind_man.wsor[i]->window_func);

			prev_hover_ided = mouse.ctrl_id->hover_ided;
		}
	wind_man.manager_is_calling = 0;

	// Mark all windows for deregistration
	for (i=0; i < wind_man.window_count; i++)
	{
		wind_man.window[i].dereg = 1;
		wind_man.window[i].already_ran = 0;
	}
}

int window_find_id_by_func(void *window_func)
{
	for (int i=0; i < wind_man.window_count; i++)
		if (wind_man.window[i].window_func == window_func)
			return i;

	return -1;
}

void window_set_parent(void *window_func, void *parent_window_func)
{
	int window_id;

	window_id = window_find_id_by_func(window_func);
	if (window_id == -1)
		return ;

	wind_man.window[window_id].parent_window_func = parent_window_func;
}

void window_move_up(int id, int offset)
{
	int i;

	// Move window up
	wind_man.window[id].order += offset;

	// Move children up
	for (i=0; i < wind_man.window_count; i++)
		if (wind_man.window[i].parent_window_func == wind_man.window[id].window_func)
			window_move_up(i, offset);
}

void window_move_to_top(void *window_func)
{
	int i, window_id, offset;

	window_id = window_find_id_by_func(window_func);
	if (window_id == -1)
		return ;

	// The window must move up by enough ranks that it's above the others
	offset = wind_man.max_order+1 - wind_man.window[window_id].order;

	// Recursively move the window and its descendants by the offset
	window_move_up(window_id, offset);

	wind_man.max_order += offset;
}
