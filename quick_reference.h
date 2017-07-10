//**** Colour ****

	// Make HSL colour
		// make_colour_hsl(hue, sat, lum (linear), hue type, sec_boost)

		col_t colour = make_colour_hsl(30., 1., 0.125, HUEDEG, 0);

//**** Draw elements ****

	// Line
		// pixel coordinates

		draw_line_thin(fb, sc_xy(p0), sc_xy(p1), drawing_thickness, white, blend_add, intensity);

	// Full rectangle
		// pixel coordinates

		draw_rect_full(fb, sc_rect(box), drawing_thickness, colour, intensity);

	// Circle (HOLLOWCIRCLE or FULLCIRCLE)
		// pixel coordinates
		draw_circle(HOLLOWCIRCLE, fb, sc_xy(circle_centre), circle_radius, drawing_thickness, colour, blend_add, intensity);

//**** Controls ****

	// Single button
		// returns 1 when released
		
		if (ctrl_button_chamf("Load", offset_scale_rect(box, offset, sm), colour))

	// Single checkbox
		// returns 1 if state changed
		// state = ptr to single int8_t value

		ctrl_checkbox(&state, "Tick this", offset_scale_rect(box, offset, sm), colour);

		if (ctrl_checkbox(&state, "Tick this", offset_scale_rect(box, offset, sm), colour))
			change = 1;

	// Knob
		// returns 2 if the value is being changed, 1 when the change is final
		// knobf functions are knobf_linear, knobf_log, knobf_recip

		static double value=NAN;
		ctrl_knob(&value, make_knob("Knob name", default_value, knobf_linear, min_value, max_value), box, colour);

//**** Layout ****

	// The function must take rect_t area as an argument
	// it should then start with some of the following:
	
	{
		xy_t offset;
		double sm;
		rect_t main_frame;
		const double margin = 12.;

		main_frame = /* TODO define the full frame in local units to fit into the given area */;
		offset = fit_into_area(area, main_frame, margin, &sm);

		// check if any of the main_frame (+margins) gets displayed at all and draw the frame and possibly titled overlay
		if (dialog_enclosing_frame(offset, sm, main_frame, margin, "Options", make_grey(0.25)))
			return;
	}

	// Coordinates
		// for a xy_t point:
		offset_scale(p, offset, sm)
		// for a rect_t rectangle:
		offset_scale_rect(box, offset, sm)
