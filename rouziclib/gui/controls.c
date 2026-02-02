int ctrl_button_invis_box_or_polygon(rect_t box, xy_t *p, int p_count, ctrl_button_state_t *butt_state_ptr)
{
	double total_scale = rect_min_side(box)*zc.scrscale;
	ctrl_button_state_t butt_state={0};

	if (isnan_rect(box))
		return 0;

	if (butt_state_ptr)
		*butt_state_ptr = butt_state;

	// Return if the button is too small
	if (total_scale < 3./fb->pixel_scale)
	{
		if (butt_state_ptr)
			butt_state_ptr->too_small = 1;
		return 0;
	}

	// If the button is off-screen
	if (check_box_on_screen(box)==0)
	{
		if (butt_state_ptr)
			butt_state_ptr->out_of_screen = 1;

		// Return if there's no click
		if (mouse.b.lmb < 1)
			return 0;

		// Return if there's no held click originating from it
		if (check_point_within_box(mouse.b.orig, box)==0)
			return 0;
	}

	// Identify and process the control if the window is in focus
	if (mouse.window_focus_flag >= 0)
		if (p)
			butt_state = proc_mouse_polygon_ctrl_lrmb(box, p, p_count)[0];
		else
			butt_state = proc_mouse_rect_ctrl(box);

	if (butt_state_ptr)
		*butt_state_ptr = butt_state;

	return butt_state.uponce;
}

int ctrl_button_invis(rect_t box, ctrl_button_state_t *butt_state_ptr)
{
	return ctrl_button_invis_box_or_polygon(box, NULL, 0, butt_state_ptr);
}

ctrl_button_state_t ctrl_button_polygon_invis(xy_t *p, int p_count)
{
	ctrl_button_state_t butt_state={0};
	ctrl_button_invis_box_or_polygon(get_bounding_box_for_polygon(p, p_count), p, p_count, &butt_state);
	return butt_state;
}

ctrl_button_state_t ctrl_button_chamf_state(const uint8_t *name, rect_t box, col_t colour)
{
	double intensity = 1.;
	double scale = rect_min_side(box);
	ctrl_button_state_t butt_state={0};
	rect_t boxb;
	xy_t dim;

	ctrl_button_invis(box, &butt_state);
	if (butt_state.too_small || butt_state.out_of_screen)
		return butt_state;

	intensity *= rect_ctrl_intensity_scale(box);

	draw_rect_chamfer(sc_rect(box), drawing_thickness, colour, cur_blend, 0.5*intensity, 1./12.);

	if (butt_state.over && butt_state.down==0)
	{
		boxb = get_rect_centred_coord(box);
		boxb.p1 = sub_xy(boxb.p1, set_xy(scale / 12.));
		boxb = make_rect_centred(boxb.p0, boxb.p1);
		draw_rect_chamfer(sc_rect(boxb), drawing_thickness, colour, cur_blend, 0.5*intensity, 1./12.*9./12.);
	}

	dim = get_rect_dim(box);
	box = rect_add_margin(box, xy(MINN(-dim.x/10., -dim.y/6.), -dim.y/6.));
	draw_string_bestfit(font, name, sc_rect(box), 0., 1e30*zc.scrscale, colour, 1., drawing_thickness, ALIG_CENTRE, NULL);

	return butt_state;
}

int ctrl_button_chamf(const uint8_t *name, rect_t box, col_t colour)
{
	return ctrl_button_chamf_state(name, box, colour).uponce;
}

int ctrl_checkbox(int *state, const uint8_t *name, rect_t box, col_t colour)
{
	double intensity = 1.;
	double scale = rect_min_side(box);
	ctrl_button_state_t butt_state={0};

	ctrl_button_invis(box, &butt_state);
	if (butt_state.too_small || butt_state.out_of_screen)
		return 0;

	intensity *= rect_ctrl_intensity_scale(box);

	if (butt_state.over && butt_state.down==0)
		draw_rect(sc_rect(box), drawing_thickness, colour, cur_blend, 0.25*intensity);

	if (butt_state.uponce && state)
		*state = (*state & 1) ^ 1;

	box = rect_add_margin(box, xy(-2.*scale/font->line_vspacing, 0.));
	if (state)
		draw_string_bestfit(font, (*state==1) ? "\xE2\x98\x91" : (*state==0) ? "\xE2\x98\x90" : "\xF3\xB2\x98\x92", sc_rect(box), 0., 1e30*zc.scrscale, colour, 1., drawing_thickness, ALIG_LEFT, NULL);
	else
		draw_string_bestfit(font, "\xE2\x98\x90", sc_rect(box), 0., 1e30*zc.scrscale, colour, 1., drawing_thickness, ALIG_LEFT, NULL);

	box.p0.x += (8.+font->letter_spacing)*scale/font->line_vspacing;
	draw_string_bestfit(font, name, sc_rect(box), 0., 1e30*zc.scrscale, colour, 1., drawing_thickness, ALIG_LEFT, NULL);

	return butt_state.uponce;
}

