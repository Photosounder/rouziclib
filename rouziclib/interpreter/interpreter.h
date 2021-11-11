enum opcode
{
	op_1word_ops = 1024,
	op_end,

	op_2word_ops = 2048,
	op_ret_v,
	op_jmp,
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

	op_jmp_v_ez,
	op_jmp_v_nz,
	op_jmp_i_ez,
	op_jmp_i_nz,
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
	op_pow_vv,

	op_jmp_vv_lt,
	op_jmp_ii_lt,
	op_func1_vv,

	op_5word_ops = 5120,
	op_func2_vvv,

	op_6word_ops = 6144,
	op_func3_vvvv,

	op_7word_ops = 7168,
	op_8word_ops = 8192,
};

typedef struct
{
	volatile int exec_on;
	int valid_prog;
	uint64_t *op;
	double *vd, **pd;
	int64_t *vi, **p_i;
	double return_value;
} rlip_t;

typedef struct
{
	const char *name;
	const void *ptr;
	const char *type;
} rlip_inputs_t;

extern rlip_t rlip_compile(const char *source, rlip_inputs_t *inputs, int input_count, buffer_t *log);
extern void free_rlip(rlip_t *prog);
