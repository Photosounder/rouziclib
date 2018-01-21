//**** Colour ****

	// Make HSL colour
		// make_colour_hsl(hue, sat, lum (linear), hue type, sec_boost)
		col_t colour = make_colour_hsl(30., 1., 0.125, HUEDEG, 0);

//**** Draw elements ****

	// Line
		// pixel coordinates
		draw_line_thin(fb, sc_xy(p0), sc_xy(p1), drawing_thickness, white, blend_add, intensity);

	// Rectangles
		// pixel coordinates
		draw_rect(fb, sc_rect(box), drawing_thickness, colour, blend_add, intensity);
		draw_rect_full(fb, sc_rect(box), drawing_thickness, colour, intensity);
		draw_black_rect(fb, sc_rect(box), drawing_thickness);

	// Circle (HOLLOWCIRCLE or FULLCIRCLE)
		// pixel coordinates
		draw_circle(HOLLOWCIRCLE, fb, sc_xy(circle_centre), circle_radius, drawing_thickness, colour, blend_add, intensity);

	// Text label
		// world coordinates
		draw_label("Control", offset_scale_rect(box, offset, sm), col, ALIG_CENTRE | MONODIGITS);

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

	// List of radio controls
		// init radio_sel with the default choice, not necessarily 0
		// the box is for the first control, it's reproduced by an offset for the other controls
		static int radio_sel=0;
		const char *radio_labels[] = {"Radio 1", "Radio 2", "Radio 3", "Radio 4"};
		ctrl_array_radio(&radio_sel, sizeof(radio_labels)/sizeof(*radio_labels), radio_labels, &col, 1, offset_scale_rect(box, offset, sm), xy(0., -step*sm));

	// Knob
		// returns 2 if the value is being changed, 1 when the change is final
		// knobf functions are knobf_linear, knobf_log, knobf_recip
		static double value=NAN;
		ctrl_knob(&value, make_knob("Knob name", default_value, knobf_linear, min_value, max_value, VALFMT_DEFAULT), box, colour);

	// Text editor
		static textedit_t te;

		if (init)
			textedit_init(&te);
			// or
			te = string_to_textedit(make_string_copy(string));

		// returns 1 if Enter (or sometimes Tab) is pressed
		if (ctrl_textedit(&te, offset_scale_rect(box, offset, sm), colour))

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

	// Turn on the tool that generates rectangle code using a right-click drag
		rect_code_tool(offset, sm);
		// or if no offset/sm
		rect_code_tool(XY0, 1.);

	// Coordinates
		// for a xy_t point:
		offset_scale(p, offset, sm)

		// for a rect_t rectangle:
		offset_scale_rect(box, offset, sm)

	// Insert-spacing rect in text
		// An insert-spacing codepoint can be inserted into a text using "\xee\x80\x90" for cp_ins_w0 and "\xf3\xa0\x84\x80" for an index of 0 (can increment last byte up to 63)
		sprintf(string, "Inserted element goes -> ""\xee\x80\x96""\xf3\xa0\x84\x80"" <- here");

		// reset the insert_rect array to clear outdated or bogus values
		reset_insert_rect_array();

		// the rectangle giving its position can be retrieved right after displaying
		// the string (and until the next reset) and turned to world coordinates
		// by its index (its order in the string) this way:
		rect_t insert_area = get_insert_rect(0);
		// the vertical range of the area can be changed from the default of 0 to 6 units
		insert_rect_change_height(get_insert_rect(0), 0., 4.5)

//**** Parsing ****

	// Load a file and have an array of lines out of it
		// only array[0] then array need to be freed, since array[0] points to the original buffer
		char **filename_array = arrayise_text(load_raw_file(full_list_path, NULL), &linecount);

//**** Memory ****

	// Alloc more in an array if needed
		alloc_enough(&array, needed_count, &alloc_count, size_elem, inc_ratio);
		alloc_enough(&array, count+=1, &alloc_count, size_elem, inc_ratio);

	// safe sprintf that reallocs the string if needed
		// string can be NULL, then realloc will allocate it
		// Arg 2 can be NULL if the alloc count isn't known, not really a problem
		// alloc_count can also be 0 if the alloc count isn't know but we'd like to store the new alloc count
		// Arg 3 is 0 for no appending (normal sprintf() behaviour) or 1 for appending, like sprintf(&string[strlen(string)], 
		sprintf_realloc(&string, &alloc_count, 1, "%g", value);

//**** Threading ****

	// Init thread handle (not for detached threads)
		static thrd_t thread_handle=NULL;
	
	// Wait for thread to end (not for detached threads, use mutex instead)
		thrd_join(thread_handle, NULL);

	// Create thread
		thrd_create(&thread_handle, thread_function, &data_struct);
		// or detached:
		thrd_create_detached(thread_function, &data_struct);

	// Thread function prototype
		int thread_function(void *ptr)

	// Mutexes
		mtx_t my_mutex;

		mtx_init(&my_mutex, mtx_plain);
		mtx_lock(&my_mutex);
		mtx_unlock(&my_mutex);
		mtx_destroy(&my_mutex);

//**** Network ****
	
	// Getting a file from HTTP
		data_size = http_get("http://www.charbase.com/images/glyph/11910", -1, ONE_RETRY, &data, &data_alloc);