ctrl_button_state_t ctrl_checkbox_pin(int *state, rect_t box, col_t colour)
{
	double intensity = 1.;
	double scale = rect_min_side(box);
	ctrl_button_state_t butt_state={0};

	ctrl_button_invis(box, &butt_state);
	if (butt_state.too_small || butt_state.out_of_screen)
		return butt_state;

	intensity *= intensity_scaling(scale*zc.scrscale, 24.);

	if (butt_state.over && butt_state.down==0)
		draw_rect(sc_rect(box), drawing_thickness, colour, cur_blend, 0.25*intensity);

	if (butt_state.uponce && state)
		*state = (*state & 1) ^ 1;

	box = rect_add_margin(box, xy(-2.*scale/font->line_vspacing, 0.));
	draw_string_bestfit(font, (*state) ? "\xE2\x98\x91\302\240\360\237\223\214" : "\xE2\x98\x90\302\240\360\237\223\214", sc_rect(box), 0., 1e30*zc.scrscale, colour, 1., drawing_thickness, ALIG_LEFT, NULL);

	return butt_state;
}

int ctrl_radio(int state, const uint8_t *name, rect_t box, col_t colour)
{
	double intensity = 1.;
	double scale = rect_min_side(box);
	ctrl_button_state_t butt_state={0};

	ctrl_button_invis(box, &butt_state);
	if (butt_state.too_small || butt_state.out_of_screen)
		return 0;

	intensity *= rect_ctrl_intensity_scale(box);

	if (butt_state.over && butt_state.down==0)
		draw_rect(sc_rect(box), drawing_thickness, colour, cur_blend, 0.25*intensity);

	draw_circle(HOLLOWCIRCLE, sc_xy(add_xy(box.p0, xy(0.5*scale, 0.5*scale))), 0.3*scale*zc.scrscale, drawing_thickness, colour, cur_blend, intensity);
	if (state)
		draw_circle(FULLCIRCLE, sc_xy(add_xy(box.p0, xy(0.5*scale, 0.5*scale))), 7./12.*0.3*scale*zc.scrscale, drawing_thickness, colour, cur_blend, 1.);

	if (name)
	{
		box = rect_add_margin(box, xy(-2.*scale/font->line_vspacing, 0.));
		box.p0.x += (8.+font->letter_spacing)*scale/font->line_vspacing;
		draw_string_bestfit(font, name, sc_rect(box), 0., 1e30*zc.scrscale, colour, 1., drawing_thickness, ALIG_LEFT, NULL);
	}

	return butt_state.uponce;
}

rect_t selectmenu_rect(rect_t box, const int id)
{
	xy_t pad0 = xy(0., rect_min_side(box) * -6/144.);
	xy_t start = add_xy(box.p0, pad0);

	if (id < 0)
		return make_rect_off( start, mul_xy(get_rect_dim(box), xy(1., -id)), xy(0., 1.) );
	else
		return make_rect_off( mad_xy(get_rect_dim(box), xy(0., -id), start) , get_rect_dim(box), xy(0., 1.) );
}

