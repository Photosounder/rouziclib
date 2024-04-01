// x = |x| for floats/doubles (makes the sign bit be 0 by binary masking), in-place
#define	ffabsf(x)	(*((uint32_t *) (x)) &= 0x7FFFFFFFUL)
#define	ffabs(x)	(*((uint64_t *) (x)) &= 0x7FFFFFFFFFFFFFFFULL)

#ifdef _MSC_VER
// Replace standard calls due to MSVC retardation (their fabsf calls fabs...)
#define fabsf fast_absf
#define fabs fast_abs
#endif

extern float u32_as_float(const uint32_t i);
extern double u64_as_double(const uint64_t i);
extern uint32_t float_as_u32(const float f);
extern uint64_t double_as_u64(const double f);
extern int float_get_exponent(const float f);
extern int double_get_exponent(const double f);
extern uint32_t float_get_mantissa(const float f);
extern uint64_t double_get_mantissa(const double f);
extern float get_fractional_part_signedf(float f);
extern double get_fractional_part_signed(double f);
extern float get_fractional_part_positivef(float f);
extern double get_fractional_part_positive(double f);
#ifdef RL_INTEL_INTR
extern __m128d _mm_get_fractional_part_positive(__m128d md);
#endif
extern xy_t get_fractional_part_positive_xy(xy_t f);

extern double double_add_ulp(double x, int ulp);
extern int64_t double_diff_ulp(double a, double b);
extern double double_increment_minulp(double v0, double inc);
extern double sign_multiply(double v, double s);
extern float sign_multiplyf(float v, float s);
extern double xor_double(double a, double b);
extern float xor_float(float a, float b);
extern double fast_abs(double x);
extern float fast_absf(float x);
extern uint32_t float_to_fixedpoint_15(float x);
