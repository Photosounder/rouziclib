polynomial_signal_t sample_signal_to_polynomial_signal(void *sample_signal, size_t sample_count, double (*sampling_func)(void*,size_t), double sample_rate, double max_freq, double sinc_freq, double rolloff_bandwidth, const int analytic)
{
	polynomial_signal_t ps = {0};

	// Each chunk is one sample long
	double sample_dur = 1. / sample_rate;
	ps.chunk_dur = sample_dur;

	// Calculate windowed sinc parameters
	double sinc_dur = 9. / rolloff_bandwidth;	// the 9 ratio depends on the frequency rolloff characteristics of the window
	int sinc_chunk_count = ceil(sinc_dur / ps.chunk_dur * 0.5) * 2. + 1.;
	sinc_dur = (double) sinc_chunk_count * ps.chunk_dur;
	double sinc_start = -0.5 * sinc_dur;

	// Padding to make room for what convolution adds at each end
	double pad = (double) sinc_chunk_count * 0.5 * ps.chunk_dur;

	// Set all parameters
	ps.orig_start_time = 0.;
	ps.orig_end_time = (double) sample_count * sample_dur;
	ps.start_time = ps.orig_start_time - pad;
	ps.end_time = ps.orig_end_time + pad;
	ps.max_freq = MINN(max_freq, sinc_freq + 0.5*rolloff_bandwidth);
	ps.chunk_count = nearbyint((ps.end_time - ps.start_time) / ps.chunk_dur);
	ps.node_count = 10;		// degree 9 gives 10 coefs per chunk and a worst precision of 5.3e-8 with nodes at ends
	ps.coef_real = calloc(ps.chunk_count * ps.node_count, sizeof(float));
	if (analytic)
		ps.coef_imag = calloc(ps.chunk_count * ps.node_count, sizeof(float));

	// Precalculate node offsets
	double *node = calloc(ps.node_count, sizeof(double));
	double end_node_scale = 1. / chebyshev_node(ps.node_count, 0.);
	for (int i=0; i < ps.node_count; i++)
		node[i] = 0.5 + 0.5 * (chebyshev_node(ps.node_count, i) * end_node_scale);

	// Precalculate windowed sinc irregular samples
	double *sinc_im = NULL;
	double *sinc_re = calloc(sinc_chunk_count * ps.node_count, sizeof(double));
	if (analytic)
		sinc_im = calloc(sinc_chunk_count * ps.node_count, sizeof(double));

	for (int ic=0; ic < sinc_chunk_count; ic++)
	{
		double t_chunk = sinc_start + (double) ic * ps.chunk_dur;

		for (int i=0; i < ps.node_count; i++)
		{
			double t = t_chunk + node[i] * ps.chunk_dur;

			double w = short_gaussian_window(t, sinc_dur*0.5, 2.5);
			sinc_re[ic*ps.node_count + i] = w * sinc(t, sinc_freq);
			if (analytic)
				sinc_im[ic*ps.node_count + i] = w * sinc_hilbert(t, sinc_freq);
		}
	}

	// Convolution
	double *conv_im = NULL;
	double *conv_re = calloc(ps.chunk_count * ps.node_count, sizeof(double));
	if (analytic)
		conv_im = calloc(ps.chunk_count * ps.node_count, sizeof(double));

	for (size_t is=0; is < sample_count; is++)
	{
		double s = sampling_func(sample_signal, is);

		int sinc_sample_count = sinc_chunk_count * ps.node_count;
		size_t ism = is * ps.node_count;

		for (size_t i=0; i < sinc_sample_count; i++)
			conv_re[ism + i] += sinc_re[i] * s;

		if (analytic)
			for (size_t i=0; i < sinc_sample_count; i++)
				conv_im[ism + i] += sinc_im[i] * s;
	}

	// Precalculate cosines for each degree
	double *cos_coef = calloc(ps.node_count * ps.node_count, sizeof(double));
	double scale = 1. / (double) ps.node_count;
	for (int id=0; id < ps.node_count; id++)
	{
		double x;
		int i;
		for (x=0.5, i=0; i < ps.node_count; i++, x+=1.)
			cos_coef[id*ps.node_count + i] = cos_tr(x * (double) id * 0.5 / (double) ps.node_count) * scale;

		if (id == 0)
			scale *= 2.;
	}

	// Chebyshev polynomial analysis
	for (int ic=0; ic < ps.chunk_count; ic++)
	{
		for (int id=0; id < ps.node_count; id++)
		{
			double sum = 0.;
			for (int i=0; i < ps.node_count; i++)
				sum += conv_re[ic*ps.node_count + i] * cos_coef[id*ps.node_count + i];
			ps.coef_real[ic*ps.node_count + id] = sum;
		}

		if (analytic)
		for (int id=0; id < ps.node_count; id++)
		{
			double sum = 0.;
			for (int i=0; i < ps.node_count; i++)
				sum += conv_im[ic*ps.node_count + i] * cos_coef[id*ps.node_count + i];
			ps.coef_imag[ic*ps.node_count + id] = sum;
		}
	}

	free(cos_coef);
	free(conv_im);
	free(conv_re);
	free(sinc_im);
	free(sinc_re);
	free(node);
	return ps;
}

void polynomial_signal_eval(polynomial_signal_t *ps, double t_start, double t_step, int output_count, double *outd, float *outf, const int analytic)
{
	ssize_t ic = 0, is = 0;
	double t = t_start;
	double chunk_rate = 1. / ps->chunk_dur;
	int degree = ps->node_count - 1;

	// Find starting chunk
	ic = floor((t_start - ps->start_time) / ps->chunk_dur);

	if (ic >= ps->chunk_count)
		return;

	// If it starts out of bounds skip ahead
	if (ic < 0)
	{
		is = ceil((ps->start_time - t_start) / t_step);		// set the index of the first output sample
		ic = 0;
	}

	double end_node_scale = chebyshev_node(ps->node_count, 0.);

	// Evaluate every output sample
	for ( ; t < ps->end_time && is < output_count; is++)
	{
		t = (double) is * t_step + t_start;				// absolute time of sample is
		ic = floor((t - ps->start_time) * chunk_rate);			// chunk at time t

		double tc0 = (double) ic * ps->chunk_dur + ps->start_time;	 // chunk start time
		double tc = ((t - tc0) * chunk_rate * 2. - 1.) * end_node_scale; // time relative to the chunk in [-1 , 1]

		// Evaluation
		double v_re = eval_chebyshev_polynomial_float_coefs(tc, &ps->coef_real[ic*ps->node_count], degree);

		if (analytic)
		{
			double v_im = eval_chebyshev_polynomial_float_coefs(tc, &ps->coef_imag[ic*ps->node_count], degree);

			if (outd)
			{
				outd[(is<<1)    ] += v_re;
				outd[(is<<1) + 1] += v_im;
			}

			if (outf)
			{
				outf[(is<<1)    ] += v_re;
				outf[(is<<1) + 1] += v_im;
			}
		}
		else
		{
			if (outd)
				outd[is] += v_re;
			if (outf)
				outf[is] += v_re;
		}
	}
}

double sampling_func_float_mono(void *ptr, size_t index)
{
	float *s = (float *) ptr;
	return (double) s[index];
}
