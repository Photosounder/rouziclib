void free_rlip(rlip_t *prog)
{
	free(prog->op);
	free(prog->vd);
	free(prog->vi);
	free(prog->ptr);
	free(prog->return_value);
	memset(prog, 0, sizeof(rlip_t));
}
