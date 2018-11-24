int ctrl_button_invis(rect_t box, ctrl_button_state_t *butt_state_ptr)
{
	double total_scale = rect_min_side(box)*zc.scrscale;
	ctrl_button_state_t butt_state={0};

	if (butt_state_ptr)
		*butt_state_ptr = butt_state;

	if (total_scale < 3.)	// return if the button is too small
	{
		if (butt_state_ptr)
			butt_state_ptr->too_small = 1;
		return 0;
	}

	// return if the button is off-screen
	if (check_box_box_intersection(box, zc.corners_dl)==0)
	{
		if (butt_state_ptr)
			butt_state_ptr->out_of_screen = 1;
		return 0;
	}

	if (zc.mouse->window_focus_flag > 0)
		butt_state = proc_mouse_rect_ctrl(box, *zc.mouse);

	if (butt_state_ptr)
		*butt_state_ptr = butt_state;

	return butt_state.uponce;
}

int ctrl_button_chamf(uint8_t *name, rect_t box, col_t colour)
{
	double intensity = 1.;
	double scale = rect_min_side(box);
	double total_scale = scale*zc.scrscale;
	ctrl_button_state_t butt_state={0};
	rect_t boxb;

	ctrl_button_invis(box, &butt_state);
	if (butt_state.too_small || butt_state.out_of_screen)
		return 0;

	intensity *= intensity_scaling(total_scale, 48.);

	draw_rect_chamfer(fb, sc_rect(box), drawing_thickness, colour, cur_blend, 0.5*intensity, 1./12.);

	if (butt_state.over && butt_state.down==0)
	{
		boxb = get_rect_centred_coord(box);
		boxb.p1 = sub_xy(boxb.p1, set_xy(scale / 12.));
		boxb = make_rect_centred(boxb.p0, boxb.p1);
		draw_rect_chamfer(fb, sc_rect(boxb), drawing_thickness, colour, cur_blend, 0.5*intensity, 1./12.*9./12.);
	}

	box = rect_add_margin(box, set_xy(-scale/6.));
	draw_string_bestfit(fb, font, name, sc_rect(box), 0., 1e30*zc.scrscale, colour, 1.*intensity, drawing_thickness, ALIG_CENTRE, NULL);

	return butt_state.uponce;
}

int ctrl_checkbox(int *state, uint8_t *name, rect_t box, col_t colour)
{
	double intensity = 1.;
	double scale = rect_min_side(box);
	double total_scale = scale*zc.scrscale;
	ctrl_button_state_t butt_state={0};

	ctrl_button_invis(box, &butt_state);
	if (butt_state.too_small || butt_state.out_of_screen)
		return 0;

	intensity *= intensity_scaling(total_scale, 24.);

	if (butt_state.over && butt_state.down==0)
		draw_rect(fb, sc_rect(box), drawing_thickness, colour, cur_blend, 0.25*intensity);

	if (butt_state.uponce && state)
		*state = (*state & 1) ^ 1;

	box = rect_add_margin(box, xy(-2.*scale/LINEVSPACING, 0.));
	if (state)
		draw_string_bestfit(fb, font, (*state==1) ? "\xE2\x98\x91" : (*state==0) ? "\xE2\x98\x90" : "\xF3\xB2\x98\x92", sc_rect(box), 0., 1e30*zc.scrscale, colour, 1.*intensity, drawing_thickness, ALIG_LEFT, NULL);
	else
		draw_string_bestfit(fb, font, "\xE2\x98\x90", sc_rect(box), 0., 1e30*zc.scrscale, colour, 1.*intensity, drawing_thickness, ALIG_LEFT, NULL);

	box.p0.x += (8.+LETTERSPACING)*scale/LINEVSPACING;
	draw_string_bestfit(fb, font, name, sc_rect(box), 0., 1e30*zc.scrscale, colour, 1.*intensity, drawing_thickness, ALIG_LEFT, NULL);

	return butt_state.uponce;
}