int ctrl_selectmenu(ctrl_selectmenu_state_t *state, rect_t box, col_t colour)
{
	double intensity = 1.;
	ctrl_button_state_t butt_state={0}, subbutt_state={0};
	rect_t open_box, entry_box;
	int i, ret = 0, over_something=0;

	state->open = state->next_open;		// set the menu's open status based on the previous input

	// Menu top control (opens/closes the menu)
	ctrl_button_invis(box, &butt_state);
	if (butt_state.too_small || (state->open==0 && butt_state.out_of_screen))
		return 0;

	if (butt_state.uponce)
		state->next_open ^= 1;		// open/close menu when top control is clicked

	if (butt_state.over)
		over_something = 1;

	// Draw top control box
	intensity *= rect_ctrl_intensity_scale(box);
	draw_rect(sc_rect(box), drawing_thickness, colour, cur_blend, intensity);
	draw_label("\363\260\204\200", make_rect_off(box.p1, set_xy(rect_min_side(box)), xy(1., 1.)), colour, ALIG_CENTRE);	// check mark

	// When the menu is open
	state->hover_id = -1;
	if (state->open)
	{
		// Draw containing frame
		open_box = selectmenu_rect(box, -state->count);
		draw_black_rect(sc_rect(open_box), drawing_thickness, 1.);
		draw_rect(sc_rect(open_box), drawing_thickness, colour, cur_blend, intensity);

		for (i=0; i < state->count; i++)
		{
			// Process input for each menu entry
			entry_box = selectmenu_rect(box, i);
			if (ctrl_button_invis(entry_box, &subbutt_state))
			{
				state->sel_id = i;
				state->next_open = 0;
				ret = 1;
			}

			if (subbutt_state.over)
			{
				over_something = 1;
				state->hover_id = i;

				// Highlight hovered entry with a blue background
				draw_rect_full(sc_rect(entry_box), drawing_thickness, make_colour_hsl(220., 0.5, 0.08, HUEDEG, 0), cur_blend, 1.);
			}
		}
	}

	// Close menu if clicked outside of any of its controls
	if (over_something==0 && butt_state.down==0 && mouse.b.lmb==2)
		state->next_open = 0;

	return ret;
}

void draw_selectmenu_entry(ctrl_selectmenu_state_t *state, rect_t box, col_t colour, int id, const char *label)
{
	double scale = rect_min_side(box);
	rect_t entry_box, main_label_box;

	// Draw the selected entry's name in the top control
	if (id == state->sel_id)
	{
		main_label_box = box;
		main_label_box.p0.x += 3.*scale/font->line_vspacing;
		main_label_box.p1.x -= scale;
		main_label_box = rect_add_margin(main_label_box, xy(0., -2.*scale/font->line_vspacing));
		draw_label(label, main_label_box, colour, ALIG_LEFT);
	}

	entry_box = selectmenu_rect(box, id);

	// Draw the entry's name in the menu
	if (state->open)
	{
		// Draw checkmark if selected
		if (id == state->sel_id)
			draw_label("\342\234\224", rect_add_margin(make_rect_off(entry_box.p0, set_xy(scale), XY0), set_xy(-1.5/12.*scale)), colour, ALIG_CENTRE);

		entry_box.p0.x += scale;
		entry_box.p1.x -= 3.*scale/font->line_vspacing;
		entry_box = rect_add_margin(entry_box, xy(0., -2.*scale/font->line_vspacing));
		draw_label(label, entry_box, colour, ALIG_LEFT);
	}
}

knob_t make_knob(char *main_label, double default_value, const knob_func_t func, double min, double max, char *fmt_str)
{
	knob_t knob={0};

	knob.main_label = main_label;
	knob.default_value = default_value;
	knob.func = func;
	knob.min = min;
	knob.max = max;
	knob.fmt_str = fmt_str;
	textedit_init(&knob.edit, 0);

	return knob;
}

void knob_value_draw_on_top(const char *str, double *scale, rect_t *area, col_t *colour)
{
	draw_black_rect(sc_rect(*area), drawing_thickness, 10./12.);
	draw_string_bestfit(font, str, sc_rect(*area), 0., *scale, *colour, 1., drawing_thickness, ALIG_CENTRE | MONODIGITS, NULL);
}

void knob_te_draw_on_top(textedit_t *te, rect_t *area, col_t *colour)
{
	draw_black_rect(sc_rect(*area), drawing_thickness, 11./12.);
	ctrl_textedit_draw(te, *area, *colour);
}

