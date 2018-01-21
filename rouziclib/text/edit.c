textedit_t *cur_textedit=NULL;

void textedit_init(textedit_t *te)
{
	memset(te, 0, sizeof(textedit_t));
	te->string = calloc(te->alloc_size = 16, sizeof(char));
	te->max_scale = 1e30;
}

void textedit_free(textedit_t *te)
{
	textundo_free(te);
	free(te->string);
	memset(te, 0, sizeof(textedit_t));
}

textedit_t string_to_textedit(char *string)
{
	textedit_t te;

	memset(&te, 0, sizeof(textedit_t));
	te.string = string;
	te.alloc_size = strlen(string)+1;
	te.max_scale = 1e30;
	
	te.curpos = strlen(string);

	return te;
}

void textedit_erase_selection(textedit_t *te, int *len)
{
	int32_t sel0, sel1;

	if (te==NULL)
		return ;

	if (te->sel0==te->sel1)
		return ;

	textundo_update(te, 1);		// save the current state in undo

	sel0 = te->sel0;
	sel1 = te->sel1;
	minmax_i32(&sel0, &sel1);

	memmove(&te->string[sel0], &te->string[sel1], *len+1 - sel1);
	*len -= sel1 - sel0;
	te->curpos = sel0;
	te->sel0 = te->sel1 = 0;
}

void textedit_copy_selection_clipboard(textedit_t *te)
{
	char swap_char;
	int32_t sel0, sel1;

	if (te==NULL)
		return ;

	if (te->sel0==te->sel1)
		return ;

	sel0 = te->sel0;
	sel1 = te->sel1;
	minmax_i32(&sel0, &sel1);

	swap_char = te->string[sel1];
	te->string[sel1] = '\0';
	#ifdef RL_SDL
	SDL_SetClipboardText(&te->string[sel0]);
	#endif
	te->string[sel1] = swap_char;
}

int textedit_find_prev_linebreak(textedit_t *te)
{
	int i;

	for (i=te->curpos-1; i>0; i--)
		if (te->string[i]=='\n')
			return i+1;

	return 0;
}

int textedit_find_next_linebreak(textedit_t *te)
{
	int i;

	for (i=te->curpos; ; i++)
		if (te->string[i]=='\n' || te->string[i]=='\0')
			return i;
}

int textedit_find_prev_wordstart(textedit_t *te)
{
	int i;

	for (i=te->curpos-2; i>0; i--)
		if (te->string[i]==' ' || te->string[i]=='\t' || te->string[i]=='\n')
			if (te->string[i+1]!=' ' && te->string[i+1]!='\t' && te->string[i+1]!='\n')
				return i+1;

	return 0;
}

int textedit_find_next_wordstart(textedit_t *te)
{
	int i;

	for (i=te->curpos; ; i++)
	{
		if (te->string[i]=='\0')
			return i;

		if (te->string[i]==' ' || te->string[i]=='\t' || te->string[i]=='\n')
			if (te->string[i+1]!=' ' && te->string[i+1]!='\t' && te->string[i+1]!='\n')
				return i+1;
	}
}

void textedit_set_new_text(textedit_t *te, char *str)	// sets a whole new text from the string
{
	int32_t sel0, sel1, len;

	if (te==NULL)
		return ;

	textundo_update(te, 1);		// save the current state in undo

	len = strlen(str);
	alloc_enough(&te->string, len+1, &te->alloc_size, sizeof(char), 1.20);		// alloc enough extra space
	strcpy(te->string, str);							// copying of str
	
	te->curpos = len;
	te->sel0 = te->sel1 = 0;
}

