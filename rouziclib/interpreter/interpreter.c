void free_rlip(rlip_t *prog)
{
	int i;

	if (prog->rf.var_deinit)
	{
		for (i=0; i < prog->vr_count; i++)
			prog->rf.var_deinit(&prog->vr[i * prog->rf.size_of_real]);

		for (i=0; i < prog->ret_as; i++)
			prog->rf.var_deinit(&prog->return_real[i]);
	}

	free(prog->op);
	free(prog->vd);
	free(prog->vi);
	free(prog->vr);
	free((void *) prog->ptr);
	free(prog->return_value);
	free(prog->return_real);
	memset(prog, 0, sizeof(rlip_t));
}

double rlip_builtin_rand01(int64_t pos)
{
	uint32_t raw_value = rand_xsm32(pos + 0x3243f6a8u);
	return (double) raw_value * (1./4294967295.);
}

double rlip_builtin_min(double a, double b) { return MINN(a, b); }
double rlip_builtin_max(double a, double b) { return MAXN(a, b); }
double rlip_builtin_clamp(double v) { return rangelimit(v, 0., 1.); }
double rlip_builtin_sign(double v) { return v < 0. ? -1. : (v > 0. ? 1. : 0.); }
int64_t rlip_builtin_isnan(double v) { return isnan(v); }
int64_t rlip_builtin_isfinite(double v) { return isfinite(v); }
int64_t rlip_builtin_float_as_u32(double vd) { return (int64_t) float_as_u32((float) vd); }
int64_t rlip_builtin_double_as_u64(double vd) { return double_as_u64(vd); }
double rlip_builtin_u32_as_float(int64_t vi) { return (double) u32_as_float((uint32_t) vi); }
double rlip_builtin_u64_as_double(int64_t vi) { return u64_as_double((uint64_t) vi); }
int64_t rlip_builtin_bit_xor(int64_t a, int64_t b) { return a ^ b; }
int64_t rlip_builtin_bit_shl(int64_t a, int64_t b) { return a << b; }
int64_t rlip_builtin_bit_shr(int64_t a, int64_t b) { return a >> b; }
int64_t rlip_builtin_bit_neg(int64_t a) { return ~a; }
