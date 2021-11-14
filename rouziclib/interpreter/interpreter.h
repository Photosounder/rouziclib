enum opcode
{
	op_1word_ops = 1024,
	op_end,

	op_2word_ops = 2048,
	op_ret_v,
	op_jmp,
	nop_jmp,
	op_set0_v,
	op_set0_i,
	op_inc1_v,
	op_inc1_i,

	op_3word_ops = 3072,
	op_load_v,
	op_load_i,
	op_set_v,
	op_set_i,
	op_cvt_i_v,
	op_cvt_v_i,
	op_sq_v,
	op_sqrt_v,

	op_jmp_cond,
	nop_jmp_cond,
	op_func0_v,

	op_4word_ops = 4096,
	op_add_vv,
	op_add_ii,
	op_sub_vv,
	op_sub_ii,
	op_mul_vv,
	op_mul_ii,
	op_div_vv,
	op_div_ii,
	op_mod_ii,
	op_mod_vv,
	op_sqadd_vv,
	op_sqsub_vv,

	op_and_ii,
	op_or_ii,

	op_cmp_vv_eq,
	op_cmp_ii_eq,
	op_cmp_vv_ne,
	op_cmp_ii_ne,
	op_cmp_vv_lt,
	op_cmp_ii_lt,
	op_cmp_vv_le,
	op_cmp_ii_le,
	op_cmp_vv_gt,
	op_cmp_ii_gt,
	op_cmp_vv_ge,
	op_cmp_ii_ge,
	op_func1_vv,

	op_5word_ops = 5120,
	op_aad_vvv,
	op_mmul_vvv,
	op_mad_vvv,
	op_adm_vvv,
	op_func2_vvv,

	op_6word_ops = 6144,
	op_func3_vvvv,

	op_7word_ops = 7168,
	op_8word_ops = 8192,
};

#define opint_t int16_t

typedef struct
{
	volatile int *exec_on;
	int valid_prog;
	opint_t *op;
	double *vd;
	int64_t *vi;
	void **ptr;
	double return_value;
} rlip_t;

typedef struct
{
	const char *name;
	const void *ptr;
	const char *type;
} rlip_inputs_t;

extern void free_rlip(rlip_t *prog);

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
	{"trunc", trunc, "fdd"}