void knob_draw_on_top(gui_layout_t *layout, knob_t *knob, rect_t *area, col_t *colour, double *t)
{
	double th;
	double scale = rect_min_side(*area);
	xy_t p0, p1, centre = get_rect_centre(*area);

	layout->offset = fit_into_area(*area, make_rect_off(XY0, xy(2., 2.), xy(0.5, 0.5)), 0., &layout->sm);

	draw_black_rect(sc_rect(*area), drawing_thickness, 1. - 1./16.);

	// Draw bottom label
	draw_string_bestfit(font, knob->main_label, sc_rect(gui_layout_elem_comp_area_os(layout, knob->circular ? 21 : 20, XY0)), 0., 0.03*scale*zc.scrscale, *colour, 1., drawing_thickness, ALIG_CENTRE, NULL);

	// Draw units label
	draw_string_bestfit(font, knob->unit_label, sc_rect(gui_layout_elem_comp_area_os(layout, knob->circular ? 31 : 30, XY0)), 0., 0.03*scale*zc.scrscale, *colour, 1., drawing_thickness, ALIG_CENTRE, NULL);

	// Draw arc circle
	draw_circle_arc(sc_xy(centre), set_xy(0.5*scale*zc.scrscale), knob->circular ? 0. : -0.375, knob->circular ? 1. : 0.375, drawing_thickness, *colour, cur_blend, 0.5);

	// Draw notch on the arc circle
	if (knob->circular)
		th = *t * -2.*pi;
	else
		th = (*t * 0.75 - 0.375) * -2.*pi;
	p0 = rotate_xy2(xy(0., 0.5*scale), th);
	p1 = rotate_xy2(xy(0., 0.4*scale), th);
	draw_line_thin(sc_xy(add_xy(centre, p0)), sc_xy(add_xy(centre, p1)), drawing_thickness, *colour, cur_blend, 2.);

	// Draw value
	rect_t label_area = gui_layout_elem_comp_area_os(layout, 11, XY0);
	draw_string_bestfit(font, knob->printed_label, sc_rect(label_area), 0., 0.03*scale*zc.scrscale, *colour, 1., drawing_thickness, ALIG_CENTRE | MONODIGITS, NULL);
}

