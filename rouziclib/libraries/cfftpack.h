#ifndef RL_EXCL_CFFTPACK

#ifndef fft_real_t
#define fft_real_t double
#endif

typedef struct {
  fft_real_t *save, *work;
  int lensav, lenwork, scale;
  xyi_t dim;
} cfft_plan_t;

extern void cfft_plan_free(cfft_plan_t *plan);
extern cfft_plan_t cfft_2D_create_plan(xyi_t dim);
extern int cfft_2D(cfft_plan_t *plan, fft_real_t *data, const int way);
extern void cfft_complex_mul_2D(void *va, void *vb, void *vr, xyi_t dim);
extern void cfft_copy_r2c_pad(fft_real_t *in, void **pout, size_t *out_as, xyi_t in_dim, xyi_t out_dim);
extern void cfft_copy_c2r(fft_real_t *in, fft_real_t *out, xyi_t dim);
extern void cfft_r2c_padded_fft(cfft_plan_t *plan, fft_real_t *in, void **pout, size_t *out_as, xyi_t in_dim, xyi_t out_dim);
extern void cfft_2D_c2r_full_ifft(cfft_plan_t *plan, fft_real_t *in, fft_real_t *out, xyi_t dim);

extern void cfft_test();

#endif
