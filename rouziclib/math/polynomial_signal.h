typedef struct
{
	double start_time, end_time, orig_start_time, orig_end_time, chunk_dur, max_freq;
	size_t chunk_count;
	int node_count, bits_per_chunk;
	int8_t *degree_bits;
	double *degree_mul;
	uint8_t *coef_real, *coef_imag;
	double *coef_buffer;
} polynomial_signal_t;

extern void polynomial_signal_free(polynomial_signal_t *ps);
extern polynomial_signal_t sample_signal_to_polynomial_signal(void *sample_signal, size_t sample_count, double (*sampling_func)(void*,size_t), double sample_rate, double max_freq, double sinc_freq, double rolloff_bandwidth, const int analytic, int degree, double quant_error);
extern polynomial_signal_t polynomial_signal_to_polynomial_signal(polynomial_signal_t *ps_in, int degree, double chunk_dur, double quant_error);
extern void polynomial_signal_eval(polynomial_signal_t *ps, double t_start, double t_step, int output_count, double *outd, float *outf, const int analytic);

extern double sampling_func_float_mono(void *ptr, size_t index);
