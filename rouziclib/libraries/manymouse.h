#ifdef RL_MANYMOUSE

typedef struct
{
	xyi_t a, d;
	mousebut_t b;
} manymouse_mouse_t;

typedef struct
{
	int init;
	manymouse_mouse_t *mouse;
	size_t mouse_count, mouse_as;
} manymouse_t;

extern void manymouse_reinit();
extern void manymouse_poll();

extern manymouse_t manymouse_state;

#endif
