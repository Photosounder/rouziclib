typedef double (*knob_func_t)(double, double, double, const int);

typedef struct
{
	char *main_label;
	knob_func_t func;
	double min, max, default_value;
	int valfmt;
} knob_t;
