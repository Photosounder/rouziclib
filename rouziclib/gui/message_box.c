typedef struct
{
	char *message, *title;
	int type, yesno, active, answer, polled, fresh, diag_on;
	uint64_t order;
	flwindow_t window;
	gui_layout_t layout;
} message_box_entry_t;

static message_box_entry_t **message_box_entry=NULL;
static size_t message_box_count=0, message_box_as=0;
static uint64_t message_box_order=0;
static void (*message_box_style)(flwindow_t *w)=NULL;

static message_box_entry_t *message_box_find(const char *message, const char *title, int yesno)
{
	size_t i;

	// Find an active message box with the same kind and text
	for (i=0; i < message_box_count; i++)
		if (message_box_entry[i]->active && message_box_entry[i]->yesno == yesno)
		if (strcmp(message_box_entry[i]->message, message)==0 && strcmp(message_box_entry[i]->title, title)==0)
			return message_box_entry[i];

	return NULL;
}

static message_box_entry_t *message_box_add(const char *message, const char *title, int type, int yesno)
{
	size_t i;
	message_box_entry_t *e;

	// Reuse an inactive entry, entries are never freed so their layouts stay valid for the layout registry
	for (i=0; i < message_box_count; i++)
		if (message_box_entry[i]->active == 0)
			break;

	// Otherwise add a new entry
	if (i == message_box_count)
	{
		alloc_enough((void **) &message_box_entry, message_box_count+1, &message_box_as, sizeof(message_box_entry_t *), 2.);
		message_box_entry[i] = calloc(1, sizeof(message_box_entry_t));
		message_box_count++;
	}

	// Fill it in as a new pending message
	e = message_box_entry[i];
	free(e->message);
	free(e->title);
	e->message = make_string_copy(message);
	e->title = make_string_copy(title);
	e->type = type;
	e->yesno = yesno;
	e->answer = -1;
	e->polled = 1;
	e->fresh = 1;
	e->diag_on = 1;
	e->order = ++message_box_order;
	memset(&e->window, 0, sizeof(flwindow_t));
	e->active = 1;

	// Also log it, which helps when nothing is displayed
	fprintf_rl(stderr, "%s: %s\n", title, message);

	return e;
}

void message_box(const char *message, const char *title, int type)
{
	// Treat missing strings as empty
	if (message==NULL) message = "";
	if (title==NULL) title = "";

	// Queue the message unless the same one is already waiting to be read
	if (message_box_find(message, title, 0))
		return ;

	message_box_add(message, title, type, 0);
}

int message_box_yesno(const char *message, const char *title)
{
	int answer;
	message_box_entry_t *e;

	// Treat missing strings as empty
	if (message==NULL) message = "";
	if (title==NULL) title = "";

	// Open the question the first time it's asked
	e = message_box_find(message, title, 1);
	if (e == NULL)
	{
		message_box_add(message, title, 1, 1);
		return -1;
	}

	// Keep the question up while the caller is still waiting for it
	e->polled = 1;
	if (e->answer == -1)
		return -1;

	// Hand the answer over once and retire the question
	answer = e->answer;
	e->active = 0;
	return answer;
}

void message_box_dialog(message_box_entry_t *e)
{
	static const char *layout_src[] = {
		"elem 0", "type none", "label Message", "pos	0	0", "dim	8	3;6", "off	0;6	1", "",
		"elem 10", "type label", "pos	3;6	-0;10", "dim	6	1;6", "off	1	1", "",
		"elem 11", "type label", "pos	-3;6	-1;7", "dim	0;9	0;9", "off	0	0;6", "",
		"elem 20", "type button", "label OK", "pos	0	-3;2", "dim	1;7	0;6", "off	0;6	0", "",
		"elem 30", "type button", "label Yes", "pos	-1	-3;2", "dim	1;7	0;6", "off	0;6	0", "",
		"elem 31", "type button", "label No", "pos	1	-3;2", "dim	1;7	0;6", "off	0;6	0", "",
	};

	// Absorb clicks meant for the windows below so the message box is modal, its own controls come later and take precedence
	ctrl_button_invis(make_rect_off(XY0, set_xy(1e30), set_xy(0.5)), NULL);

	// Place a new message box near the top centre and give it its text
	gui_layout_init_pos_scale(&e->layout, XY0, 4./3., xy(0., 3.), e->fresh);
	make_gui_layout(&e->layout, layout_src, sizeof(layout_src)/sizeof(char *), "Message box");
	gui_printf_to_label(&e->layout, 0, 0, "%s", e->title);
	gui_printf_to_label(&e->layout, 10, 0, "%s", e->message);
	e->fresh = 0;

	// Style a new window like the app's other dialogs
	if (e->window.init == 0)
	{
		flwindow_init_defaults(&e->window);
		e->window.hide_pin = 1;
		if (message_box_style)
			message_box_style(&e->window);
	}

	// Only a plain message gets a close button, a question must be answered
	draw_dialog_window_fromlayout(&e->window, e->yesno ? NULL : &e->diag_on, NULL, &e->layout, 0);

	// Draw the icon left of the text like a native message box, a red cross for an error (type 0) and a yellow warning sign otherwise
	rect_t icon_box = sc_rect(gui_layout_elem_comp_area_os(&e->layout, 11, XY0));
	if (e->type == 0)
		draw_string_bestfit(font, "\342\235\214", icon_box, 0., 1e30*zc.scrscale, make_colour_hsl(0., 1., 0.2, HUEDEG, 0), 1., drawing_thickness, ALIG_CENTRE, NULL);
	else
		draw_string_bestfit(font, "\342\232\240", icon_box, 0., 1e30*zc.scrscale, make_colour_hsl(60., 1., 0.4, HUEDEG, 0), 1., drawing_thickness, ALIG_CENTRE, NULL);

	// Fit the message left-aligned keeping its line breaks, a single line taking at most a quarter of the area's height
	rect_t text_box = sc_rect(gui_layout_elem_comp_area_os(&e->layout, 10, XY0));
	draw_string_bestfit_multiline(font, e->message, text_box, 0., 0.25*fabs(get_rect_dim(text_box).y), e->layout.elem[10]->colour, 1., drawing_thickness, ALIG_LEFT, NULL);

	if (e->yesno)
	{
		// Record the answer for message_box_yesno() to collect
		if (ctrl_button_fromlayout(&e->layout, 30))
			e->answer = 1;
		if (ctrl_button_fromlayout(&e->layout, 31))
			e->answer = 0;
	}
	else
	{
		// Dismiss the message with OK or the close button
		if (ctrl_button_fromlayout(&e->layout, 20) || e->diag_on == 0)
			e->active = 0;
	}
}

int message_box_dialogs(void (*style_func)(flwindow_t *w))
{
	size_t i;
	message_box_entry_t *e, *first=NULL;

	message_box_style = style_func;

	for (i=0; i < message_box_count; i++)
	{
		e = message_box_entry[i];

		// Retire questions that their caller stopped asking about since the last frame
		if (e->active && e->yesno)
		{
			if (e->polled == 0)
				e->active = 0;
			e->polled = 0;
		}

		// Find the oldest message box still waiting for the user
		if (e->active && e->answer == -1)
			if (first == NULL || e->order < first->order)
				first = e;
	}

	if (first == NULL)
		return 0;

	// Show one message box at a time on top of the other windows
	window_register(1, message_box_dialog, first, RECTNAN, NULL, 1, first);
	return 1;
}
