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
