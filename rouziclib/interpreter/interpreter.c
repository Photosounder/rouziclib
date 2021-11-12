void free_rlip(rlip_t *prog)
{
	free(prog->op);
	free(prog->vd);
	free(prog->vi);
	free(prog->ptr);
}
