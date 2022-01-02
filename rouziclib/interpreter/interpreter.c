void free_rlip(rlip_t *prog)
{
	int i;

	if (prog->rf.var_deinit)
	{
		for (i=0; i < prog->vr_count; i++)
			prog->rf.var_deinit(&prog->vr[i * prog->rf.size_of_real]);

		for (i=0; i < prog->ret_count; i++)
			prog->rf.var_deinit(&prog->return_real[i]);
	}

	free(prog->op);
	free(prog->vd);
	free(prog->vi);
	free(prog->vr);
	free(prog->ptr);
	free(prog->return_value);
	free(prog->return_real);
	memset(prog, 0, sizeof(rlip_t));
}

double rlip_builtin_rand01(int64_t pos)
{
	uint32_t raw_value = rand_xsm32(pos + 0x3243f6a8u);
	return (double) raw_value * (1./4294967295.);
}