int ctrl_radio(int state, uint8_t *name, rect_t box, col_t colour)
{
	double intensity = 1.;
	double scale = rect_min_side(box);
	double total_scale = scale*zc.scrscale;
	ctrl_button_state_t butt_state={0};

	ctrl_button_invis(box, &butt_state);
	if (butt_state.too_small || butt_state.out_of_screen)
		return 0;

	intensity *= intensity_scaling(total_scale, 24.);

	if (butt_state.over && butt_state.down==0)
		draw_rect(fb, sc_rect(box), drawing_thickness, colour, cur_blend, 0.25*intensity);

	draw_circle(HOLLOWCIRCLE, fb, sc_xy(add_xy(box.p0, xy(0.5*scale, 0.5*scale))), 0.3*total_scale, drawing_thickness, colour, cur_blend, intensity);
	if (state)
		draw_circle(FULLCIRCLE, fb, sc_xy(add_xy(box.p0, xy(0.5*scale, 0.5*scale))), 7./12.*0.3*total_scale, drawing_thickness, colour, cur_blend, 1.);

	if (name)
	{
		box = rect_add_margin(box, xy(-2.*scale/LINEVSPACING, 0.));
		box.p0.x += (8.+LETTERSPACING)*scale/LINEVSPACING;
		draw_string_bestfit(fb, font, name, sc_rect(box), 0., 1e30*zc.scrscale, colour, 1.*intensity, drawing_thickness, ALIG_LEFT, NULL);
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
		return make_rect_off( add_xy(start , mul_xy(get_rect_dim(box), xy(0., -id))) , get_rect_dim(box), xy(0., 1.) );
}

int ctrl_selectmenu(ctrl_selectmenu_state_t *state, rect_t box, col_t colour)
{
	double intensity = 1.;
	double total_scale = rect_min_side(box)*zc.scrscale;
	ctrl_button_state_t butt_state={0}, subbutt_state={0};
	rect_t open_box, entry_box;
	int i, ret = 0, over_something=0;

	state->open = state->next_open;

	ctrl_button_invis(box, &butt_state);
	if (butt_state.too_small || (state->open==0 && butt_state.out_of_screen))
		return 0;

	if (butt_state.uponce)
		state->next_open ^= 1;

	if (butt_state.over)
		over_something = 1;

	intensity *= intensity_scaling(total_scale, 24.);

	draw_rect(fb, sc_rect(box), drawing_thickness, colour, cur_blend, intensity);

	state->hover_id = -1;
	if (state->open)
	{
		open_box = selectmenu_rect(box, -state->count);
		draw_black_rect(fb, sc_rect(open_box), drawing_thickness);
		draw_rect(fb, sc_rect(open_box), drawing_thickness, colour, cur_blend, intensity);

		for (i=0; i < state->count; i++)
		{
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
				draw_rect_full(fb, sc_rect(entry_box), drawing_thickness, make_colour_hsl(220., 0.5, 0.08, HUEDEG, 0), cur_blend, 1.);
			}
		}
	}

	if (over_something==0 && mouse.b.lmb==2)
		state->next_open = 0;

	return ret;
}

