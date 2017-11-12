int noop_i(const char *function_name)
{
	fprintf_rl(stderr, "Warning: noop_i() being used instead of %s\n", function_name);
	return 0;
}

float noop_f(const char *function_name)
{
	fprintf_rl(stderr, "Warning: noop_f() being used instead of %s\n", function_name);
	return 0.f;
}
