#ifdef RL_TINYCTHREAD

int thrd_create_detached(thrd_start_t func, void *arg)
{
	int ret;
	thrd_t thr;

	ret = thrd_create(&thr, func, arg);
	if (ret != thrd_success)
	{
		if (ret == thrd_nomem)
			fprintf_rl(stderr, "No memory could be allocated for the thread requested (in thrd_create_detached(), for thrd_create())");
		if (ret == thrd_error)
			fprintf_rl(stderr, "The request could not be honoured (in thrd_create_detached(), for thrd_create())");

		return ret;
	}

	ret = thrd_detach(thr);
	if (ret != thrd_success)
		fprintf_rl(stderr, "The thread could not be detached (in thrd_create_detached(), for thrd_detach())");

	return ret;
}

#endif
