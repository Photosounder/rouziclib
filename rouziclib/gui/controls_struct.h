typedef double (*knob_func_t)(double, double, double, const int);

typedef struct
{
	char *main_label, *fmt_str;
	knob_func_t func;
	double min, max, default_value;
	textedit_t edit;
	int edit_open, circular;
} knob_t;
