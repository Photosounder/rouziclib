#define RL_PI 3.14159265358979323846264338327950288
#define pi RL_PI
#define e_ 2.7182818284590452353602874713526625

#define sign(x)		(((x) > 0) - ((x) < 0))
#define positive(x)	((x) > 0 ? (x) : 0)
#define MAXN(x, y)	(((x) > (y)) ? (x) : (y))
#define MINN(x, y)	(((x) < (y)) ? (x) : (y))

extern int32_t fphypot(int32_t x, int32_t y);
extern double sq(double x);
extern float sqf(float x);
extern double gaussian(double x);
extern double erfr(double x);
extern double gamma_dist(double x, double a, double b);

extern double roundaway(double x);
extern double rangewrap(double x, double low, double high);
extern double rangelimit(double x, double min, double max);
extern float rangelimitf(float x, float min, float max);
extern int32_t rangelimit_i32(int32_t x, int32_t min, int32_t max);
extern void swap_double(double *a, double *b);
extern void swap_i32(int32_t *a, int32_t *b);
extern void minmax_double(double *a, double *b);
extern void minmax_i32(int32_t *a, int32_t *b);
extern double normalised_notation_split(double number, double *m);
extern double fabs_min(double a, double b);
extern double fabs_max(double a, double b);
extern int ceil_rshift(int v, int sh);
extern int idiv_ceil(int a, int b);
extern int find_largest_prime_factor(int n);
extern int is_prime(int n);
extern int next_prime(int n);
extern int64_t next_power_of_2(int64_t n);
extern int modulo_euclidian(int a, int b);
