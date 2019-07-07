//**** Colour ****

	// Make HSL colour
		// make_colour_hsl(hue, sat, lum (linear), hue type, sec_boost)
		col_t colour = make_colour_hsl(30., 1., 0.125, HUEDEG, 0);

	// Get colour from sinusoidal colour sequence
		// get_colour_seq(double x, xyz_t freq, xyz_t phase)
		col = get_colour_seq((double) i+1.5, xyz(0.36, 0.187, 0.13), set_xyz(0.2));

//**** Draw elements ****

	// Line
		// pixel coordinates
		draw_line_thin(sc_xy(p0), sc_xy(p1), drawing_thickness, white, blend_add, intensity);

	// Rectangles
		// pixel coordinates
		draw_rect(sc_rect(box), drawing_thickness, colour, blend_add, intensity);
		draw_rect_full(sc_rect(box), drawing_thickness, colour, blend_add, intensity);
		draw_black_rect(sc_rect(box), drawing_thickness);

	// Circle (HOLLOWCIRCLE or FULLCIRCLE)
		// pixel coordinates
		draw_circle(HOLLOWCIRCLE, sc_xy(circle_centre), circle_radius, drawing_thickness, colour, blend_add, intensity);

	// Text label
		// world coordinates
		draw_label("Control", offset_scale_rect(box, offset, sm), col, ALIG_CENTRE | MONODIGITS);

//**** Controls ****

	// Single button
		// returns 1 when released
		if (ctrl_button_chamf("Load", offset_scale_rect(box, offset, sm), colour))
		// invisible version
		ctrl_button_invis(offset_scale_rect(box, offset, sm), NULL)
		// or
		ctrl_button_state_t butt_state={0};
		ctrl_button_invis(offset_scale_rect(box, offset, sm), &butt_state)

	// Single checkbox
		// returns 1 if state changed
		// state = ptr to single int value
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
		static knob_t value_knob={0};
		if (value_knob.main_label==NULL)
			value_knob = make_knob("Knob name", default_value, knobf_linear, min_value, max_value, VALFMT_DEFAULT)
		ctrl_knob(&value, &value_knob, box, colour);

	// Text editor
		static textedit_t te={0};

		if (te.string==NULL)		// if te.string is NULL it's not initialised
			te = string_to_textedit(make_string_copy(string));
			// or (not needed as ctrl_textedit() can do it too)
			textedit_init(&te, 1);

			// some options can be specified like:
			te.edit_mode = te_mode_value;
			te.max_scale = 1./6.;
			te.rect_brightness = 0.25;

		// returns 1 if Enter (or sometimes Tab) is pressed, 2 when clicked out of it, 3 when tabbed out, 4 when modified
		if (ctrl_textedit(&te, offset_scale_rect(box, offset, sm), colour))

		// set next text while saving the previous one in the undo
		textedit_set_new_text(&te, "Label");

		// remove all the undo history then set the next text
		textedit_clear_then_set_new_text(&te, "Label");

	// Resizing rectangle
		static ctrl_resize_rect_t resize_state={0};
		static rect_t resize_box={0};

		if (resize_state.init==0)
			resize_box = make_rect_centred( XY0, XY1 );
		ctrl_resizing_rect(&resize_state, &resize_box);

	// Disable input processing for some controls
		// this works by saving the whole state of the control identification and restoring it to ignore any controls in between
		static mouse_ctrl_id_t ctrl_id_save={0};
		ctrl_id_save = *mouse.ctrl_id;
		ctrl_something();
		*mouse.ctrl_id = ctrl_id_save;

