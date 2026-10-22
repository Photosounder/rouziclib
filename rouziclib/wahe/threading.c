typedef struct
{
	wahe_mutex_t lock;
	uint32_t owner, depth;
} rl_wahe_mutex_t;

static _Thread_local uint32_t rl_wahe_thread_token;

EXPORT("module_thread_entry")
char *rl_wahe_thread_entry(char *message)
{
	// Dispatch only worker messages generated inside this module
	size_t function = 0, argument = 0;
	int end = 0;
	sscanf(message, "rouziclib worker %zx %zx%n", &function, &argument, &end);
	if (!end || !function)
		return NULL;
	((int (*)(void *)) function)((void *) argument);
	return NULL;
}

int rl_thread_create(rl_thread_t *handle, int (*function)(void *), void *argument)
{
	// Start an independent WAHE runner with its own heap-allocated stack and TLS
	char message[128];
	size_t id = 0;
	snprintf(message, sizeof(message), "rouziclib worker %zx %zx", (size_t) function, (size_t) argument);
	int success = wahe_thread_create(&id, message, 256 * 1024) == 0;
	*handle = (void *) id;
	return success;
}

int thread_join(rl_thread_t handle)
{
	// Release the host worker and its dynamically allocated runner storage
	return wahe_thread_join((size_t) handle, NULL) == 0;
}

int rl_thread_join_and_null(rl_thread_t *handle)
{
	// Preserve the native join-and-clear convention
	if (!handle || !*handle)
		return 0;
	int result = thread_join(*handle);
	*handle = NULL;
	return result;
}

int rl_thread_detach(rl_thread_t handle)
{
	// Require callers to retain joinable handles until worker shutdown
	(void) handle;
	return 0;
}

int rl_thread_create_detached(int (*function)(void *), void *argument)
{
	// Reject untracked workers rather than leaking their host runner and stack
	(void) function;
	(void) argument;
	return 0;
}

void thread_set_high_priority(void) {}
void rl_thread_set_priority_low(void) {}
void thread_yield(void)
{
	// Yield without relying on a native scheduling API
	sleep_ms(0);
}

void rl_sem_init(rl_sem_t *sem, int value)
{
	// Initialize a shared atomic semaphore
	wahe_semaphore_init(sem, value);
}
void rl_sem_destroy(rl_sem_t *sem)
{
	// Shared atomic semaphores do not retain native resources
	(void) sem;
}
void rl_sem_wait(rl_sem_t *sem)
{
	// Sleep atomically until a semaphore token becomes available
	wahe_semaphore_wait(sem);
}
void rl_sem_post(rl_sem_t *sem)
{
	// Release one token and wake a waiting worker
	wahe_semaphore_post(sem);
}

void rl_mutex_init(rl_mutex_t *mutex)
{
	// Fit recursive ownership bookkeeping inside the existing opaque mutex storage
	_Static_assert(sizeof(rl_wahe_mutex_t) <= sizeof(rl_mutex_t), "Mutex storage too small");
	memset(mutex, 0, sizeof(*mutex));
}
void rl_mutex_destroy(rl_mutex_t *mutex)
{
	// Shared atomic mutexes do not retain native resources
	(void) mutex;
}

int rl_mutex_trylock(rl_mutex_t *mutex)
{
	// Recognize recursive acquisition by the current runner's unique TLS address
	rl_wahe_mutex_t *state = (rl_wahe_mutex_t *) mutex;
	uint32_t token = (uint32_t) (size_t) &rl_wahe_thread_token;
	if (wahe_atomic_load_u32(&state->owner) == token)
	{
		state->depth++;
		return 1;
	}
	if (wahe_mutex_trylock(&state->lock) != WAHE_SYNC_OK)
		return 0;
	state->depth = 1;
	wahe_atomic_store_u32(&state->owner, token);
	return 1;
}

void rl_mutex_lock(rl_mutex_t *mutex)
{
	// Wait only when the mutex belongs to a different runner
	if (rl_mutex_trylock(mutex))
		return;
	rl_wahe_mutex_t *state = (rl_wahe_mutex_t *) mutex;
	wahe_mutex_lock(&state->lock);
	state->depth = 1;
	wahe_atomic_store_u32(&state->owner, (uint32_t) (size_t) &rl_wahe_thread_token);
}

void rl_mutex_unlock(rl_mutex_t *mutex)
{
	// Release the underlying mutex only after the outermost recursive acquisition
	rl_wahe_mutex_t *state = (rl_wahe_mutex_t *) mutex;
	if (--state->depth)
		return;
	wahe_atomic_store_u32(&state->owner, 0);
	wahe_mutex_unlock(&state->lock);
}

rl_mutex_t *rl_mutex_init_alloc(void)
{
	// Allocate zero-initialized recursive mutex storage
	return calloc(1, sizeof(rl_mutex_t));
}
void rl_mutex_destroy_free(rl_mutex_t **mutex)
{
	// Free only the allocated mutex and clear its owning pointer
	if (mutex)
	{
		free(*mutex);
		*mutex = NULL;
	}
}

int32_t rl_atomic_load_i32(volatile int32_t *ptr)
{
	// Observe writes published before the matching release operation
	return __atomic_load_n(ptr, __ATOMIC_ACQUIRE);
}
void rl_atomic_store_i32(volatile int32_t *ptr, int32_t value)
{
	// Publish preceding writes to readers using acquire loads
	__atomic_store_n(ptr, value, __ATOMIC_RELEASE);
}
int32_t rl_atomic_add_i32(volatile int32_t *ptr, int32_t value)
{
	// Return the value preceding an indivisible addition
	return __atomic_fetch_add(ptr, value, __ATOMIC_SEQ_CST);
}
int32_t rl_atomic_get_and_set(volatile int32_t *ptr, int32_t value)
{
	// Exchange the shared word and return its previous value
	return __atomic_exchange_n(ptr, value, __ATOMIC_SEQ_CST);
}
