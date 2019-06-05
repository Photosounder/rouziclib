void flwindow_set_defaults(flwindow_t *w)
{
	w->bg_col = make_colour_hsl(240., 1., 0.0006, HUEDEG, 0);
	w->bar_col = make_colour_hsl(240., 0.5, 0.006, HUEDEG, 0);
	w->bar_height = 0.5;
	w->close_hover_col = make_grey(0.06);
	w->close_down_col = make_grey(0.12);
	w->title_col = make_grey(0.5);
	w->close_x_col = make_grey(0.5);
}

void draw_dialog_window_fromlayout(flwindow_t *w, int *diag_on, gui_layout_t *layout, const int id)
{
	int ret;
	layout_elem_t *cur_elem=NULL;
	rect_t area, area_os, bar_area, bar_area_os, corner_area_os, title_area, diag_area, close_area, close_x_area, pin_area;
	ctrl_button_state_t close_butt_state={0};

	if (ctrl_fromlayout_resizing(layout, id, 1))
		return ;
	cur_elem = &layout->elem[id];

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

	// Prevent the bar from being out of the screen when pinned
	if (w->pinned)
	{
		bar_area_os = offset_scale_rect(bar_area, layout->offset, layout->sm);
		rect_t bao_moved = bar_area_os;

		if (keep_box_inside_area(&bao_moved, zc.corners_dl))
		{
			xy_t offset = sub_xy(get_rect_centre(bao_moved), get_rect_centre(bar_area_os));
			layout->offset = add_xy(layout->offset, offset);
		}
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
		}
	}
	//---- Background, title bar and resizing logic ----

	// Areas calculation
	bar_area_os = offset_scale_rect(bar_area, layout->offset, layout->sm);
	title_area = get_subdiv_area(bar_area_os, xy(10./12., 8./12.), xy(0.5, 0.5));				// title area
	diag_area = get_subdiv_area(area_os, xy(1., 1. - w->bar_height / get_rect_dim(area).y), xy(0.5, 0.));	// diag area
	close_area = get_subdiv_area(bar_area_os, xy(div_y_by_x_xy(get_rect_dim(bar_area)), 1.), xy(1., 0.5));	// close button area
	close_x_area = get_subdiv_area(close_area, set_xy(6./12.), set_xy(0.5));				// shrink
	pin_area = get_subdiv_area(bar_area_os, xy(div_y_by_x_xy(get_rect_dim(bar_area)), 1.), xy(0., 0.5));	// pin area

	// close button
	if (diag_on)
	{
		if (ctrl_button_invis(close_area, &close_butt_state))
			*diag_on = 0;
	}

	// Drawing
	if (w->shadow_strength)
		draw_rect_full(fb, sc_rect(rect_move(area_os, mul_xy(xy(2., -3.), set_xy(get_rect_dim(area_os).y / 108.)))), hypot(zc.scrscale*get_rect_dim(area_os).y / 24., drawing_thickness), make_grey(0.), blend_alphablend, w->shadow_strength);	// shadow FIXME doesn't work in CL mode
	draw_black_rect(fb, sc_rect(area_os), drawing_thickness);					// black out the background
	draw_rect_full(fb, sc_rect(diag_area), drawing_thickness, w->bg_col, blend_add, 1.);		// diag rectangle
	draw_rect_full(fb, sc_rect(bar_area_os), drawing_thickness, w->bar_col, blend_add, 1.);		// title bar rectangle

	if (close_butt_state.over)
		draw_rect_full(fb, sc_rect(close_area), drawing_thickness, close_butt_state.down ? w->close_down_col : w->close_hover_col, blend_add, 1.);	// close button hover rect

	// Pin control
	if (w->hide_pin==0)
	{
		ctrl_checkbox_pin(&w->pinned, pin_area, w->title_col);

		world_os_to_pinned_os(layout->offset, layout->sm, &w->pinned_offset, &w->pinned_sm);
	}

	blend_func_t orig_blend = cur_blend;
	cur_blend = blend_add;
	draw_label(cur_elem->label, title_area, w->title_col, ALIG_CENTRE);				// title bar text
	if (diag_on)
		draw_label("\342\250\211", close_x_area, w->close_x_col, ALIG_CENTRE);			// close button
	cur_blend = orig_blend;

	if (w->hide_corner==0)
	{
		corner_area_os = make_rect_off( rect_p10(area_os), set_xy(get_rect_dim(bar_area_os).y * 6./12.), xy(1., 0.) );

		int hover = check_point_within_box(mouse.u, corner_area_os) || w->corner_drag.down;

		corner_area_os = get_subdiv_area(corner_area_os, set_xy(8./12.), set_xy(0.5));
		draw_line_thin(fb, sc_xy(corner_area_os.p0), sc_xy(corner_area_os.p1), drawing_thickness, GUI_COL_DEF, cur_blend, hover ? 0.5 : 0.25);
		corner_area_os = get_subdiv_area(corner_area_os, set_xy(6./12.), xy(1., 0.));
		draw_line_thin(fb, sc_xy(corner_area_os.p0), sc_xy(corner_area_os.p1), drawing_thickness, GUI_COL_DEF, cur_blend, hover ? 0.5 : 0.25);
	}

	ctrl_fromlayout_resizing(layout, id, 2);
}