//**** Controls from text layout ****

	// How to make a new layout
		static gui_layout_t layout={0};
		const char *layout_src[] = {""};

		gui_layout_init_pos_scale(&layout, XY0, 1., XY0, 0);
		make_gui_layout(&layout, layout_src, sizeof(layout_src)/sizeof(char *), "Layout name");

	// Put it in the upper left corner
		gui_layout_init_pos_scale(&layout, add_xy(neg_x(zc.limit_u), neg_y(set_xy(0.25))), 1., XY0, 0);

	// Make it float permanently
		layout.offset = zc.offset_u;
		layout.sm = 1./zc.zoomscale;

	// Make a floating window
		// 2nd argument is a int * that can be NULL if a close button isn't needed
		// 4th argument must be a layout ID which by its rect defines the size of the window in local coordinates
		static flwindow_t window={0};

		flwindow_init_defaults(&window);
		flwindow_init_pinned(&window);
		draw_dialog_window_fromlayout(&window, &diag_on, &layout, id);

		// Example of window-defining elem
		"elem 0", "type none", "label Window Bar Title", "pos	0", "dim	8	6", "off	0	1", "",

	// Fit a sublayout into a layout's rectangle
		fit_sublayout_into_layout_rect(&window_layout, id_to_fit_into, &sublayout, id_of_frame, offset);

	// Get the rect of a layout element
		// the last argument is the provided offset
		gui_layout_elem_comp_area_os(&layout, id, XY0);

	// Label
		draw_label_fromlayout(&layout, id, ALIG_CENTRE | MONODIGITS);
		// optionally the label can be set prior to drawing using this:
		gui_printf_to_label(&layout, id, 0, "value %d", some_value);	// the 3rd argument is append

	// Rect
		// the first argument is 0 for outline rect, 1 for full rect, 2 for black rect
		draw_rect_fromlayout(0, &layout, id);

	// Button
		if (ctrl_button_fromlayout(&layout, id))
		if (ctrl_button_invis_fromlayout(NULL, &layout, id))

		ctrl_button_state_t butt_state={0};
		if (ctrl_button_invis_fromlayout(&butt_state, &layout, id))

	// Checkbox
		static int state=0;
		ctrl_checkbox_fromlayout(&state, &layout, id);

	// Knob
		static double value=NAN;
		ctrl_knob_fromlayout(&value, &layout, id);
		// set knob attribute
		get_knob_data_fromlayout(&layout, id)->min = 0.;
		// set knob circularity
		set_knob_circularity_fromlayout(1, &layout, id);

	// Text editor
		ctrl_textedit_fromlayout(&layout, id);
		// set text
		// the third argument is clear_undo
		print_to_layout_textedit(&layout, id, 1, "");
		// get textedit string
		string = get_textedit_string_fromlayout(&layout, id);

		// Example: using a text editor as a read-only log
		get_textedit_fromlayout(&layout, id)->read_only = 1;
		print_to_layout_textedit(&layout, id, 1, "");
		ctrl_textedit_fromlayout(&layout, id);

	// Selection menu
		const char *menu_opts[] = { "Opt 1", "Opt 2" };
		int menu_count = sizeof(menu_opts)/sizeof(char*);

		gui_layout_selmenu_set_count(menu_count, &layout, id);
		if (ctrl_selmenu_fromlayout(&layout, id))
			opt = menu_opts[get_selmenu_selid_fromlayout(&layout, id)];

		for (i=0; i < menu_count; i++)
			draw_selmenu_entry_fromlayout(i, menu_opts[i], &layout, id);

		// set the default option by id
		gui_layout_selmenu_set_entry_id(7, &layout, id);

//**** Keyboard input ****

	// Get state by scancode
		// 0 = nothing, 1 = down, 2 = newly down, 3 = repeated down event
		mouse.key_state[RL_SCANCODE_?]
		// and by name (see https://wiki.libsdl.org/SDL_Keycode for names)
		get_key_state_by_name("a")
		// compare with >= 2 for once and repeating, == 2 for once and no repeating, != 0 for down at all
		if (mouse.key_state[RL_SCANCODE_?] >= 2)

	// Get modifier keys
		get_kb_shift() (also ctrl, guikey, alt)
		// all of the above put together
		get_kb_all_mods()

	// Get all the return keys at once
		get_kb_enter()

