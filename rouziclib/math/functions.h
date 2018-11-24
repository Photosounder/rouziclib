#define pi 3.14159265358979323846264338327950288
#define e_ 2.7182818284590452353602874713526625

#define sign(x)		(((x) > 0) - ((x) < 0))
#define positive(x)	((x) > 0 ? (x) : 0)
#define MAXN(x, y)	(((x) > (y)) ? (x) : (y))
#define MINN(x, y)	(((x) < (y)) ? (x) : (y))
#define	ffabs(x)	(*((uint64_t *) (x)) &= 0x7FFFFFFFFFFFFFFF)		// x = |x| for doubles (makes the sign bit be 0 by binary masking), in-place
#define get_bit(word, pos)	(((word) >> (pos)) & 1)

#ifdef _WIN32
//#define copysign(x, y)	( (((x) < 0 && (y) > 0) || ((x) > 0 && (y) < 0)) ? -(x) : (x) )
//#define nearbyint(x)	(((x)>=0.) ? (int32_t) ((x)+0.5) : (int32_t) ((x)-0.5))
#endif

extern int32_t fphypot(int32_t x, int32_t y);
extern double sq(double x);
extern float sqf(float x);
extern double gaussian(double x);
extern double sinc(double x, double fc);
extern double blackman(double x, double range);
extern double erfr(double x);

extern double roundaway(double x);
extern double rangewrap(double x, double low, double high);
extern double rangelimit(double x, double min, double max);
extern float rangelimitf(float x, float min, float max);
extern int32_t rangelimit_i32(int32_t x, int32_t min, int32_t max);
extern void swap_double(double *a, double *b);
extern void swap_i32(int32_t *a, int32_t *b);
extern void minmax_double(double *a, double *b);
extern void minmax_i32(int32_t *a, int32_t *b);
extern double double_add_ulp(double x, int ulp);
extern int64_t double_diff_ulp(double a, double b);
extern double double_increment_minulp(const double v0, const double inc);
extern double normalised_notation_split(double number, double *m);
extern double fabs_min(double a, double b);
extern double fabs_max(double a, double b);
extern int get_bit_32(const uint32_t word, const int pos);
extern int find_largest_prime_factor(int n);
extern int ceil_rshift(int v, int sh);
extern float u32_as_float(uint32_t i);
extern double u64_as_double(uint64_t i);
extern uint32_t float_as_u32(float f);
extern uint64_t double_as_u64(double f);
