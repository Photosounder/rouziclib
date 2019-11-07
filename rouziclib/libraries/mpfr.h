#ifdef RL_MPFR

#ifdef _MSC_VER
#pragma comment (lib, "mpfr.lib")	// check if correct
#endif

#include <gmp.h>
#include <mpfr.h>
#include <mpf2mpfr.h>

#define real	mpfr_t

#define r_init(x)	mpfr_init_set_d(x, 0., MPFR_RNDN)
#define r_free(x)	mpfr_clear(x)
#define r_set(x,y)	mpfr_set(x, y, MPFR_RNDN)
#define r_setd(x,y)	mpfr_set_d(x, y, MPFR_RNDN)
#define r_abs(x)	mpfr_abs(x, x, MPFR_RNDN)
#define r_add(x,y)	mpfr_add(x, x, y, MPFR_RNDN)
#define r_addd(x,y)	mpfr_add_d(x, x, y, MPFR_RNDN)
#define r_radd(r,x,y)	mpfr_add(r, x, y, MPFR_RNDN)
#define r_sub(x,y)	mpfr_sub(x, x, y, MPFR_RNDN)
#define r_rsub(r,x,y)	mpfr_sub(r, x, y, MPFR_RNDN)
#define r_subd(x,y)	mpfr_sub_d(x, x, y, MPFR_RNDN)
#define r_neg(x)	mpfr_neg(x, x, MPFR_RNDN)
#define r_rneg(r,x)	mpfr_neg(r, x, MPFR_RNDN)
#define r_mul(x,y)	mpfr_mul(x, x, y, MPFR_RNDN)
#define r_rmul(r,x,y)	mpfr_mul(r, x, y, MPFR_RNDN)
#define r_muld(x,y)	mpfr_mul_d(x, x, y, MPFR_RNDN)
#define r_rmuld(r,x,y)	mpfr_mul_d(r, x, y, MPFR_RNDN)
#define r_muli(x,y)	mpfr_mul_si(x, x, y, MPFR_RNDN)
#define r_div(x,y)	mpfr_div(x, x, y, MPFR_RNDN)
#define r_rdiv(r,x,y)	mpfr_div(r, x, y, MPFR_RNDN)
#define r_fma(x,y,z)	mpfr_fma(x, x, y, z, MPFR_RNDN)
#define r_rfma(x,y,z)	mpfr_rfma(x, x, y, z, MPFR_RNDN)
#define r_pow(x,y)	mpfr_pow(x, x, y, MPFR_RNDN)
#define r_rpow(r,x,y)	mpfr_pow(r, x, y, MPFR_RNDN)
#define r_powi(x,y)	mpfr_pow_si(x, x, y, MPFR_RNDN)
#define r_rpowi(r,x,y)	mpfr_pow_si(r, x, y, MPFR_RNDN)
#define r_todouble(x)	mpfr_get_d(x, MPFR_RNDN)
#define r_pi(x)		mpfr_const_pi(x, MPFR_RNDN)
#define r_cos(r,x)	mpfr_cos(r, x, MPFR_RNDN)
#define r_sin(r,x)	mpfr_sin(r, x, MPFR_RNDN)
#define r_sq(r,x)	mpfr_sqr(r, x, MPFR_RNDN)
#define r_sqrt(r,x)	mpfr_sqrt(r, x, MPFR_RNDN)
#define r_exp(r,x)	mpfr_exp(r, x, MPFR_RNDN)
#define r_log(r,x)	mpfr_log(r, x, MPFR_RNDN)
#define r_log2(r,x)	mpfr_log2(r, x, MPFR_RNDN)
#define r_log10(r,x)	mpfr_log10(r, x, MPFR_RNDN)
#define r_exp10(r,x)	mpfr_exp10(r, x, MPFR_RNDN)

extern void r_flipsign(real y, real x);
extern void r_gaussian(real y, real x);

#endif
