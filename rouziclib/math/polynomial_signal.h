typedef struct
{
	double start_time, end_time, orig_start_time, orig_end_time, chunk_dur, max_freq;
	size_t chunk_count;
	int node_count;
	float *coef_real, *coef_imag;
} polynomial_signal_t;

extern polynomial_signal_t sample_signal_to_polynomial_signal(void *sample_signal, size_t sample_count, double (*sampling_func)(void*,size_t), double sample_rate, double max_freq, double sinc_freq, double rolloff_bandwidth, const int analytic);
extern void polynomial_signal_eval(polynomial_signal_t *ps, double t_start, double t_step, int output_count, double *outd, float *outf, const int analytic);

extern double sampling_func_float_mono(void *ptr, size_t index);