int ctrl_knob(double *v_orig, knob_t *knob, rect_t box, col_t colour)
{
	int ret, val_set_by_edit=0;
	double intensity = 1.;
	double scale = rect_min_side(box);
	double v=NAN, t, t_off=0., th;
	static double t_rate=0., v_downonce=NAN;
	xy_t p0, p1, centre = get_rect_centre(box);
	static gui_layout_t layout={0};
	static const char *layout_src[] = {
		"elem 0", "type none", "pos	0	0", "dim	2", "off	0;6", "",
		"elem 10", "type none", "pos	0	0", "dim	1;6	0;8", "off	0;6", "",
		"elem 11", "type none", "pos	0	0", "dim	1;5	0;9", "off	0;6", "",
		"elem 20", "type none", "pos	0	-0;10", "dim	1;2	0;6", "off	0;6", "",
		"elem 21", "type none", "pos	0	-0;6;4", "dim	0;10	0;4", "off	0;6", "",
		"elem 30", "type none", "pos	0	-0;3;6", "dim	1;6	0;2;6", "off	0;6	1", "",
		"elem 31", "type none", "pos	0;10	-0;3;6", "dim	0;5	0;2;6", "off	1", "",
	};

	layout.offset = fit_into_area(box, make_rect_off(XY0, xy(2., 2.), xy(0.5, 0.5)), 0., &layout.sm);
	make_gui_layout(&layout, layout_src, sizeof(layout_src)/sizeof(char *), NULL);

	if (v_orig)
		v = *v_orig;

	// Initialise the value
	if (isnan(v))
	{
		v = knob->default_value;
		if (v_orig)
			*v_orig = v;
	}

	if (knob->circular)
		v = rangewrap(v, knob->min, knob->max);
	else
		if (knob->func == knobf_dboff)
			v = MINN(v, knob->max);
		else
			v = rangelimit(v, knob->min, knob->max);

	if (v_orig)
		*v_orig = v;

	if (scale*zc.scrscale < 3./fb->pixel_scale)
		return 0;

	if (check_box_on_screen(rect_size_mul(box, set_xy(1.2)))==0)	// box is extended so that it covers the whole label
		return 0;

	// Process input
	if (mouse.window_focus_flag >= 0)
	{
		knob->knob_state = proc_mouse_knob_ctrl(box);
		t_off = knob->knob_state.vert_delta * 1./1024.;		// was 2./1728. until 2024

		if (knob->knob_state.downonce)
		{
			t_rate = 0.;
			v_downonce = v;
		}

		if (get_kb_alt() > 0 && knob->knob_state.down)
		{
			t_rate += t_off * 1./1024.;			// was 1./144. until 2024
			t_off = t_rate;
		}

		if (knob->knob_state.rightclick && knob->edit_open==0)
		{
			knob->edit_open = 1;

			// Print editor value
			if (knob->editor_print_func)
				knob->editor_print_func(knob->printed_label, knob, v);
			else
				snprintf(knob->printed_label, sizeof(knob->printed_label), "%.10g", v);

			textedit_set_new_text(&knob->edit, knob->printed_label);
			knob->edit.rect_brightness = 0.125;
			cur_textedit = &knob->edit;
		}
	}
	else	// release the mouse if the window focus is lost
	{
		mouse.warp_if_move = 0;
		mouse.zoom_scroll_freeze = 0;
	}

	if (knob->knob_state.down)
	{
		mouse.warp_if_move = 1;
		mouse.zoom_scroll_freeze = 1;
	}

	if (knob->knob_state.uponce || knob->knob_state.doubleclick)
	{
		mouse.warp_if_move = 0;
		mouse.zoom_scroll_freeze = 0;
	}

	// Reset on doubleclick
	if (knob->knob_state.doubleclick)
		v = knob->default_value;

	// Calculate new position and value
	t = knob->func(v, knob->min, knob->max, knob->arg, 1);

	if (knob->circular)
		t = rangewrap(t+t_off, 0., 1.);
	else
		t = rangelimit(t+t_off, 0., 1.);

	if (t_off)
		v = knob->func(t, knob->min, knob->max, knob->arg, 0);

	// Draw knob
	intensity *= intensity_scaling(scale*zc.scrscale, 24.);

	// Print value
	if (knob->display_print_func)
		knob->display_print_func(knob->printed_label, knob, v);
	else
		snprintf(knob->printed_label, sizeof(knob->printed_label), knob->fmt_str ? knob->fmt_str : VALFMT_DEFAULT, v);

	// Draw value string
	if (knob->edit_open==0)
	{
		rect_t label_area = gui_layout_elem_comp_area_os(&layout, 11, XY0);
		const double height_limit = 0.75;
		double orig_height = get_rect_dim(label_area).y * zc.zoomscale;
		if (orig_height*1.5 < height_limit && knob->knob_state.down)
		{
			#if 0
			label_area = rect_size_mul(label_area, set_xy(height_limit / (get_rect_dim(label_area).y * zc.zoomscale)));
			keep_box_inside_area(&label_area, rect_add_margin(zc.corners, set_xy((-1./12.)/zc.zoomscale)));

			// Register "window" to display the string
			static double ontop_scale;	ontop_scale = 0.03*scale*zc.scrscale * height_limit/orig_height;
			static rect_t ontop_area;	ontop_area = label_area;
			static col_t ontop_colour;	ontop_colour = colour;
			window_late_register(knob_value_draw_on_top, NULL, 4, knob->printed_label, &ontop_scale, &ontop_area, &ontop_colour);
			#else
			static rect_t ontop_area;	ontop_area = rect_size_mul(box, set_xy(height_limit / (get_rect_dim(label_area).y * zc.zoomscale)));
			static col_t ontop_colour;	ontop_colour = colour;
			static double ontop_t;		ontop_t = t;
			window_late_register(knob_draw_on_top, NULL, 5, &layout, knob, &ontop_area, &ontop_colour, &ontop_t);
			#endif
		}
		else
			draw_string_bestfit(font, knob->printed_label, sc_rect(label_area), 0., 0.03*scale*zc.scrscale, colour, 1., drawing_thickness, ALIG_CENTRE | MONODIGITS, NULL);
	}
	else
	{
		// Calculate editor area (which is enlarged at low sizes) and process the text editor
		rect_t edit_area = gui_layout_elem_comp_area_os(&layout, 10, XY0);
		const double height_limit = 1.25;
		if (get_rect_dim(edit_area).y * zc.zoomscale * 1.5 < height_limit)
		{
			edit_area = rect_size_mul(edit_area, set_xy(height_limit / (get_rect_dim(edit_area).y * zc.zoomscale)));
			keep_box_inside_area(&edit_area, rect_add_margin(zc.corners, set_xy((-1./12.)/zc.zoomscale)));

			// Register "window" to display the editor
			static rect_t ontop_area;	ontop_area = edit_area;
			static col_t ontop_colour;	ontop_colour = colour;
			window_late_register(knob_te_draw_on_top, NULL, 3, &knob->edit, &ontop_area, &ontop_colour);

			ret = ctrl_textedit_invis(&knob->edit, edit_area);
		}
		else
			ret = ctrl_textedit(&knob->edit, edit_area, colour);

		if (ret==1 || ret==2 || ret==3)
		{
			// Parse editor string into value
			if (knob->parse_func)
				v = knob->parse_func(knob->edit.string, knob);
			else
				v = etof(knob->edit.string);

			if (isnan(v))
				v = knob->default_value;

			if (knob->circular)
				v = rangewrap(v, knob->min, knob->max);
			else
				if (knob->func == knobf_dboff)
					v = MINN(v, knob->max);
				else
					v = rangelimit(v, knob->min, knob->max);

			val_set_by_edit = 1;
			knob->edit_open = 0;
			cur_textedit = NULL;
		}
	}

	// Draw bottom label
	draw_string_bestfit(font, knob->main_label, sc_rect(gui_layout_elem_comp_area_os(&layout, knob->circular ? 21 : 20, XY0)), 0., 0.03*scale*zc.scrscale, colour, 1., drawing_thickness, ALIG_CENTRE, NULL);

	// Draw units label
	draw_string_bestfit(font, knob->unit_label, sc_rect(gui_layout_elem_comp_area_os(&layout, knob->circular ? 31 : 30, XY0)), 0., 0.03*scale*zc.scrscale, colour, 1., drawing_thickness, ALIG_CENTRE, NULL);

	// Draw arc circle
	draw_circle_arc(sc_xy(centre), set_xy(0.5*scale*zc.scrscale), knob->circular ? 0. : -0.375, knob->circular ? 1. : 0.375, drawing_thickness, colour, cur_blend, 0.5*intensity);

	// Draw notch on the arc circle
	if (knob->circular)
		th = t * -2.*pi;
	else
		th = (t * 0.75 - 0.375) * -2.*pi;
	p0 = rotate_xy2(xy(0., 0.5*scale), th);
	p1 = rotate_xy2(xy(0., 0.4*scale), th);
	draw_line_thin(sc_xy(add_xy(centre, p0)), sc_xy(add_xy(centre, p1)), drawing_thickness, colour, cur_blend, 2.*intensity);

	if (v_orig)
		*v_orig = v;

	if ((knob->knob_state.uponce && v_downonce != v) || knob->knob_state.doubleclick || val_set_by_edit)	// the final value change when the mouse button is released
		return 1;
	if (knob->knob_state.down && t_off != 0.)					// an ongoing value change when the mouse button is held down
		return 2;
	return 0;
}

