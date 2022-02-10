enum opcode
{
	op_1word_ops = 1024,
	op_end,

	op_2word_ops = 2048,
	op_ret_d,
	op_ret_r,
	op_jmp,
	nop_jmp,
	op_set0_d,
	op_set0_i,
	op_inc1_d,
	op_inc1_i,

	op_3word_ops = 3072,
	op_ret_dd,
	op_ret_rr,
	op_load_d,
	op_load_i,
	op_load_r,
	op_set_d,
	op_set_i,
	op_set_r,
	op_cvt_i_d,
	op_cvt_d_i,
	op_cvt_r_d,
	op_cvt_d_r,
	op_cvt_r_i,
	op_cvt_i_r,
	op_sq_d,
	op_sqrt_d,

	op_jmp_cond,
	nop_jmp_cond,
	op_func0_d,
	op_func0_r,

	op_4word_ops = 4096,
	op_ret_ddd,
	op_ret_rrr,
	op_add_dd,
	op_add_ii,
	op_sub_dd,
	op_sub_ii,
	op_mul_dd,
	op_mul_ii,
	op_div_dd,
	op_div_ii,
	op_mod_ii,
	op_mod_dd,
	op_sqadd_dd,
	op_sqsub_dd,
	op_diff_dd,

	op_and_ii,
	op_or_ii,

	op_cmp_dd_eq,
	op_cmp_ii_eq,
	op_cmp_rr_eq,
	op_cmp_dd_ne,
	op_cmp_ii_ne,
	op_cmp_rr_ne,
	op_cmp_dd_lt,
	op_cmp_ii_lt,
	op_cmp_rr_lt,
	op_cmp_dd_le,
	op_cmp_ii_le,
	op_cmp_rr_le,
	op_cmp_dd_gt,
	op_cmp_ii_gt,
	op_cmp_rr_gt,
	op_cmp_dd_ge,
	op_cmp_ii_ge,
	op_cmp_rr_ge,
	op_func1_dd,
	op_func1_di,
	op_func1_ii,
	op_func1_rr,

	op_5word_ops = 5120,
	op_ret_dddd,
	op_ret_rrrr,
	op_aad_ddd,
	op_mmul_ddd,
	op_mad_ddd,
	op_adm_ddd,
	op_func2_ddd,
	op_func2_rrr,

	op_6word_ops = 6144,
	op_func3_dddd,
	op_func3_dddi,
	op_func3_rrrr,

	op_7word_ops = 7168,
	op_8word_ops = 8192,
};

typedef struct
{
	size_t size_of_real;
	void (*set)(uint8_t *,uint8_t *);
	double (*cvt_r_d)(uint8_t *);
	void (*cvt_d_r)(uint8_t *,double);
	int64_t (*cvt_r_i)(uint8_t *);
	void (*cvt_i_r)(uint8_t *,int64_t);
	int (*cmp)(const uint8_t *,const uint8_t *);
	void (*ator)(uint8_t *,const char *,char **);
	void (*get_pi)(uint8_t *);
	void (*get_e)(uint8_t *);
	void (*set_nan)(uint8_t *);
	void (*var_init)(uint8_t *);		// optional
	void (*var_deinit)(uint8_t *);		// optional
} rlip_real_functions_t;

#define opint_t int16_t

typedef struct
{
	volatile int *exec_on;
	int valid_prog;
	opint_t *op;
	double *vd;
	int64_t *vi;
	uint8_t *vr;
	rlip_real_functions_t rf;
	void **ptr;
	double *return_value;
	uint8_t *return_real;
	int vr_count, ret_count;
} rlip_t;

typedef struct
{
	const char *name;
	const void *ptr;
	const char *type;
} rlip_inputs_t;

extern void free_rlip(rlip_t *prog);
extern double rlip_builtin_rand01(int64_t pos);

// Add these defaults to your inputs by doing rlip_inputs_t inputs[] = { RLIP_FUNC, {"x", &x, "pd"}, ... };
#define RLIP_FUNC					\
	{"abs", fabs,	"fdd"},				\
	{"acos", acos,	"fdd"},				\
	{"asin", asin,	"fdd"},				\
	{"atan", atan,	"fdd"},				\
	{"atan2", atan2, "fddd"},			\
	{"ceil", ceil,	"fdd"},				\
	{"cos", cos,	"fdd"},				\
	{"cosh", cosh,	"fdd"},				\
	{"exp", exp,	"fdd"},				\
	{"floor", floor, "fdd"},			\
	{"log", log,	"fdd"},				\
	{"log10", log10, "fdd"},			\
	{"log2", log2,	"fdd"},				\
	{"pow", pow, "fddd"},				\
	{"copysign", copysign, "fddd"},			\
	{"sin", sin, "fdd"},				\
	{"sinh", sinh, "fdd"},				\
	{"tan", tan, "fdd"},				\
	{"tanh", tanh, "fdd"},				\
	{"gaussian", gaussian, "fdd"},			\
	{"erf", erf, "fdd"},				\
	{"erfr", erfr, "fdd"},				\
	{"erfinv", erfinv, "fdd"},			\
	{"integral_erfr", integral_of_erfr, "fdd"},	\
	{"short_erf", short_erf, "fddd"},		\
	{"sinc", sinc, "fddd"},				\
	{"lab_to_linear", Lab_L_to_linear, "fdd"},	\
	{"linear_to_lab", linear_to_Lab_L, "fdd"},	\
	{"lab_invert", Lab_L_invert, "fdd"},		\
	{"lsrgb", lsrgb, "fdd"},			\
	{"slrgb", slrgb, "fdd"},			\
	{"db_to_vol", db_to_vol, "fdd"},		\
	{"vol_to_db", vol_to_db, "fdd"},		\
	{"nearbyint", nearbyint, "fdd"},		\
	{"min", min_tefunc, "fddd"},			\
	{"max", max_tefunc, "fddd"},			\
	{"clamp", clamp_tefunc, "fdd"},			\
	{"rangelimit", rangelimit, "fdddd"},		\
	{"mix", mix, "fdddd"},				\
	{"sign", sign_tefunc, "fdd"},			\
	{"trunc", trunc, "fdd"},			\
	{"rand01", rlip_builtin_rand01, "fdi"}
