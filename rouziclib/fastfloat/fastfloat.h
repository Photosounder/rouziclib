#ifndef RL_EXCL_APPROX

extern float polynomial_from_lutf(const float *lut, const int lutind, const int order, const float x);
extern double polynomial_from_lut(const double *lut, const int lutind, const int order, const double x);
extern double fastlog2(double x);
extern double fastexp2(double x);
extern double fastpow(double x, double y);
extern double fastsqrt(double x);
extern float fastcosf_d2(float x);
extern double fastcos_d2(double x);
extern double fastcos_d5(double x);
extern float fastwsincf(float x);
extern double fastwsinc(double x);
extern float fast_lsrgbf(float x);
extern float fastgaussianf_d0(float x);
extern float fastgaussianf_d1(float x);
extern float fasterfrf_d0(float x);
extern float fasterfrf_d1(float x);
extern double fastatan2(double y, double x);

#define fastsinf_d2(x) fastcosf_d2((x)-0.5*pi)
#define fastsin_d2(x) fastcos_d2((x)-0.5*pi)
#define fastsin_d5(x) fastcos_d5((x)-0.5*pi)

#else

#define fastlog2	log2
#define fastexp2	exp2
#define fastpow		pow
#define fastsqrt	sqrt
#define fastatan2	atan2
#define fast_lsrgbf	lsrgb

#define fastgaussianf_d0	gaussian
#define fastgaussianf_d1	gaussian
#define fasterfrf_d0		erfr
#define fasterfrf_d1		erfr

#endif