ctrl_drag_state_t make_drag_state(xy_t pos, xy_t freedom)
{
	ctrl_drag_state_t state={0};

	state.pos = pos;
	state.freedom = freedom;
	state.click_offset = set_xy(0.5);

	return state;
}

void ctrl_drag_set_dim(ctrl_drag_state_t *state, xy_t dim1)
{
	if (state->down==0)
		state->click_offset = set_xy(0.5);

	rect_t r = resize_rect_around_offset(make_rect_centred(state->pos, state->dim), dim1, state->click_offset);

	state->pos = get_rect_centre(r);
	state->dim = dim1;
}

int ctrl_draggable(ctrl_drag_state_t *state)
{
	int ret=0;
	double total_scale = min_of_xy(state->dim)*zc.scrscale;
	rect_t box;

	if (total_scale < 3./fb->pixel_scale && state->down==0)
		return 0;

	box = make_rect_off( state->pos, state->dim, xy(0.5, 0.5) );

	if (check_box_on_screen(box)==0 && state->down==0)
		return 0;

	if (mouse.window_focus_flag >= 0)
		ret = proc_mouse_draggable_ctrl(state, box);

	return ret;
}

int ctrl_draggable_circle_invis(xy_t *pos, double radius, int *sel_id, int cur_id, int *dragged, ctrl_button_state_t *butt_state_ptr, rect_t pos_area)
{
	int ret=0;
	double total_scale = radius*zc.scrscale;
	rect_t box;
	ctrl_button_state_t butt_state={0};

	// Quit if control is too small and not being dragged
	if (total_scale < 0.2/fb->pixel_scale && *dragged != cur_id)
		return -10;

	// Quit if control is off-screen and not being dragged
	box = make_rect_off( *pos, set_xy(2.*radius), xy(0.5, 0.5) );
	if (check_box_on_screen(box)==0 && *dragged != cur_id)
		return -10;

	if (mouse.window_focus_flag >= 0)
	{
		butt_state = proc_mouse_circular_ctrl(pos, radius, *dragged == cur_id);

		// Limit pos to the pos_area
		if (isfinite(pos_area.p0.x)) pos->x = MAXN(pos->x, pos_area.p0.x);
		if (isfinite(pos_area.p1.x)) pos->x = MINN(pos->x, pos_area.p1.x);
		if (isfinite(pos_area.p0.y)) pos->y = MAXN(pos->y, pos_area.p0.y);
		if (isfinite(pos_area.p1.y)) pos->y = MINN(pos->y, pos_area.p1.y);

		if (butt_state.once)
		{
			ret = 1;
			*dragged = cur_id;
			*sel_id = cur_id;
		}

		if (butt_state.down)
			ret = 3;

		if (butt_state.uponce)
		{
			ret = 2;
			*dragged = -1;
		}

		if (butt_state.doubleclick)
		{
			ret = -1;
			*dragged = -1;
			*sel_id = -1;
		}
	}

	if (butt_state_ptr)
		*butt_state_ptr = butt_state;

	return ret;
}

