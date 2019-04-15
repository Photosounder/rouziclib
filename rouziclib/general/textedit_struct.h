typedef struct
{
	char *string;
	int alloc_size, curpos;
	uint32_t timestamp;
} textundostate_t;

typedef struct
{
	textundostate_t *state;
	int alloc_count, state_count, cur_state_index, latest_is_saved;
	uint32_t timestamp;
} textundo_t;

typedef struct
{
	char *string;
	int alloc_size, curpos, curpos_up, curpos_down, sel0, sel1;
	xy_t click;
	xy_t cur_screen_pos, cur_screen_pos_prev;
	int click_on;
	double max_scale, rect_brightness;
	textundo_t undo;
	int edit_mode, read_only, first_click_no_sel, return_flag, draw_string_mode, was_cur_te, tab_switch, sel_all;
} textedit_t;
