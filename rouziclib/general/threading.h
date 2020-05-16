// _Thread_local definition
#if !(defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201102L)) && !defined(_Thread_local)
  #if defined(__GNUC__) || defined(__INTEL_COMPILER) || defined(__SUNPRO_CC) || defined(__IBMCPP__)
    #define _Thread_local __thread
  #else
    #define _Thread_local __declspec(thread)
  #endif
#elif defined(__GNUC__) && defined(__GNUC_MINOR__) && (((__GNUC__ << 8) | __GNUC_MINOR__) < ((4 << 8) | 9))
  #define _Thread_local __thread
#endif

#if defined( __linux__ ) || defined( __APPLE__ ) || defined( __ANDROID__ )
    #include <errno.h>
    #include <unistd.h>
#endif

#define THREAD_U64 uint64_t
#include "../libraries/orig/thread.h"

extern int thread_detach(thread_ptr_t thread);
extern void thread_set_low_priority();

#define rl_thread_t thread_ptr_t
#define rl_thread_detach thread_detach
#define rl_thread_set_priority_low thread_set_low_priority
#define rl_thread_set_priority_high thread_set_high_priority
#define rl_thread_yield	thread_yield
#define rl_thread_join thread_join
extern int rl_thread_create(rl_thread_t *thread_handle, int (*func)(void *), void *arg);
extern int rl_thread_create_detached(int (*func)(void *), void *arg);
extern int rl_thread_join_and_null(rl_thread_t *thread_handle);

#define rl_mutex_t thread_mutex_t

#define rl_mutex_init(my_mutex)		thread_mutex_init(my_mutex)
#define rl_mutex_lock(my_mutex)		thread_mutex_lock(my_mutex)
#define rl_mutex_unlock(my_mutex)	thread_mutex_unlock(my_mutex)
#define rl_mutex_destroy(my_mutex)	thread_mutex_term(my_mutex)
extern rl_mutex_t *rl_mutex_init_alloc();
extern void rl_mutex_destroy_free(rl_mutex_t **mutex);

extern int32_t rl_atomic_get_and_set(int32_t *ptr, int32_t new_value);
