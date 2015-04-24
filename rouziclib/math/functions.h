#define pi 3.14159265358979323846264338327950288
#define e_ 2.7182818284590452353602874713526625

#define sign(x)		(((x) > 0) - ((x) < 0))
#define positive(x)	((x) > 0 ? (x) : 0)
#define MAXN(x, y)	(((x) > (y)) ? (x) : (y))
#define MINN(x, y)	(((x) < (y)) ? (x) : (y))
#define	ffabs(x)	(*((uint64_t *) (x)) &= 0x7FFFFFFFFFFFFFFF)		// x = |x| for doubles (makes the sign bit be 0 by binary masking)

#ifdef _WIN32
#define copysign(x, y)	( (((x) < 0 && (y) > 0) || ((x) > 0 && (y) < 0)) ? -(x) : (x) )
#define nearbyint(x)	(((x)>=0.) ? (int32_t) ((x)+0.5) : (int32_t) ((x)-0.5))
#endif

extern int32_t fphypot(int32_t x, int32_t y);
extern double distance_xy(double dx, double dy);
extern double gaussian(double x);
extern double erf(double x);

extern double roundaway(double x);
extern double rangewrap(double x, double low, double high);
extern double rangelimit(double x, double min, double max);
