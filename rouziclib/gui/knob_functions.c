double knobf_linear(double v, double min, double max, double *arg, const int mode)
{
	if (mode==0)
		return v*(max-min) + min;	// [0 , 1] to value
	else
		return (v-min) / (max-min);	// value to [0 , 1]
}

double knobf_log(double v, double min, double max, double *arg, const int mode)
{
	if (mode==0)
		return exp(knobf_linear(v, log(min), log(max), NULL, mode));
	else
		return knobf_linear(log(v), log(min), log(max), NULL, mode);
}

double knobf_recip(double v, double min, double max, double *arg, const int mode)
{
	if (mode==0)
		return 1. / knobf_linear(v, 1./min, 1./max, NULL, mode);
	else
		return knobf_linear(1./v, 1./min, 1./max, NULL, mode);
}

double knobf_dboff(double v, double min, double max, double *arg, const int mode)
{
	double maxoff = vol_to_db(db_to_vol(max) + db_to_vol(min));

	if (mode==0)
		return vol_to_db(db_to_vol(knobf_linear(v, min, maxoff, NULL, mode)) - db_to_vol(min));	// [0 , 1] -> [min dB , maxoff dB] -> [-inf dB , max dB]
	else
		return knobf_linear(vol_to_db(db_to_vol(v) + db_to_vol(min)), min, maxoff, NULL, mode);	// [-inf dB , max dB] -> [min dB , maxoff dB] -> [0 , 1]
}

double knobf_logoff(double v, double min, double max, double *arg, const int mode)
{
	double off = arg[0];

	if (mode==0)
		return exp(knobf_linear(v, log(min+off), log(max+off), NULL, mode)) - off;
	else
		return knobf_linear(log(v+off), log(min+off), log(max+off), NULL, mode);
}

const char *knob_func_name[] =
{
	"linear",
	"log",
	"recip",
	"dboff",
	"logoff",
};

const knob_func_t knob_func_array[] =
{
	knobf_linear,
	knobf_log,
	knobf_recip,
	knobf_dboff,
	knobf_logoff,
};

const int knob_func_count = sizeof(knob_func_name)/sizeof(*knob_func_name);

knob_func_t knob_func_name_to_ptr(const char *name)
{
	int i;

	if (name==NULL)	return NULL;
	if (name[0]=='\0') return NULL;

	for (i=0; i < knob_func_count; i++)
		if (strcmp(name, knob_func_name[i])==0)
			return knob_func_array[i];

	return NULL;
}

const char *knob_func_ptr_to_name(knob_func_t fp)
{
	int i;

	for (i=0; i < knob_func_count; i++)
		if (fp == knob_func_array[i])
			return knob_func_name[i];

	return NULL;
}