int ctrl_draggable_circle(xy_t *pos, double radius, int *sel_id, int cur_id, int *dragged, col_t colour, rect_t pos_area)
{
	int ret;
	double total_scale = radius*zc.scrscale;
	double intensity = intensity_scaling(total_scale, 6.);
	rect_t box;
	ctrl_button_state_t butt_state={0};

	ret = ctrl_draggable_circle_invis(pos, radius, sel_id, cur_id, dragged, &butt_state, pos_area);
	if (ret == -10)
		return 0;

	if (butt_state.over)
		draw_circle(FULLCIRCLE, sc_xy(*pos), total_scale+drawing_thickness, drawing_thickness, colour, cur_blend, intensity);
	else
		draw_circle(HOLLOWCIRCLE, sc_xy(*pos), total_scale, drawing_thickness, colour, cur_blend, intensity);

	return ret;
}

/*int ctrl_draggable_circle_splineeq(xy_t *pos, double radius, int *sel_id, int cur_id, int *dragged, int handle_type, double mouseout_time)
{


	 
	   	hanc0 = make_colour_srgb(255, 224, 0, 255);	// yellow

	hanc1 = make_colour_srgb(32, 96, 224, 255);		// blue
	hanc2 = make_colour_srgb(255, 64, 96, 255);		// pink

	hanc1.a = gaussian(ci->mouseout_time * 1.) * ONEF + 0.5;
	hanc2.a = gaussian(ci->mouseout_time * 1.) * ONEF + 0.5;

		if (hh2)
			draw_circle(FULLCIRCLE, posx2, posy2, 5.8, aarad, hanc2, blend_blend, 1.);		// red dot full
		else
			draw_circle(HOLLOWCIRCLE, posx2, posy2, 5., aarad, hanc2, blend_alphablendfg, 1.);	// red dot

		if (hh1)
			draw_circle(FULLCIRCLE, posx1, posy1, 5.8, aarad, hanc1, blend_blend, 1.);		// blue handle dot full
		else
			draw_circle(HOLLOWCIRCLE, posx1, posy1, 5., aarad, hanc1, blend_alphablendfg, 1.);	// blue handle dot

		if (ci->eqs[i].id==ci->selband)
		{
			hanc3 = hanc0;	// yellow

			for (j=0; j<4; j++)
			{
				mix = 1. - fmod(ci->selrings_offset + (double) j * 0.25, 1.);
				diam = 0.*mix + 20.*(1.-mix);
				//hanc3.a = pow(mix, 2.) * 255. + 0.5;
				hanc3.a = pow(mix, 4.) * ONEF + 0.5;
				draw_circle(HOLLOWCIRCLE, posx0, posy0, diam, aarad, hanc3, blend_alphablendfg, 1.);	// yellow-orange dot
			}
		}

		if(hh0)
			draw_circle(HOLLOWCIRCLE, posx0, posy0, 5., aarad, hanc0, blend_blend, 1.);	// yellow dot
		else
			draw_circle(FULLCIRCLE, posx0, posy0, 5.8, aarad, hanc0, blend_blend, 1.);	// yellow dot
}*/

