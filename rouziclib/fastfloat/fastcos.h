#ifndef RL_EXCL_APPROX

extern float fastcosf_tr_d2(float x);
extern double fastcos_tr_d2(double x);
extern double fastcos_tr_d3(double x);
extern double fastcos_tr_d4(double x);
extern double fastcos_tr_d5(double x);

#define fastcosf_d2(x)	fastcosf_tr_d2(x*0.159154943f)
#define fastcos_d2(x)	fastcos_tr_d2(x*0.15915494309189533576888)
#define fastcos_d3(x)	fastcos_tr_d3(x*0.15915494309189533576888)
#define fastcos_d4(x)	fastcos_tr_d4(x*0.15915494309189533576888)
#define fastcos_d5(x)	fastcos_tr_d5(x*0.15915494309189533576888)

#define fastsinf_d2(x) fastcosf_d2((x)-0.5*pi)
#define fastsin_d2(x) fastcos_d2((x)-0.5*pi)
#define fastsin_d5(x) fastcos_d5((x)-0.5*pi)

#else

#define fastcos_d3	cos

#endif