void draw_selectmenu_entry(ctrl_selectmenu_state_t *state, rect_t box, col_t colour, int id, const char *label)
{
	double scale = rect_min_side(box);
	rect_t entry_box, main_label_box;

	if (id == state->sel_id)
	{
		main_label_box = box;
		main_label_box.p0.x += 3.*scale/LINEVSPACING;
		main_label_box.p1.x -= scale;
		main_label_box = rect_add_margin(main_label_box, xy(0., -2.*scale/LINEVSPACING));
		draw_label(label, main_label_box, colour, ALIG_LEFT);
		draw_label("\xf3\xb0\x84\x80", make_rect_off(box.p1, set_xy(scale), xy(1., 1.)), colour, ALIG_CENTRE);
	}

	entry_box = selectmenu_rect(box, id);

	if (state->open)
	{
		if (id == state->sel_id)
			draw_label("\xe2\x9c\x94", rect_add_margin(make_rect_off(entry_box.p0, set_xy(scale), XY0), set_xy(-1.5/12.*scale)), colour, ALIG_CENTRE);

		entry_box.p0.x += scale;
		entry_box.p1.x -= 3.*scale/LINEVSPACING;
		entry_box = rect_add_margin(entry_box, xy(0., -2.*scale/LINEVSPACING));
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

	return knob;
}

int ctrl_knob_value_button(double *v, double t, double vt, double box_scale, xy_t centre, double scale, col_t colour)
{
	char str[64];
	double th;
	xy_t pos;
	rect_t box;

	th = (t * 0.75 - 0.375) * -2.*pi;
	pos = rotate_xy2(xy(0., 0.5-0.1*box_scale), th);
	box = make_rect_centred( pos, mul_xy(set_xy(box_scale), xy(0.1, 0.04)) );
	sprintf(str, "%g", vt);

	if (ctrl_button_chamf(str, offset_scale_rect(box, centre, scale), colour))
	{
		*v = vt;
		return 1;
	}

	return 0;
}

double find_next_round_knob_value(double vo, knob_t knob, double t_step)
{
	double to, t, v, dv, dvexp, dvm;

	to = knob.func(vo, knob.min, knob.max, 1);
	v = knob.func(MINN(1., to+t_step), knob.min, knob.max, 0);

	//dv = v - vo;

	dvexp = normalised_notation_split(v, &dvm);

	if (dvm <= 2.)
		return 2.*dvexp;
	else if (dvm <= 5.)
		return 5.*dvexp;
	else
		return 10.*dvexp;
}

int ctrl_knob_value_buttons(framebuffer_t fb, zoom_t zc, mouse_t mouse, vector_font_t *font, double drawing_thickness, 
		double *v, knob_t knob, xy_t centre, double scale, col_t colour)
{
	int i, ret=0;
	double lt, t, vt, box_scale;

	/*for (i=0, vt=knob.min; i >= 0 && vt<=knob.max; i++)
	{
		vt = find_next_round_knob_value(vt, knob, 24. / (scale*zc.scrscale));
		//vt = knob.func(t, knob.min, knob.max, 0);
		box_scale = 1.;//500. / (scale*zc.scrscale);

		t = knob.func(vt, knob.min, knob.max, 1);
		if (t > 1.)
			break;

		if (ctrl_knob_value_button(fb, zc, mouse, font, drawing_thickness, v, t, vt, box_scale, centre, scale, colour))
			ret = 1;
	}*/

	double log_min, log_max;

	if (knob.min==0. || knob.max==0.)	// FIXME can't be 0
		return 0;

	log_min = log10(knob.min);
	log_max = log10(knob.max);

	for (lt=ceil(log_min); lt < log_max; lt+=1.)
	{
		vt = pow(10., lt);
		box_scale = 1.;

		t = knob.func(vt, knob.min, knob.max, 1);
		if (t > 1.)
			break;

		if (ctrl_knob_value_button(v, t, vt, box_scale, centre, scale, colour))
			ret = 1;
	}

	return ret;
}

int ctrl_knob(double *v_orig, knob_t knob, rect_t box, col_t colour)
{
	double intensity = 1.;
	double scale = rect_min_side(box);
	double total_scale = scale*zc.scrscale;
	ctrl_knob_state_t knob_state={0};
	char str[64];
	double v=NAN, t, t_off=0., th;
	rect_t valbox;
	xy_t p0, p1, centre = get_rect_centre(box);

	if (v_orig)
		v = *v_orig;

	if (isnan(v))	// initialise the value
	{
		v = knob.default_value;
		if (v_orig)
			*v_orig = v;
	}

	v = rangelimit(v, knob.min, knob.max);
	if (v_orig)
		*v_orig = v;

	if (total_scale < 1.)
		return 0;

	if (check_box_box_intersection(box, zc.corners_dl)==0)
		return 0;

	// process input
	if (mouse.window_focus_flag > 0)
	{
		knob_state = proc_mouse_knob_ctrl(box, mouse);
		t_off = knob_state.vert_delta * 0.25;
	}

	// reset on doubleclick
	if (knob_state.doubleclick)
	{
		v = knob.default_value;
		if (v_orig)
			*v_orig = v;
	}

	// calculate new position and value
	t = knob.func(v, knob.min, knob.max, 1);

	t = rangelimit(t+t_off, 0., 1.);
	v = knob.func(t, knob.min, knob.max, 0);

	// show value buttons
	/*if (total_scale > 96.)
		if (ctrl_knob_value_buttons(fb, zc, mouse, font, drawing_thickness, &v, knob, centre, scale, colour))
			knob_state.uponce = 1;*/

	// draw knob
	intensity *= intensity_scaling(total_scale, 24.);

	sprintf(str, knob.fmt_str, v);
	valbox = make_rect_centred(centre, set_xy(0.707*scale));
	draw_string_bestfit(fb, font, str, sc_rect(valbox), 0., 0.03*scale*zc.scrscale, colour, 1.*intensity, drawing_thickness, ALIG_CENTRE | MONODIGITS, NULL);

	valbox = make_rect_centred(add_xy(centre, xy(0., -5./12.*scale)), xy(7./12.*scale, 0.25*scale));
	draw_string_bestfit(fb, font, knob.main_label, sc_rect(valbox), 0., 0.03*scale*zc.scrscale, colour, 1.*intensity, drawing_thickness, ALIG_CENTRE | MONODIGITS, NULL);

	draw_circle_arc(fb, sc_xy(centre), set_xy(0.5*total_scale), -0.375, 0.375, drawing_thickness, colour, cur_blend, 0.5*intensity);

	th = (t * 0.75 - 0.375) * -2.*pi;
	p0 = rotate_xy2(xy(0., 0.5*scale), th);
	p1 = rotate_xy2(xy(0., 0.4*scale), th);
	draw_line_thin(fb, sc_xy(add_xy(centre, p0)), sc_xy(add_xy(centre, p1)), drawing_thickness, colour, cur_blend, 2.*intensity);

	if (v_orig)
		*v_orig = v;

	if (knob_state.uponce || knob_state.doubleclick)	// the final value change when the mouse button is released
		return 1;
	if (knob_state.down)					// an ongoing value change when the mouse button is held down
		return 2;
	return 0;
}

ctrl_drag_state_t make_drag_state(xy_t pos, xy_t freedom)
{
	ctrl_drag_state_t state={0};

	state.pos = pos;
	state.freedom = freedom;

	return state;
}

int ctrl_draggable(ctrl_drag_state_t *state, xy_t dim)
{
	int ret=0;
	double total_scale = min_of_xy(dim)*zc.scrscale;
	rect_t box;

	if (total_scale < 3. && state->down==0)
		return 0;

	box = make_rect_off( state->pos, dim, xy(0.5, 0.5) );

	if (check_box_box_intersection(box, zc.corners_dl)==0 && state->down==0)
		return 0;

	if (mouse.window_focus_flag > 0)
		ret = proc_mouse_draggable_ctrl(state, box, mouse);

	return ret;
}

int ctrl_draggable_circle_invis(xy_t *pos, double radius, int *sel_id, int cur_id, int *dragged, ctrl_button_state_t *butt_state_ptr)
{
	int ret=0;
	double total_scale = radius*zc.scrscale;
	rect_t box;
	ctrl_button_state_t butt_state={0};

	if (total_scale < 0.2 && *dragged != cur_id)
		return -10;

	box = make_rect_off( *pos, set_xy(2.*radius), xy(0.5, 0.5) );
	if (check_box_box_intersection(box, zc.corners_dl)==0 && *dragged != cur_id)
		return -10;

	if (mouse.window_focus_flag > 0)
	{
		butt_state = proc_mouse_circular_ctrl(pos, radius, mouse, *dragged == cur_id);

		if (butt_state.once)
		{
			ret = 1;
			*dragged = cur_id;
			*sel_id = cur_id;
		}

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

int ctrl_draggable_circle(xy_t *pos, double radius, int *sel_id, int cur_id, int *dragged, col_t colour)
{
	int ret;
	double total_scale = radius*zc.scrscale;
	double intensity = intensity_scaling(total_scale, 6.);
	rect_t box;
	ctrl_button_state_t butt_state={0};

	ret = ctrl_draggable_circle_invis(pos, radius, sel_id, cur_id, dragged, &butt_state);
	if (ret == -10)
		return 0;

	if (butt_state.over)
		draw_circle(FULLCIRCLE, fb, sc_xy(*pos), total_scale+drawing_thickness, drawing_thickness, colour, cur_blend, intensity);
	else
		draw_circle(HOLLOWCIRCLE, fb, sc_xy(*pos), total_scale, drawing_thickness, colour, cur_blend, intensity);

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
			draw_circle(FULLCIRCLE, ci->fb, posx2, posy2, 5.8, aarad, hanc2, blend_blend, 1.);		// red dot full
		else
			draw_circle(HOLLOWCIRCLE, ci->fb, posx2, posy2, 5., aarad, hanc2, blend_alphablendfg, 1.);	// red dot

		if (hh1)
			draw_circle(FULLCIRCLE, ci->fb, posx1, posy1, 5.8, aarad, hanc1, blend_blend, 1.);		// blue handle dot full
		else
			draw_circle(HOLLOWCIRCLE, ci->fb, posx1, posy1, 5., aarad, hanc1, blend_alphablendfg, 1.);	// blue handle dot

		if (ci->eqs[i].id==ci->selband)
		{
			hanc3 = hanc0;	// yellow

			for (j=0; j<4; j++)
			{
				mix = 1. - fmod(ci->selrings_offset + (double) j * 0.25, 1.);
				diam = 0.*mix + 20.*(1.-mix);
				//hanc3.a = pow(mix, 2.) * 255. + 0.5;
				hanc3.a = pow(mix, 4.) * ONEF + 0.5;
				draw_circle(HOLLOWCIRCLE, ci->fb, posx0, posy0, diam, aarad, hanc3, blend_alphablendfg, 1.);	// yellow-orange dot
			}
		}

		if(hh0)
			draw_circle(HOLLOWCIRCLE, ci->fb, posx0, posy0, 5., aarad, hanc0, blend_blend, 1.);	// yellow dot
		else
			draw_circle(FULLCIRCLE, ci->fb, posx0, posy0, 5.8, aarad, hanc0, blend_blend, 1.);	// yellow dot
}*/

void update_ctrl_resizing_rect_positions(ctrl_resize_rect_t *state, rect_t box)
{
	int i;
	xy_t rpos;

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

	if (total_scale < 3. && state->dragged==0)
		return 0;

	aspect_ratio = min_of_xy(dim) / max_of_xy(dim);
	cdim = scale / (9.*aspect_ratio + 3.);		// size of the controls, from 1/12*scale for a square towards 1/3*scale

	if (check_box_box_intersection(rect_add_margin(*box, set_xy(cdim*0.5)), zc.corners_dl)==0 && state->dragged==0)
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
	for (i=0; i<9; i++)
	{
		ret = ctrl_draggable(&state->drag[i], i==0 ? dim : set_xy(cdim));
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

	// Draw controls
	for (i=0; i<9; i++)
	{
		cbox = make_rect_centred(state->drag[i].pos, i==0 ? dim : set_xy(cdim));
		draw_rect(fb, sc_rect(cbox), drawing_thickness, GUI_COL_DEF, cur_blend, intensity);
	}

	return state->dragged;
}
