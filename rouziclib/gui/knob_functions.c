double knobf_linear(double v, double min, double max, const int mode)
{
	if (mode==0)
		return v*(max-min) + min;	// [0 , 1] to value
	else
		return (v-min) / (max-min);	// value to [0 , 1]
}

double knobf_log(double v, double min, double max, const int mode)
{
	if (mode==0)
		return exp(knobf_linear(v, log(min), log(max), mode));
	else
		return knobf_linear(log(v), log(min), log(max), mode);
}

double knobf_recip(double v, double min, double max, const int mode)
{
	if (mode==0)
		return 1. / knobf_linear(v, 1./min, 1./max, mode);
	else
		return knobf_linear(1./v, 1./min, 1./max, mode);
}

const char *knob_func_name[] =
{
	"linear",
	"log",
	"recip",
};

const knob_func_t knob_func_array[] =
{
	knobf_linear,
	knobf_log,
	knobf_recip,
};

knob_func_t knob_func_name_to_ptr(const char *name)
{
	int i;

	if (name==NULL)	return NULL;
	if (name[0]=='\0') return NULL;

	for (i=0; i < sizeof(knob_func_name)/sizeof(char *); i++)
		if (strcmp(name, knob_func_name[i])==0)
			return knob_func_array[i];

	return NULL;
}

const char *knob_func_ptr_to_name(knob_func_t fp)
{
	int i;

	for (i=0; i < sizeof(knob_func_name)/sizeof(char *); i++)
		if (fp == knob_func_array[i])
			return knob_func_name[i];

	return NULL;
}