//**** Images ****

	// Creating a raster
		// the first arg can be an array if it already exists
		r = make_raster(NULL, dim, XYI0, IMAGE_USE_FRGB);

	// Loading an image as a tiled mipmap
		mipmap_t image_mm={0};
		free_mipmap(&image_mm);
		image_mm = load_mipmap(path, IMAGE_USE_SQRGB);
		// or by HTTP
		image_mm = load_mipmap_from_http(url, IMAGE_USE_SQRGB);

	// Displaying
		// penultimate argument set to 1 keeps the pixel aspect ratio
		blit_in_rect(&r, sc_rect(image_frame), 1, LINEAR_INTERP);
		blit_mipmap_in_rect(image_mm, sc_rect(image_frame), 1, LINEAR_INTERP);

	// Updating
		// if a raster's contents are updated the drawqueue data copy of the new contents can be triggered before blitting like this:
		cl_unref_raster(&r);

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

	// check if a box is on screen or not
		if (check_box_on_screen(box_os))

	// Coordinates
		// for a xy_t point:
		offset_scale(p, offset, sm)

		// for a rect_t rectangle:
		offset_scale_rect(box, offset, sm)

	// Insert-spacing rect in text
		// An insert-spacing codepoint can be inserted into a text using \356\200\220 for cp_ins_w0 and \363\240\204\200 for an index of 0
		// if there are more than one cp_ins_w* characters the widths are summed
		// see vector_type/insert_rect.h for reference
		sprintf(string, "Inserted element goes -> \356\200\226\363\240\204\200 <- here");

		// reset the insert_rect array to clear outdated or bogus values
		reset_insert_rect_array();

		// the rectangle giving its position can be retrieved right after displaying
		// the string (and until the next reset) and turned to world coordinates
		// by its index (its order in the string) this way:
		rect_t insert_area = get_insert_rect(0);
		// the vertical range of the area can be changed from the default of 0 to 6 units
		insert_rect_change_height(get_insert_rect(0), 0., 4.5)

	// Fitting a rect inside a rect area
		// the 3rd argument works just like the offset in make_rect_off()
		frame = fit_rect_in_area(get_rect_dim(im_rect), area, xy(0.5, 0.5));

	// Subdividing an area into a smaller one by ratio and offset
		sub_area = get_subdiv_area(area, xy(1., 1./8.), xy(0.5, 1.));

//**** Parsing ****

	// Load a file and have an array of lines out of it
		// only array[0] then array need to be freed, since array[0] points to the original buffer
		// free_2d(array, 1); does it
		int linecount;
		char **array = arrayise_text(load_raw_file_dos_conv(path, NULL), &linecount);

	// Parse a dozenal fractional notation number
		// from a string pointer p into a double v
		// p is updated to point to after the number and its following whitespace
		p = string_parse_fractional_12(p, &v);

	// Read a UTF-8 character in a loop
		// i is the iterator, it's incremented further in case of a multi-byte character
		cp = utf8_to_unicode32(&string[i], &i);

	// Print a Unicode codepoint as UTF-8 in a string
		// generally the pointer should be able to accomodate 5 bytes
		sprint_unicode(buf, cp);

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

	// vsprintf equivalent but with allocation
		va_start(args, format);
		string = vsprintf_alloc(format, args);
		va_end(args);
		free(string);

//**** Threading ****

	// Init thread handle (not for detached threads)
		static rl_thread_t thread_handle=NULL;
	// Declare the thread data
		my_thread_data_t data={0};

	// before rl_thread_join the caller should signal to the thread function to quit using this element in the data struct
		volatile int thread_on;
		data.thread_on = 0;

	// Wait for thread to end (not for detached threads, use mutex instead)
		if (thread_handle)
			rl_thread_join_and_null(&thread_handle);

		// and before creating the thread:
		data.thread_on = 1;

	// Create thread
		rl_thread_create(&thread_handle, thread_function, &data);
		// or detached:
		rl_thread_create_detached(thread_function, &data);

	// Thread function prototype
		int thread_function(void *ptr)

	// Mutexes
		rl_mutex_t my_mutex;

		rl_mutex_init(&my_mutex);
		rl_mutex_lock(&my_mutex);
		rl_mutex_unlock(&my_mutex);
		rl_mutex_destroy(&my_mutex);

		// or
		rl_mutex_t *mutex_ptr;
		mutex_ptr = rl_mutex_init_alloc();
		rl_mutex_lock(mutex_ptr);
		rl_mutex_unlock(mutex_ptr);
		rl_mutex_destroy_free(&mutex_ptr);

	// Set priority for the current thread
		rl_thread_set_priority_low();
		rl_thread_set_priority_high();

	// Yield
		rl_thread_yield();

//**** Network ****

	// Getting a file from HTTP (needs #define RL_INCL_NETWORK and perhaps #define RL_LIBCURL)
		data_size = http_get("http://www.charbase.com/images/glyph/11910", -1, ONE_RETRY, &data, &data_alloc);

