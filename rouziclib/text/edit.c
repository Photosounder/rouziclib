textedit_t *cur_textedit=NULL;

void textedit_init(textedit_t *te)
{
	int new_len=0;

	memset(te, 0, sizeof(textedit_t));
	alloc_more(&te->string, 16, &new_len, &te->alloc_size, sizeof(char), 1.20);		// alloc enough extra space
}

void textedit_add(textedit_t *te, char *str, int32_t cmd, int32_t mod)
{
	int orig_len=0, new_len=0, ins_len, new_pos, char_len;
	char *clipboard;

	if (te==NULL)
		return ;

	if (te->string)
		new_len = orig_len = strlen(te->string);

	if (str)	// insert this string into the text
	{
		ins_len = strlen(str);
		alloc_more(&te->string, ins_len+1, &new_len, &te->alloc_size, sizeof(char), 1.20);		// alloc enough extra space
		memmove(&te->string[te->curpos+ins_len], &te->string[te->curpos], orig_len+1 - te->curpos);	// shift part of the text right of the cursor further right
		memcpy(&te->string[te->curpos], str, ins_len);							// insertion of str
		te->curpos += ins_len;
	}
	#ifdef RL_SDL
	else
	{
		if ((mod & ~KMOD_NUM & ~KMOD_CAPS)==0)	// if there's no modifier key except maybe Num Lock and Caps Lock
		{
			switch (cmd)
			{

				case SDLK_RETURN:
				case SDLK_RETURN2:
				case SDLK_KP_ENTER:
					textedit_add(te, "\n", 0, 0);
					break;

				case SDLK_TAB:
					textedit_add(te, "\t", 0, 0);
					break;

				case SDLK_BACKSPACE:
					new_pos = find_prev_utf8_char(te->string, te->curpos);
					memmove(&te->string[new_pos], &te->string[te->curpos], orig_len+1 - te->curpos);	// shift right side of the text one utf8 character left
					te->curpos = new_pos;
					break;

				case SDLK_DELETE:
					char_len = utf8_char_size(&te->string[te->curpos]);
					memmove(&te->string[te->curpos], &te->string[te->curpos+char_len], orig_len+1 - te->curpos - char_len);
					break;

				case SDLK_LEFT:
					te->curpos = find_prev_utf8_char(te->string, te->curpos);
					break;
				case SDLK_RIGHT:
					te->curpos = find_next_utf8_char(te->string, te->curpos);
					break;

				case SDLK_UP:
					te->cur_vert_mov++;
					break;
				case SDLK_DOWN:
					te->cur_vert_mov--;
					break;

				case SDLK_HOME:
					te->curpos = 0;
					break;

				case SDLK_END:
					te->curpos = strlen(te->string);
					break;
			}
		}
		else if (mod & KMOD_CTRL)
		{
			switch (cmd)
			{
				case SDLK_v:
					clipboard = SDL_GetClipboardText();
					if (clipboard==NULL)
						break;

					textedit_add(te, clipboard, 0, 0);
					SDL_free(clipboard);
					break;

				case SDLK_c:
				case SDLK_x:
					SDL_SetClipboardText(te->string);

					if (cmd==SDLK_x)
					{
						te->string[0] = '\0';
						te->curpos = 0;
					}
					break;

				case SDLK_BACKSPACE:
					te->string[0] = '\0';
					te->curpos = 0;
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

	if (butt_state.once)
	{
		cur_textedit = te;
		te->click = XY0;
		te->click_on = 1;
	}

	intensity *= intensity_scaling(total_scale, 100.);

	draw_string_bestfit_asis(fb, font, te->string, sc_rect(box), 0., 1e30*zc.scrscale, colour, intensity, drawing_thickness, ALIG_LEFT, NULL);
	//draw_string_bestfit(fb, font, name, sc_rect(box), 0., 1e30*zc.scrscale, colour, 1.*intensity, drawing_thickness, ALIG_CENTRE, NULL);
	//draw_rect_chamfer(fb, sc_rect(box), drawing_thickness, colour, blend_add, 0.5*intensity, 1./12.);

	if (butt_state.uponce)
		return 1;
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
