textedit_t *cur_textedit=NULL, *prev_textedit=NULL, *next_textedit=NULL;

void textedit_init(textedit_t *te, const int alloc)
{
	//memset(te, 0, sizeof(textedit_t));
	if (alloc)
		te->string = calloc(te->alloc_size = 16, sizeof(char));

	if (te->max_scale==0.)
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
	textedit_t te={0};

	te.string = string;
	te.alloc_size = strlen(string)+1;
	te.max_scale = 1e30;

	te.curpos = strlen(string);

	return te;
}

void textedit_erase_selection(textedit_t *te, int *len)
{
	int sel0, sel1;

	if (te==NULL)
		return ;

	if (te->sel0==te->sel1 || te->sel0 < 0 || te->sel1 < 0)
		return ;

	textundo_update(te, 1);		// save the current state in undo

	sel0 = te->sel0;
	sel1 = te->sel1;
	minmax_i32(&sel0, &sel1);

	memmove(&te->string[sel0], &te->string[sel1], *len+1 - sel1);
	*len -= sel1 - sel0;
	te->curpos = sel0;
	te->sel0 = te->sel1 = 0;
	te->return_flag = 4;	// indicates modification
}

void textedit_copy_selection_clipboard(textedit_t *te)
{
	char swap_char;
	int sel0, sel1;

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

int textedit_find_next_wordend(textedit_t *te)
{
	int i;

	for (i=te->curpos; ; i++)
	{
		if (te->string[i]==' ' || te->string[i]=='\t' || te->string[i]=='\n' || te->string[i]=='\0')
			return i;
	}
}

void textedit_set_new_text(textedit_t *te, char *str)	// sets a whole new text from the string
{
	int sel0, sel1, len;

	if (te==NULL)
		return ;

	textundo_update(te, 1);		// save the current state in undo

	len = strlen(str);
	alloc_enough(&te->string, len+1, &te->alloc_size, sizeof(char), 1.20);		// alloc enough extra space
	strcpy(te->string, str);							// copying of str

	te->curpos = len;
	te->sel0 = te->sel1 = 0;
}

void textedit_clear_then_set_new_text(textedit_t *te, char *str)	// sets a whole new text from the string
{
	int sel0, sel1, len=0;

	if (te==NULL)
		return ;

	textundo_clear_all(te);

	if (str==NULL)
	{
		sprintf_realloc(&te->string, &te->alloc_size, 0, "");
	}
	else
	{
		len = strlen(str);
		alloc_enough(&te->string, len+1, &te->alloc_size, sizeof(char), 1.20);		// alloc enough extra space
		strcpy(te->string, str);							// copying of str
	}

	if (te->read_only==0)
	{
		te->curpos = len;
		te->sel0 = te->sel1 = 0;
	}
}

void textedit_add(textedit_t *te, char *str)
{
	int orig_len=0, ins_len, new_pos, char_len;
	char *clipboard=NULL;

	if (te==NULL)
		return ;

	if (te->string)
		orig_len = strlen(te->string);

	// Do things allowed in read_only
	if (str==NULL && get_kb_cmd())
	{
		if (get_key_state_by_name("c") >= 2 || get_key_state_by_name("x") >= 2)	// Copy/Cut
		{
			textedit_copy_selection_clipboard(te);

			if (get_key_state_by_name("x") >= 2 && te->read_only==0)
				textedit_erase_selection(te, &orig_len);
		}
		else if (get_key_state_by_name("a") >= 2)		// Select All
		{
			te->sel0 = 0;
			te->sel1 = strlen(te->string);
			te->curpos = te->sel1;
		}
	}

	if (te->read_only)
		return ;

	if (str)	// insert this string into the text
	{
		textundo_update(te, 0);
		textedit_erase_selection(te, &orig_len);

		ins_len = strlen(str);
		alloc_enough(&te->string, orig_len+ins_len+1, &te->alloc_size, sizeof(char), 1.20);		// alloc enough extra space
		if (te->curpos >= 0)
		{
			memmove(&te->string[te->curpos+ins_len], &te->string[te->curpos], orig_len+1 - te->curpos);	// shift part of the text right of the cursor further right
			memcpy(&te->string[te->curpos], str, ins_len);							// insertion of str
			te->curpos += ins_len;
		}
		te->return_flag = 4;	// indicates modification
	}
	else		// run commands (from keyboard shortcuts)
	{
		// position changing
		if (	mouse.key_state[RL_SCANCODE_LEFT] >= 2 ||
			mouse.key_state[RL_SCANCODE_RIGHT] >= 2 ||
			mouse.key_state[RL_SCANCODE_UP] >= 2 ||
			mouse.key_state[RL_SCANCODE_DOWN] >= 2 ||
			mouse.key_state[RL_SCANCODE_HOME] >= 2 ||
			mouse.key_state[RL_SCANCODE_END] >= 2 )
		{
			if (get_kb_shift())
			{
				if (te->sel0 == te->sel1)
					te->sel0 = te->sel1 = te->curpos;
			}
			else
				te->sel0 = te->sel1 = 0;

			if (mouse.key_state[RL_SCANCODE_LEFT] >= 2)
			{
				if (get_kb_cmd())
					te->curpos = textedit_find_prev_wordstart(te);
				else
					te->curpos = find_prev_utf8_char(te->string, te->curpos);
			}
			else if (mouse.key_state[RL_SCANCODE_RIGHT] >= 2)
			{
				if (get_kb_cmd())
					te->curpos = textedit_find_next_wordstart(te);
				else
					te->curpos = find_next_utf8_char(te->string, te->curpos);
			}
			else if (mouse.key_state[RL_SCANCODE_UP] >= 2)
				te->curpos = te->curpos_up;
			else if (mouse.key_state[RL_SCANCODE_DOWN] >= 2)
				te->curpos = te->curpos_down;
			else if (mouse.key_state[RL_SCANCODE_HOME] >= 2)
				te->curpos = textedit_find_prev_linebreak(te);
			else if (mouse.key_state[RL_SCANCODE_END] >= 2)
				te->curpos = textedit_find_next_linebreak(te);

			if (get_kb_shift())
				te->sel1 = te->curpos;
		}

		if (mouse.key_state[RL_SCANCODE_TAB] >= 2 && (get_kb_ctrl() | get_kb_alt() | get_kb_guikey())==0)
		{
			if (te->edit_mode == te_mode_value)
			{
				te->return_flag = 3;
				te->tab_switch = get_kb_shift() ? -1 : 1;	// flags switch to other editor
			}
			else
			{
				textedit_erase_selection(te, &orig_len);
				textedit_add(te, "\t");
			}
		}

		if (get_kb_all_mods() == 0)	// if there's no modifier key (shift, ctrl, guikey, alt)
		{
			if (get_kb_enter() >= 2)
			{
				if (te->edit_mode == te_mode_value)
					cur_textedit = NULL;
				else
				{
					textedit_erase_selection(te, &orig_len);
					textedit_add(te, "\n");
				}
				te->return_flag = 1;
			}
			else if (mouse.key_state[RL_SCANCODE_BACKSPACE] >= 2)
			{
				if (te->sel0==te->sel1)			// if there's no selection
				{					// just erase the previous character
					if (te->string[0])		// if the string isn't already empty
						textundo_update(te, 0);
					new_pos = find_prev_utf8_char(te->string, te->curpos);
					memmove(&te->string[new_pos], &te->string[te->curpos], orig_len+1 - te->curpos);	// shift right side of the text one utf8 character left
					te->curpos = new_pos;
					te->return_flag = 4;		// indicates modification
				}
				else
					textedit_erase_selection(te, &orig_len);
			}
			else if (mouse.key_state[RL_SCANCODE_DELETE] >= 2)
			{
				if (te->sel0==te->sel1)
				{
					if (te->curpos != strlen(te->string))	// if we're not at the end of the string already
						textundo_update(te, 0);
					char_len = utf8_char_size(&te->string[te->curpos]);
					memmove(&te->string[te->curpos], &te->string[te->curpos+char_len], orig_len+1 - te->curpos - char_len);
					te->return_flag = 4;		// indicates modification
				}
				else
					textedit_erase_selection(te, &orig_len);
			}
			else if (mouse.key_state[RL_SCANCODE_ESCAPE] == 2)	// Escape gets you out of the editor
			{
				cur_textedit = NULL;
			}
		}
		else if (get_kb_cmd())
		{
			if (get_key_state_by_name("v") >= 2)			// Paste
			{
				#ifdef RL_SDL
				clipboard = sdl_get_clipboard_dos_conv();
				#endif

				if (clipboard)
				{
					if (te->edit_mode == te_mode_value)
						replace_char(clipboard, '\n', ' ');

					textedit_add(te, clipboard);
					free(clipboard);
				}
			}
			else if (get_key_state_by_name("z") >= 2)		// Undo/Redo
			{
				if (get_kb_shift())
					textundo_redo(te);
				else
					textundo_undo(te);
			}
			else if (get_key_state_by_name("y") >= 2)		// Redo
				textundo_redo(te);
		}
	}
}

void textedit_prev_next_logic(textedit_t *te)	// Logic for finding the next and the previous textedits
{
	if (te->tab_switch)
	{
		cur_textedit = te->tab_switch==1 ? next_textedit : prev_textedit;
		te->tab_switch = 0;

		if (cur_textedit)
		{
			cur_textedit->was_cur_te = 1;

			// Select all the text
			cur_textedit->sel0 = 0;
			cur_textedit->sel1 = strlen(cur_textedit->string);
			cur_textedit->curpos = cur_textedit->sel1;
		}
	}

	if (next_textedit == NULL)
		next_textedit = te;

	if (cur_textedit == te)
		next_textedit = NULL;
	else
		prev_textedit = te;		// prev_textedit is only accurate when te is cur_textedit, which is fine
}

int ctrl_textedit(textedit_t *te, rect_t box, col_t colour)
{
	int ret;
	double intensity = 1.;
	double scale = rect_min_side(box);
	double total_scale = scale*zc.scrscale;
	ctrl_button_state_t butt_state={0};
	rect_t boxb;

	if (te->string==NULL)		// if te->string is NULL it's not initialised
		textedit_init(te, 1);

	textedit_prev_next_logic(te);

	if (total_scale < 1.)
		return 0;

	if (mouse.window_focus_flag > 0)
		butt_state = proc_mouse_rect_ctrl(box, mouse);

	te->click_on = 0;
	if (butt_state.orig && mouse.b.lmb > 0)
	{
		te->click = sc_xy(mouse.u);		// sets the click position so that the draw function may find it
		te->click_on = 1;
	}

	if (butt_state.once)		// on a new click
	{
		cur_textedit = te;	// any click unsets cur_textedit so every click into te must set it again

		if (te->sel0 == te->sel1)
			te->sel0 = te->sel1 = te->curpos;

		if (mouse.mod_key[mouse_mod_shift]==0)
			if (mouse.b.clicks==1)
				te->sel0 = te->sel1 = 0;	// single click: select nothing
			else if (mouse.b.clicks==2)		// double click: select word
			{
				te->sel0 = textedit_find_prev_wordstart(te);
				te->sel1 = textedit_find_next_wordend(te);
			}
			else if (mouse.b.clicks==3)		// triple click: select line
			{
				te->sel0 = textedit_find_prev_linebreak(te);
				te->sel1 = textedit_find_next_linebreak(te);
				//if (te->string[te->sel1] != '\0')
				//	te->sel1++;
			}
	}

	if (cur_textedit == te)
	{
		if (te->was_cur_te==0 && te->edit_mode==te_mode_value && te->first_click_no_sel==0)	// select all if te wasn't previously the cur_textedit
		{
			te->sel0 = 0;
			te->sel1 = strlen(te->string);
			te->curpos = te->sel1;
			te->sel_all = 1;	// indicate that the rest of the click must be ignored
		}

		te->was_cur_te = 1;
	}

	if (butt_state.uponce && te->sel_all==1)	// at the end of the click set curpos to the end
		te->curpos = te->sel1;

	if (te->was_cur_te && cur_textedit != te)	// if te was cur_textedit but not anymore
	{
		if (te->return_flag==0)
			te->return_flag = 2;		// this indicates that te was clicked out
		te->was_cur_te = 0;
	}

	// Set up the up-down cursor seeking by the draw function
	te->curpos_up = 0;
	if (te->string)
		te->curpos_down = strlen(te->string);
	te->cur_screen_pos_prev = te->cur_screen_pos;

	//**** Draw ****
	intensity *= intensity_scaling(total_scale, 24.);

	draw_string_bestfit_asis(font, te->string, sc_rect(box), 1./12., te->max_scale*0.1*total_scale, colour, intensity, drawing_thickness, te->draw_string_mode, NULL);
	//draw_string_bestfit(font, te->string, sc_rect(box), 0., te->max_scale*0.1*total_scale, colour, intensity, drawing_thickness, ALIG_LEFT, NULL);
	//draw_string_fixed_thresh(font, te->string, sc_rect(box), 66.*5.5, te->max_scale*0.1*total_scale, colour, intensity, drawing_thickness, ALIG_LEFT, NULL);

	// Rectangle frame drawing
	if (te->rect_brightness > 0.)
		draw_rect(sc_rect(box), drawing_thickness, colour, cur_blend, intensity * te->rect_brightness);
	//---- Draw ----

	// Selection (te->curpos is set by the draw_string_* function)
	if (mouse.b.clicks==1)
	{
		if (te->sel_all==0)
		{
			if (butt_state.once && mouse.mod_key[mouse_mod_shift]==0)
				te->sel0 = te->curpos;
			if (butt_state.orig && mouse.b.lmb > 0)
				te->sel1 = te->curpos;
		}

		if (butt_state.uponce)
			te->sel_all = 0;
	}

	prev_textedit = te;		// set prev_textedit to te in case there is only one textedit left to avoid referencing one that's gone

	ret = te->return_flag;
	te->return_flag = 0;
	return ret;			// returns 1 if Enter used, 2 if clicked out, 3 if Tab used, 4 if probably modified
}

void draw_textedit_cursor(xy_t offset, double scale, int bidi, int bidi_change, double drawing_thickness)
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
	draw_line_thin(offset_scale(p0, offset, scale), offset_scale(p1, offset, scale), drawing_thickness, col, cur_blend, 1.);
	draw_line_thin(offset_scale(neg_y(p0), offset, scale), offset_scale(neg_y(p1), offset, scale), drawing_thickness, col, cur_blend, 1.);
}