void textedit_add(textedit_t *te, char *str, int32_t cmd, int32_t mod)
{
	int orig_len=0, ins_len, new_pos, char_len;
	char *clipboard;

	if (te==NULL)
		return ;

	if (te->string)
		orig_len = strlen(te->string);

	if (str)	// insert this string into the text
	{
		textundo_update(te, 0);
		textedit_erase_selection(te, &orig_len);

		ins_len = strlen(str);
		alloc_enough(&te->string, orig_len+ins_len+1, &te->alloc_size, sizeof(char), 1.20);		// alloc enough extra space
		memmove(&te->string[te->curpos+ins_len], &te->string[te->curpos], orig_len+1 - te->curpos);	// shift part of the text right of the cursor further right
		memcpy(&te->string[te->curpos], str, ins_len);							// insertion of str
		te->curpos += ins_len;
	}
	#ifdef RL_SDL
	else
	{
		// position changing
		switch (cmd)
		{
			case SDLK_LEFT:
			case SDLK_RIGHT:
			case SDLK_UP:
			case SDLK_DOWN:
			case SDLK_HOME:
			case SDLK_END:
				if (mod & KMOD_SHIFT)
				{
					if (te->sel0 == te->sel1)
						te->sel0 = te->sel1 = te->curpos;
				}
				else
					te->sel0 = te->sel1 = 0;

				switch (cmd)
				{
					case SDLK_LEFT:
						if (mod & KMOD_CTRL)
							te->curpos = textedit_find_prev_wordstart(te);
						else
							te->curpos = find_prev_utf8_char(te->string, te->curpos);
						break;
					case SDLK_RIGHT:
						if (mod & KMOD_CTRL)
							te->curpos = textedit_find_next_wordstart(te);
						else
							te->curpos = find_next_utf8_char(te->string, te->curpos);
						break;

					case SDLK_UP:
						te->curpos = te->curpos_up;
						break;
					case SDLK_DOWN:
						te->curpos = te->curpos_down;
						break;

					case SDLK_HOME:
						te->curpos = textedit_find_prev_linebreak(te);
						break;

					case SDLK_END:
						te->curpos = textedit_find_next_linebreak(te);
						break;
				}

				if (mod & KMOD_SHIFT)
					te->sel1 = te->curpos;
				break;
		}

		if ((mod & ~KMOD_NUM & ~KMOD_CAPS)==0)	// if there's no modifier key except maybe Num Lock and Caps Lock
		{
			switch (cmd)
			{
				case SDLK_RETURN:
				case SDLK_RETURN2:
				case SDLK_KP_ENTER:	// TODO notify of return
					if (te->edit_mode == te_mode_value)
						cur_textedit = NULL;
					else
					{
						textedit_erase_selection(te, &orig_len);
						textedit_add(te, "\n", 0, 0);
					}
					te->return_flag = 1;
					break;

				case SDLK_TAB:
					if (te->edit_mode == te_mode_value)
					{
						te->return_flag = 1;
						cur_textedit = NULL;
					}
					else
					{
						textedit_erase_selection(te, &orig_len);
						textedit_add(te, "\t", 0, 0);
					}
					break;

				case SDLK_BACKSPACE:
					if (te->sel0==te->sel1)
					{
						if (te->string[0])		// if the string isn't already empty
							textundo_update(te, 0);
						new_pos = find_prev_utf8_char(te->string, te->curpos);
						memmove(&te->string[new_pos], &te->string[te->curpos], orig_len+1 - te->curpos);	// shift right side of the text one utf8 character left
						te->curpos = new_pos;
					}
					else
						textedit_erase_selection(te, &orig_len);
					break;

				case SDLK_DELETE:
					if (te->sel0==te->sel1)
					{
						if (te->curpos != strlen(te->string))	// if we're not at the end of the string already
							textundo_update(te, 0);
						char_len = utf8_char_size(&te->string[te->curpos]);
						memmove(&te->string[te->curpos], &te->string[te->curpos+char_len], orig_len+1 - te->curpos - char_len);
					}
					else
						textedit_erase_selection(te, &orig_len);
					break;
			}
		}
		else if (mod & KMOD_CTRL)
		{
			switch (cmd)
			{
				case SDLK_v:
					clipboard = sdl_get_clipboard_dos_conv();
					if (clipboard==NULL)
						break;

					if (te->edit_mode == te_mode_value)
						replace_char(clipboard, '\n', ' ');

					textedit_add(te, clipboard, 0, 0);
					free(clipboard);
					break;

				case SDLK_c:
				case SDLK_x:
					if (te->sel0==te->sel1)
					{
						SDL_SetClipboardText(te->string);

						if (cmd==SDLK_x)
						{
							textundo_update(te, 1);
							te->string[0] = '\0';
							te->curpos = 0;
						}
					}
					else
					{
						textedit_copy_selection_clipboard(te);

						if (cmd==SDLK_x)
							textedit_erase_selection(te, &orig_len);
					}
					break;

				case SDLK_BACKSPACE:		// Delete All
					textundo_update(te, 1);
					te->string[0] = '\0';
					te->curpos = 0;
					break;

				case SDLK_a:			// Select All
					te->curpos = 0;
					te->sel0 = 0;
					te->sel1 = strlen(te->string);
					break;

				case SDLK_z:
					if (mod & KMOD_SHIFT)
						textundo_redo(te);
					else
						textundo_undo(te);
					break;

				case SDLK_y:
					textundo_redo(te);
					break;
			}
		}
	}
	#endif
}

void test_textedit_add()
{
	textedit_t te;

	te.alloc_size = 9;
	te.string = calloc(te.alloc_size, sizeof(char));
	sprintf(te.string, "abcdefgh");
	te.curpos = strlen(te.string) / 2;

	textedit_add(&te, "123", 0, 0);

	fprintf_rl(stdout, "%s\nalloc_size: %d\tcurpos: %d", te.string, te.alloc_size, te.curpos);
}

