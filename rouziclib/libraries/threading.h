#ifdef RL_TINYCTHREAD

extern int thrd_create_detached(thrd_start_t func, void *arg);
extern mtx_t *mtx_init_alloc(int type);
extern void mtx_destroy_free(mtx_t **mtx);

#endif
