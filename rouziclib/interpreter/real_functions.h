// Double
static void real_d_set(double *r, double *a) { *r = *a; }
static double real_d_cvt_r_d(double *a) { return *a; }
static void real_d_cvt_d_r(double *r, double a) { *r = a; };
static int64_t real_d_cvt_r_i(double *a) { return (int64_t) *a; }
static void real_d_cvt_i_r(double *r, int64_t a) { *r = (double) a; }
static void real_d_ator(double *r, const char *string, char **endptr) { *r = strtod(string, endptr); }

static void real_d_add(double *r, double *a, double *b) { *r = *a + *b; }
static void real_d_sub(double *r, double *a, double *b) { *r = *a - *b; }
static void real_d_mul(double *r, double *a, double *b) { *r = *a * *b; }
static void real_d_div(double *r, double *a, double *b) { *r = *a / *b; }

static void real_d_nan(double *r) { *r = NAN; }
static void real_d_pi(double *r) { *r = RL_PI; }
static void real_d_e(double *r) { *r = 2.7182818284590451; }

static void real_d_abs(double *r, double *a) { *r = fabs(*a); }
static void real_d_sign(double *r, double *a) { if (*a < 0.) *r = -1.; else *r = (*a > 0.); }
static void real_d_nearbyint(double *r, double *a) { *r = nearbyint(*a); }
static void real_d_floor(double *r, double *a) { *r = floor(*a); }
static void real_d_ceil(double *r, double *a) { *r = ceil(*a); }
static void real_d_trunc(double *r, double *a) { *r = trunc(*a); }
static void real_d_clamp(double *r, double *a) { *r = rangelimit(*a, 0., 1.); }
static void real_d_cos(double *r, double *a) { *r = cos(*a); }
static void real_d_sin(double *r, double *a) { *r = sin(*a); }

static void real_d_min(double *r, double *a, double *b) { *r = MINN(*a, *b); }
static void real_d_max(double *r, double *a, double *b) { *r = MAXN(*a, *b); }
static void real_d_pow(double *r, double *a, double *b) { *r = pow(*a, *b); }

static void real_d_rangelimit(double *r, double *a, double *b, double *c) { *r = rangelimit(*a, *b, *c); }

static rlip_real_functions_t real_d_functions = {
	sizeof(double),
	(void (*)(uint8_t *,uint8_t *))			real_d_set,
	(double (*)(uint8_t *))				real_d_cvt_r_d,
	(void (*)(uint8_t *,double))			real_d_cvt_d_r,
	(int64_t (*)(uint8_t *))			real_d_cvt_r_i,
	(void (*)(uint8_t *,int64_t))			real_d_cvt_i_r,
	(int (*)(const uint8_t *,const uint8_t *))	cmp_double,
	(void (*)(uint8_t *,const char *,char **))	real_d_ator,
	(void (*)(uint8_t *))				NULL,
	(void (*)(uint8_t *))				NULL,
};

// Add these defaults to your inputs by doing rlip_inputs_t inputs[] = { RLIP_REAL_DOUBLE, RLIP_FUNC, {"x", &x, "pr"}, ... };
#define RLIP_REAL_DOUBLE				\
	{"rlip_real_functions", &real_d_functions, ""},	\
	{"add_", real_d_add,	"frrr"},		\
	{"sub_", real_d_sub,	"frrr"},		\
	{"mul_", real_d_mul,	"frrr"},		\
	{"div_", real_d_div,	"frrr"},		\
	{"nan_", real_d_nan,	"fr"},			\
	{"pi_", real_d_pi,	"fr"},			\
	{"e_", real_d_e,	"fr"},			\
	{"abs_", real_d_abs,	"frr"},			\
	{"sign_", real_d_sign,	"frr"},			\
	{"nearbyint_", real_d_nearbyint, "frr"},	\
	{"floor_", real_d_floor, "frr"},		\
	{"ceil_", real_d_ceil,	"frr"},			\
	{"trunc_", real_d_trunc, "frr"},		\
	{"clamp_", real_d_clamp, "frr"},		\
	{"cos_", real_d_cos,	"frr"},			\
	{"sin_", real_d_sin,	"frr"},			\
	{"min_", real_d_min,	"frrr"},		\
	{"max_", real_d_max,	"frrr"},		\
	{"pow_", real_d_pow,	"frrr"},		\
	{"rangelimit_", rangelimit, "frrrr"}


// Double-double
static void real_q_set(ddouble_t *r, ddouble_t *a) { *r = *a; }
static double real_q_cvt_r_d(ddouble_t *a) { return a->hi; }
static void real_q_cvt_d_r(ddouble_t *r, double a) { *r = ddouble(a); };
static int64_t real_q_cvt_r_i(ddouble_t *a) { return (int64_t) a->hi; }
static void real_q_cvt_i_r(ddouble_t *r, int64_t a) { *r = ddouble((double) a); }
static void real_q_ator(ddouble_t *r, const char *string, char **endptr) { *r = string_to_ddouble(string, endptr); }