int ctrl_textedit_fullarg(raster_t fb, zoom_t zc, mouse_t mouse, vector_font_t *font, double drawing_thickness, 
		textedit_t *te, rect_t box, col_t colour)
{
	double intensity = 1.;
	double scale = MINN(fabs(box.p1.x-box.p0.x), fabs(box.p1.y-box.p0.y));
	double total_scale = scale*zc.scrscale;
	ctrl_button_state_t butt_state;
	rect_t boxb;

	if (total_scale < 1.)
		return 0;

	memset(&butt_state, 0, sizeof(ctrl_button_state_t));
	te->click_on = 0;

	if (mouse.window_focus_flag > 0)
		butt_state = proc_mouse_rect_ctrl(box, mouse);
		
	if (butt_state.once && te->sel0 == te->sel1)
		te->sel0 = te->sel1 = te->curpos;

	if (butt_state.once && mouse.mod_key[mouse_mod_shift]==0)
		if (mouse.b.clicks==1)
			te->sel0 = te->sel1 = -1;
		else if (mouse.b.clicks==2)
		{
			te->sel0 = textedit_find_prev_wordstart(te);
			te->sel1 = textedit_find_next_wordstart(te);
		}
		else if (mouse.b.clicks==3)
		{
			te->sel0 = textedit_find_prev_linebreak(te);
			te->sel1 = textedit_find_next_linebreak(te);
			if (te->string[te->sel1] != '\0')
				te->sel1++;
		}

	if (butt_state.down)
	{
		cur_textedit = te;
		te->click = sc_xy(mouse.u);
		te->click_on = 1;
	}

	te->curpos_up = 0;
	if (te->string)
		te->curpos_down = strlen(te->string);
	te->cur_screen_pos_prev = te->cur_screen_pos;

	intensity *= intensity_scaling(total_scale, 100.);

	draw_string_bestfit_asis(fb, font, te->string, sc_rect(box), 1./12., te->max_scale*0.1*total_scale, colour, intensity, drawing_thickness, te->draw_string_mode, NULL);
	//draw_string_bestfit(fb, font, te->string, sc_rect(box), 0., te->max_scale*0.1*total_scale, colour, intensity, drawing_thickness, ALIG_LEFT, NULL);
	//draw_string_fixed_thresh(fb, font, te->string, sc_rect(box), 66.*5.5, te->max_scale*0.1*total_scale, colour, intensity, drawing_thickness, ALIG_LEFT, NULL);
	//draw_rect_chamfer(fb, sc_rect(box), drawing_thickness, colour, blend_add, 0.5*intensity, 1./12.);

	if (te->rect_brightness > 0.)
		draw_rect(fb, sc_rect(box), drawing_thickness, colour, blend_add, intensity * te->rect_brightness);

	if (mouse.b.clicks==1)
	{
		if (butt_state.once && mouse.mod_key[mouse_mod_shift]==0)
			te->sel0 = te->curpos;
		if (butt_state.down)
			te->sel1 = te->curpos;
	}
	
	if (te->return_flag)
	{
		te->return_flag = 0;
		return 1;
	}
	return 0;
}

void draw_textedit_cursor(raster_t fb, xy_t offset, double scale, int bidi, int bidi_change, double drawing_thickness)
{
	static uint32_t t0=0;
	double frame_s;
	const double oscAp = 0.75;
	static double oscAn;
	xy_t pd, p0, p1;
	const col_t lime = make_colour(0., 0.5, 0., 0.);
	const col_t col2 = make_colour(1., 0.125, 0., 0.);
	col_t col;

	if (t0==0)
		t0 = get_time_ms();

	frame_s = (double) get_time_diff(&t0) * 0.001;
	frame_s = rangelimit(frame_s, 0., 0.1);
	oscAn = fmod(oscAn + frame_s / oscAp, 1.);

	col = bidi > 0 ? lime : col2;

	offset.y -= 3.*scale;
	pd.x = LETTERSPACING*0.5;
	if (bidi >= 0)
		pd.x = -pd.x;

	pd.y = (1. - sq(2.*oscAn - 1.)) * 1.5;

	p0 = pd;
	p1 = add_xy(pd, xy(0., 3.));
	draw_line_thin(fb, offset_scale(p0, offset, scale), offset_scale(p1, offset, scale), drawing_thickness, col, blend_add, 1.);
	draw_line_thin(fb, offset_scale(neg_y(p0), offset, scale), offset_scale(neg_y(p1), offset, scale), drawing_thickness, col, blend_add, 1.);
}
