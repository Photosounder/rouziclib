int ctrl_button_chamf_fullarg(raster_t fb, zoom_t zc, mouse_t mouse, vector_font_t *font, double drawing_thickness, 
		uint8_t *name, rect_t box, col_t colour)
{
	double intensity = 1.;
	double scale = rect_min_side(box);
	double total_scale = scale*zc.scrscale;
	ctrl_button_state_t butt_state;
	rect_t boxb;

	if (total_scale < 3.)
		return 0;

	if (check_box_box_intersection(box, zc.corners)==0)
		return 0;

	memset(&butt_state, 0, sizeof(ctrl_button_state_t));

	if (mouse.window_focus_flag > 0)
		butt_state = proc_mouse_rect_ctrl(box, mouse);

	intensity *= intensity_scaling(total_scale, 100.);

	draw_rect_chamfer(fb, sc_rect(box), drawing_thickness, colour, blend_add, 0.5*intensity, 1./12.);

	if (butt_state.over && butt_state.down==0)
	{
		boxb = get_rect_centred_coord(box);
		boxb.p1 = sub_xy(boxb.p1, set_xy(scale / 12.));
		boxb = make_rect_centred(boxb.p0, boxb.p1);
		draw_rect_chamfer(fb, sc_rect(boxb), drawing_thickness, colour, blend_add, 0.5*intensity, 1./12.*9./12.);
	}

	box.p0.x += (box.p1.x-box.p0.x)/16.;
	box.p1.x -= (box.p1.x-box.p0.x)/16.;
	draw_string_bestfit(fb, font, name, sc_rect(box), 0., 1e30*zc.scrscale, colour, 1.*intensity, drawing_thickness, ALIG_CENTRE, NULL);

	if (butt_state.uponce)
		return 1;
	return 0;
}

int ctrl_checkbox_fullarg(raster_t fb, zoom_t zc, mouse_t mouse, vector_font_t *font, double drawing_thickness, 
		int8_t *state, uint8_t *name, rect_t box, col_t colour)
{
	double intensity = 1.;
	double scale = rect_min_side(box);
	double total_scale = scale*zc.scrscale;
	ctrl_button_state_t butt_state;

	if (total_scale < 1.)
		return 0;

	if (check_box_box_intersection(box, zc.corners)==0)
		return 0;

	memset(&butt_state, 0, sizeof(ctrl_button_state_t));

	if (mouse.window_focus_flag > 0)
		butt_state = proc_mouse_rect_ctrl(box, mouse);

	intensity *= intensity_scaling(total_scale, 24.);

	if (butt_state.over && butt_state.down==0)
		draw_rect(fb, sc_rect(box), drawing_thickness, colour, blend_add, 0.25*intensity);

	if (butt_state.uponce)
		*state = (*state & 1) ^ 1;

	box.p0.x += 2.*scale/LINEVSPACING;
	box.p1.x -= 2.*scale/LINEVSPACING;
	draw_string_bestfit(fb, font, (*state==1) ? "\xE2\x98\x91" : (*state==0) ? "\xE2\x98\x90" : "\xF3\xB2\x98\x92", sc_rect(box), 0., 1e30*zc.scrscale, colour, 1.*intensity, drawing_thickness, ALIG_LEFT, NULL);

	box.p0.x += (8.+LETTERSPACING)*scale/LINEVSPACING;
	draw_string_bestfit(fb, font, name, sc_rect(box), 0., 1e30*zc.scrscale, colour, 1.*intensity, drawing_thickness, ALIG_LEFT, NULL);

	if (butt_state.uponce)
		return 1;
	return 0;
}

int ctrl_radio_fullarg(raster_t fb, zoom_t zc, mouse_t mouse, vector_font_t *font, double drawing_thickness, 
		int8_t state, uint8_t *name, rect_t box, col_t colour)
{
	double intensity = 1.;
	double scale = rect_min_side(box);
	double total_scale = scale*zc.scrscale;
	ctrl_button_state_t butt_state;

	if (total_scale < 1.)
		return 0;

	if (check_box_box_intersection(box, zc.corners)==0)
		return 0;

	memset(&butt_state, 0, sizeof(ctrl_button_state_t));

	if (mouse.window_focus_flag > 0)
		butt_state = proc_mouse_rect_ctrl(box, mouse);

	intensity *= intensity_scaling(total_scale, 24.);

	if (butt_state.over && butt_state.down==0)
		draw_rect(fb, sc_rect(box), drawing_thickness, colour, blend_add, 0.25*intensity);

	draw_circle(HOLLOWCIRCLE, fb, sc_xy(add_xy(box.p0, xy(0.5*scale, 0.5*scale))), 0.3*total_scale, drawing_thickness, colour, blend_add, intensity);
	if (state)
		draw_circle(FULLCIRCLE, fb, sc_xy(add_xy(box.p0, xy(0.5*scale, 0.5*scale))), 7./12.*0.3*total_scale, drawing_thickness, colour, blend_add, intensity);

	box.p0.x += 2.*scale/LINEVSPACING;
	box.p1.x -= 2.*scale/LINEVSPACING;

	box.p0.x += (8.+LETTERSPACING)*scale/LINEVSPACING;
	draw_string_bestfit(fb, font, name, sc_rect(box), 0., 1e30*zc.scrscale, colour, 1.*intensity, drawing_thickness, ALIG_LEFT, NULL);

	if (butt_state.uponce)
		return 1;
	return 0;
}

knob_t make_knob(char *main_label, double default_value, const knob_func_t func, double min, double max, int valfmt)
{
	knob_t knob;

	memset(&knob, 0, sizeof(knob));
	//knob.main_label = make_string_copy(main_label);
	knob.main_label = main_label;
	knob.default_value = default_value;
	knob.func = func;
	knob.min = min;
	knob.max = max;
	knob.valfmt = valfmt;

	return knob;
}

