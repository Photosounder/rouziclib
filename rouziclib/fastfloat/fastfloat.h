#ifndef RL_EXCL_APPROX

extern double fastlog2(double x);
extern double fastexp2(double x);
extern double fastpow(double x, double y);
extern double fastsqrt(double x);
extern float fastcosf_d2(float x);
extern double fastcos_d2(double x);
extern double fastcos_d5(double x);
extern double fastatan2(double y, double x);
extern float fastgaussianf_d0(float x);
extern float fastgaussianf_d1(float x);
extern float fasterfrf_d0(float x);
extern float fasterfrf_d1(float x);

#define fastsinf_d2(x) fastcosf_d2((x)-0.5*pi)
#define fastsin_d2(x) fastcos_d2((x)-0.5*pi)
#define fastsin_d5(x) fastcos_d5((x)-0.5*pi)

#else

#define fastlog2	log2
#define fastexp2	exp2
#define fastpow		pow
#define fastsqrt	sqrt
#define fastcos		cos
#define	fastsin		sin
#define fastatan2	atan2

#define fastgaussianf_d0	gaussian
#define fastgaussianf_d1	gaussian
#define fasterfrf_d0		erfr
#define fasterfrf_d1		erfr

#endif