//**** Misc ****

	// Reset zoom and offset
		zoom_reset(&zc, &mouse.zoom_flag);

	// Change zoom or view offset
		// if either argument is NAN it is unchanged
		change_zoom(pos, NAN);
		change_zoom_and_turn_off_zoom_mode(pos, 4.);

	// Timing
		uint32_t td=0;
		get_time_diff(&td);
		function_to_time();
		fprintf_rl(stdout, "function_to_time() took %d ms\n\n", get_time_diff(&td));

	// Paths
		append_name_to_path(fullpath, path, name);	// puts 'path/name' into char fullpath[PATH_MAX*4], fullpath can be NULL in which case the function returns the allocated string
		remove_name_from_path(dirpath, fullpath);	// makes dirpath from fullpath without the final name (can be a folder) nor the last /. Any trailing input / is ignored
		remove_extension_from_path(outpath, fullpath);	// removes the extension
		create_dirs_for_file(filepath);			// create the necessary folders, good to call before writing a file

	// Folders
		// This loads a folder, the 3rd argument is -1 for full tree loading, 0 for excluding subfolders, >0 for a given depth level
		fs_dir_t dir={0};
		load_dir_depth(dir_path, &dir, 0);
		free_dir(&dir);
		// Go through each file of a subfolder and create the full path
		for (i=0; i < dir->subfile_count; i++)
			full_path[i] = append_name_to_path(NULL, dir->path, dir->subfile[i].name);

	// Drag and drop of files
		if (dropfile_get_count())
			path = dropfile_pop_first();

	// Generate a video from a raster struct
		ff_videnc_t d = ff_video_enc_init_file(path, r.dim, 29.97, AV_CODEC_ID_H265, bit_depth, crf);

		for (...)
			ff_video_enc_write_raster(&d, &r);

		ff_video_enc_finalise_file(&d);

	// Generate a video from tls framebuffer and layout
		static gui_layout_t layout={0};
		const char *layout_src[] = {
			"elem 10", "type none", "pos	0	0", "dim	32	18", "off	0.5", "",
			"elem 20", "type label", "pos	-8	8", "dim	3	2", "off	0	1", "",
		};

		layout.sm = 1.;
		make_gui_layout(&layout, layout_src, sizeof(layout_src)/sizeof(char *), "Video framebuffer layout");

		init_tls_fb(xyi(960, 540));
		ff_videnc_t d = ff_video_enc_init_file(path, fb.r.dim, 29.97, AV_CODEC_ID_H265, bit_depth, crf);

		for (...)
		{
			gui_printf_to_label(&layout, 20, 0, "%d", i);
			draw_label_fromlayout(&layout, 20, ALIG_RIGHT | MONODIGITS);

			ff_video_enc_write_raster(&d, &fb.r);
			memset(fb.r.f, 0, mul_x_by_y_xyi(fb.r.dim)*sizeof(frgb_t));
		}

		ff_video_enc_finalise_file(&d);
		free_raster(&fb.r);

	// FFTs
		// Choose out_size this way
		out_size = next_fast_fft_size(out_size);

		// 1D real to complex. For size n the output layout is c0, c1, c2, ..., cn/2-2, cn/2-1, cn/2, c-(n/2-1), c-(n/2-2), ..., c-2, c-1
		// therefore the positive frequency at c[i] is found at c[n-i]
		static cfft_plan_t plan={0};
		size_t out_as=0;
		cfft_1D_r2c_padded_fft(&plan, in, sizeof(in[0]), &out, &out_as, in_size, out_size);

		// Access positive frequency complex pairs this way, the negative frequencies are beyond out[out_size]
		for (i=0; i <= out_size; i+=2)
		{
			real = out[i];
			imag = out[i+1];
		}

//**** C syntax I can't ever remember ****

	// Function pointers as function arguments
		 void some_function(int (*func_ptr_name)(void*,int))

	// How to get one line from a string
		n=0;
		sscanf(p, "%[^\n]\n%n", line, &n);

	// Adding some stack checking
		#pragma strict_gs_check(on)
		#pragma check_stack(on)

		#pragma check_stack()
		#pragma strict_gs_check()

// How to transition code
	// 190610 transition that involves removing any 'fb' from any function call, making it a TLS global only
	// run this on every file that passes fb to functions
	%s/framebuffer_t \**fb\>,* *//e | %s/(&*\**fb, /(/e | %s/(&*\**fb)/()/e | %s/, fb,/,/e | %s/, fb)/)/e | %s/fb->/fb./ge
