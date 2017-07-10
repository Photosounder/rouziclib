extern double fastlog2(double x);
extern double fastexp2(double x);
extern double fastpow(double x, double y);
extern double fastsqrt(double x);
extern double fastcos(double x);
extern double fastatan2(double y, double x);
extern float fastgaussianf_d0(float x);
extern float fastgaussianf_d1(float x);
extern float fasterfrf_d0(float x);
extern float fasterfrf_d1(float x);

#define fastsin(x) fastcos((x)-0.5*pi)
