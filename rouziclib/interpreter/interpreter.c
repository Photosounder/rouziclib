void free_rlip(rlip_t *prog)
{
	free(prog->op);
	free(prog->vd);
	free(prog->vi);
	free(prog->ptr);
	free(prog->return_value);
	memset(prog, 0, sizeof(rlip_t));
}

double rlip_builtin_rand01(int64_t pos)
{
	uint32_t raw_value = rand_xsm32(pos + 0x3243f6a8u);
	return (double) raw_value * (1./4294967295.);
}
