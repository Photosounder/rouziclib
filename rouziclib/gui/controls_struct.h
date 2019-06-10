typedef double (*knob_func_t)(double, double, double, const int);

typedef struct
{
	char *main_label, *fmt_str;
	knob_func_t func;
	double min, max, default_value;
	textedit_t edit;
	int edit_open, circular;
} knob_t;

typedef struct
{
	int init, pinned, hide_corner, hide_pin;
	ctrl_drag_state_t bar_drag, corner_drag;
	col_t bg_col, bar_col, close_hover_col, close_down_col, title_col, close_x_col;
	double bar_height, shadow_strength;
	xy_t pinned_offset;
	double pinned_sm;
} flwindow_t;