void update_ctrl_resizing_rect_positions(ctrl_resize_rect_t *state, rect_t box)
{
	int i;
	xy_t rpos;

	if (state == NULL)
		return;

	state->drag[0].pos = get_rect_centre(box);

	for (i=0; i<4; i++)
	{
		rpos = xy(i&1, i>>1);
		state->drag[i+1].pos = pos_in_rect_by_ratio(box, rpos);

		rpos = mul_xy(set_xy(0.5), xy( 2 - abs(i-1) , 2 - abs(i-2) ));
		state->drag[i+5].pos = pos_in_rect_by_ratio(box, rpos);
	}
}

int ctrl_resizing_rect(ctrl_resize_rect_t *state, rect_t *box)
{
	int i, ret;
	double intensity = 1.;
	xy_t dim = get_rect_dim(*box);
	double scale = min_of_xy(dim);
	double total_scale = scale*zc.scrscale;
	double aspect_ratio, cdim;
	rect_t cbox;

	if (total_scale < 3./fb->pixel_scale && state->dragged==0)
		return 0;

	aspect_ratio = min_of_xy(dim) / max_of_xy(dim);
	cdim = scale / (9.*aspect_ratio + 3.);		// size of the controls, from 1/12*scale for a square towards 1/3*scale

	if (check_box_on_screen(rect_add_margin(*box, set_xy(cdim*0.5)))==0 && state->dragged==0)
		return 0;

	intensity *= intensity_scaling(total_scale, 48.);

	if (state->init==0)
	{
		memset(state, 0, sizeof(ctrl_resize_rect_t));
		state->init = 1;

		state->drag[0].freedom = XY1;
		for (i=0; i<4; i++)
		{
			state->drag[i+1].freedom = XY1;
			state->drag[i+5].freedom = xy(i&1, i+1&1);
		}
	}

	update_ctrl_resizing_rect_positions(state, *box);

	// Input processing and dragging
	state->prev_dragged = state->dragged;
	state->dragged = 0;
	for (i=0; i < 9; i++)
	{
		ctrl_drag_set_dim(&state->drag[i], i==0 ? dim : set_xy(cdim));
		if (i > 0)		// make larger button that does nothing to absord misplaced edge clicks
			ctrl_button_invis(make_rect_centred(state->drag[i].pos, set_xy(cdim*1.2)), NULL);
		ret = ctrl_draggable(&state->drag[i]);
		if (ret)
		{
			state->dragged = 1;

			switch (i)
			{
				case 0:	*box = add_rect_xy(*box, state->drag[i].offset);	break;

				case 1:	box->p0 = state->drag[i].pos;				break;
				case 2:	rect_set_p10(box, state->drag[i].pos);			break;
				case 3:	rect_set_p01(box, state->drag[i].pos);			break;
				case 4:	box->p1 = state->drag[i].pos;				break;

				case 5:	box->p0.y = state->drag[i].pos.y;			break;
				case 6:	box->p1.x = state->drag[i].pos.x;			break;
				case 7:	box->p1.y = state->drag[i].pos.y;			break;
				case 8:	box->p0.x = state->drag[i].pos.x;			break;
			}

			update_ctrl_resizing_rect_positions(state, *box);
			dim = get_rect_dim(*box);
		}
	}

	if (state->prev_dragged && state->dragged==0)
		*box = sort_rect(*box);		// sort when the dragging just stopped

	// Draw controls
	for (i=0; i < 9; i++)
	{
		cbox = make_rect_centred(state->drag[i].pos, i==0 ? dim : set_xy(cdim));
		draw_rect(sc_rect(cbox), drawing_thickness, GUI_COL_DEF, cur_blend, intensity);
	}

	return state->dragged + state->prev_dragged*2;	// signal when the dragging just stopped so the final sorting is taken into account
}