static void real_q_add(ddouble_t *r, ddouble_t *a, ddouble_t *b) { *r = add_qq(*a, *b); }
static void real_q_sub(ddouble_t *r, ddouble_t *a, ddouble_t *b) { *r = sub_qq(*a, *b); }
static void real_q_mul(ddouble_t *r, ddouble_t *a, ddouble_t *b) { *r = mul_qq(*a, *b); }
static void real_q_div(ddouble_t *r, ddouble_t *a, ddouble_t *b) { *r = div_qq(*a, *b); }

static void real_q_nan(ddouble_t *r) { *r = (ddouble_t) {NAN, NAN}; }
static void real_q_pi(ddouble_t *r) { *r = Q_PI; }
static void real_q_e(ddouble_t *r) { *r = Q_E; }

static void real_q_abs(ddouble_t *r, ddouble_t *a) { *r = *a; if (a->hi < 0.) *r = neg_q(*a); }
static void real_q_sign(ddouble_t *r, ddouble_t *a) { if (a->hi < 0.) *r = ddouble(-1.); else *r = ddouble(a->hi > 0.); }
static void real_q_nearbyint(ddouble_t *r, ddouble_t *a) { *r = nearbyint_q(*a); }
static void real_q_floor(ddouble_t *r, ddouble_t *a) { *r = floor_q(*a); }
static void real_q_ceil(ddouble_t *r, ddouble_t *a) { *r = ceil_q(*a); }
static void real_q_trunc(ddouble_t *r, ddouble_t *a) { *r = trunc_q(*a); }
static void real_q_clamp(ddouble_t *r, ddouble_t *a) { *r = rangelimit_qqq(*a, Q_ZERO, Q_ONE); }
static void real_q_cos(ddouble_t *r, ddouble_t *a) { *r = cos_q(*a); }
static void real_q_sin(ddouble_t *r, ddouble_t *a) { *r = sin_q(*a); }

static void real_q_min(ddouble_t *r, ddouble_t *a, ddouble_t *b) { *r = min_qq(*a, *b); }
static void real_q_max(ddouble_t *r, ddouble_t *a, ddouble_t *b) { *r = max_qq(*a, *b); }
static void real_q_pow(ddouble_t *r, ddouble_t *a, ddouble_t *b) { *r = ddouble(pow(a->hi, b->hi)); }	// FIXME

static void real_q_rangelimit(ddouble_t *r, ddouble_t *a, ddouble_t *b, ddouble_t *c) { *r = rangelimit_qqq(*a, *b, *c); }

static rlip_real_functions_t real_q_functions = {
	sizeof(ddouble_t),
	(void (*)(uint8_t *,uint8_t *))			real_q_set,
	(double (*)(uint8_t *))				real_q_cvt_r_d,
	(void (*)(uint8_t *,double))			real_q_cvt_d_r,
	(int64_t (*)(uint8_t *))			real_q_cvt_r_i,
	(void (*)(uint8_t *,int64_t))			real_q_cvt_i_r,
	(int (*)(const uint8_t *,const uint8_t *))	cmp_qq,
	(void (*)(uint8_t *,const char *,char **))	real_q_ator,
	(void (*)(uint8_t *))				NULL,
	(void (*)(uint8_t *))				NULL,
};

// Add these defaults to your inputs by doing rlip_inputs_t inputs[] = { RLIP_REAL_DOUBLEDOUBLE, RLIP_FUNC, {"x", &x, "pr"}, ... };
#define RLIP_REAL_DOUBLEDOUBLE				\
	{"rlip_real_functions", &real_q_functions, ""},	\
	{"add_", real_q_add,	"frrr"},		\
	{"sub_", real_q_sub,	"frrr"},		\
	{"mul_", real_q_mul,	"frrr"},		\
	{"div_", real_q_div,	"frrr"},		\
	{"nan_", real_q_nan,	"fr"},			\
	{"pi_", real_q_pi,	"fr"},			\
	{"e_", real_q_e,	"fr"},			\
	{"abs_", real_q_abs,	"frr"},			\
	{"sign_", real_q_sign,	"frr"},			\
	{"nearbyint_", real_q_nearbyint, "frr"},	\
	{"floor_", real_q_floor, "frr"},		\
	{"ceil_", real_q_ceil,	"frr"},			\
	{"trunc_", real_q_trunc, "frr"},		\
	{"clamp_", real_q_clamp, "frr"},		\
	{"cos_", real_q_cos,	"frr"},			\
	{"sin_", real_q_sin,	"frr"},			\
	{"min_", real_q_min,	"frrr"},		\
	{"max_", real_q_max,	"frrr"},		\
	{"pow_", real_q_pow,	"frrr"},		\
	{"rangelimit_", rangelimit, "frrrr"}