int ctrl_knob_value_button(raster_t fb, zoom_t zc, mouse_t mouse, vector_font_t *font, double drawing_thickness, 
		double *v, double t, double vt, double box_scale, xy_t centre, double scale, col_t colour)
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

	dv = v - vo;

	dvexp = normalised_notation_split(v, &dvm);

	if (dvm <= 2.)
		return 2.*dvexp;
	else if (dvm <= 5.)
		return 5.*dvexp;
	else
		return 10.*dvexp;
}

int ctrl_knob_value_buttons(raster_t fb, zoom_t zc, mouse_t mouse, vector_font_t *font, double drawing_thickness, 
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

		if (ctrl_knob_value_button(fb, zc, mouse, font, drawing_thickness, v, t, vt, box_scale, centre, scale, colour))
			ret = 1;
	}

	return ret;
}

int ctrl_knob_fullarg(raster_t fb, zoom_t zc, mouse_t mouse, vector_font_t *font, double drawing_thickness, 
		double *v_orig, knob_t knob, rect_t box, col_t colour)
{
	double intensity = 1.;
	double scale = rect_min_side(box);
	double total_scale = scale*zc.scrscale;
	ctrl_knob_state_t knob_state;
	char str[64];
	double v=*v_orig, t, th;
	rect_t valbox;
	xy_t p0, p1, centre = get_rect_centre(box);

	if (total_scale < 1.)
		return 0;

	if (isnan(v))
	{
		v = knob.default_value;
		*v_orig = v;
	}

	if (check_box_box_intersection(box, zc.corners)==0)
		return 0;

	t = knob.func(v, knob.min, knob.max, 1);

	memset(&knob_state, 0, sizeof(ctrl_knob_state_t));

	if (mouse.window_focus_flag > 0)
	{
		knob_state = proc_mouse_knob_ctrl(box, mouse);
		t += knob_state.vert_delta * 0.25;
	}

	t = rangelimit(t, 0., 1.);
	v = knob.func(t, knob.min, knob.max, 0);

	if (total_scale > 96.)
		if (ctrl_knob_value_buttons(fb, zc, mouse, font, drawing_thickness, &v, knob, centre, scale, colour))
			knob_state.uponce = 1;

	intensity *= intensity_scaling(total_scale, 24.);

//draw_rect(fb, sc_rect(box), drawing_thickness, colour, blend_add, 0.25*intensity);

	print_valfmt(str, sizeof(str), v, knob.valfmt);
	valbox = make_rect_centred(centre, set_xy(0.707*scale));
	draw_string_bestfit(fb, font, str, sc_rect(valbox), 0., 0.03*scale*zc.scrscale, colour, 1.*intensity, drawing_thickness, ALIG_CENTRE | MONODIGITS, NULL);

	valbox = make_rect_centred(add_xy(centre, xy(0., -5./12.*scale)), xy(7./12.*scale, 0.25*scale));
	draw_string_bestfit(fb, font, knob.main_label, sc_rect(valbox), 0., 0.03*scale*zc.scrscale, colour, 1.*intensity, drawing_thickness, ALIG_CENTRE | MONODIGITS, NULL);
//draw_rect(fb, sc_rect(valbox), drawing_thickness, colour, blend_add, 0.25*intensity);

	draw_circle_arc(fb, sc_xy(centre), set_xy(0.5*total_scale), -0.375, 0.375, drawing_thickness, colour, blend_add, 0.5*intensity);

	th = (t * 0.75 - 0.375) * -2.*pi;
	p0 = rotate_xy2(xy(0., 0.5*scale), th);
	p1 = rotate_xy2(xy(0., 0.4*scale), th);
	draw_line_thin(fb, sc_xy(add_xy(centre, p0)), sc_xy(add_xy(centre, p1)), drawing_thickness, colour, blend_add, 2.*intensity);

	*v_orig = v;

	if (knob_state.uponce)
		return 1;
	if (knob_state.down)
		return 2;
	return 0;
}

ctrl_drag_state_t make_drag_state(xy_t pos, xy_t freedom)
{
	ctrl_drag_state_t state;

	memset(&state, 0, sizeof(ctrl_drag_state_t));

	state.pos = pos;
	state.freedom = freedom;

	return state;
}

int ctrl_draggable_fullarg(raster_t fb, zoom_t zc, mouse_t mouse, vector_font_t *font, double drawing_thickness, 
		ctrl_drag_state_t *state, uint8_t *name, xy_t dim, col_t colour)
{
	double intensity = 1.;
	double scale = min_of_xy(dim);
	double total_scale = scale*zc.scrscale;
	rect_t box;

	if (total_scale < 3. && state->down==0)
		return 0;

	box = make_rect_off( state->pos, dim, xy(0.5, 0.5) );

	if (check_box_box_intersection(box, zc.corners)==0 && state->down==0)
		return 0;

	if (mouse.window_focus_flag > 0)
		proc_mouse_draggable_ctrl(state, box, mouse);

	box = make_rect_off( state->pos, dim, xy(0.5, 0.5) );
	intensity *= intensity_scaling(total_scale, 100.);
	draw_rect_chamfer(fb, sc_rect(box), drawing_thickness, colour, blend_add, 0.5*intensity, 1./12.);
	draw_string_bestfit(fb, font, name, sc_rect(rect_add_margin(box, xy(-(box.p1.x-box.p0.x)/8., 0.))), 0., 1e30*zc.scrscale, colour, 1.*intensity, drawing_thickness, ALIG_CENTRE, NULL);

	return 0;
}
