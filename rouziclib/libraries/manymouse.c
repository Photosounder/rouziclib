#ifdef RL_MANYMOUSE
#define init_mouse mm_init_mouse

#ifdef _WIN32
typedef PVOID HDEVINFO;

typedef struct _SP_DEVINFO_DATA {
    DWORD cbSize;
    GUID  ClassGuid;
    DWORD DevInst;
    ULONG_PTR Reserved;
} SP_DEVINFO_DATA, *PSP_DEVINFO_DATA;

#define WM_DESTROY 2
#define WS_OVERLAPPEDWINDOW 0x00CF0000L
#define DIGCF_PRESENT 2
#define DIGCF_ALLCLASSES 4
#define SPDRP_DEVICEDESC 0
#endif

#include "orig/manymouse/linux_evdev.c"
#include "orig/manymouse/macosx_hidmanager.c"
#include "orig/manymouse/macosx_hidutilities.c"
#include "orig/manymouse/windows_wminput.c"
#include "orig/manymouse/x11_xinput2.c"
#include "orig/manymouse/manymouse.c"

manymouse_t manymouse_state = {0};

void manymouse_reinit()
{
	if (manymouse_state.init)
		ManyMouse_Quit();

	manymouse_state.mouse_count = ManyMouse_Init();
	alloc_enough(&manymouse_state.mouse, manymouse_state.mouse_count, &manymouse_state.mouse_as, sizeof(*manymouse_state.mouse), 1.2);
	manymouse_state.init = 1;

	for (int i = 0; i < manymouse_state.mouse_count; i++)
		fprintf_rl(stdout, "Mouse \"%s\" detected\n", ManyMouse_DeviceName(i));
}

void manymouse_button_update(int *mb, mousebut_flags_t *flags, int new_state)
{
	if (new_state == 0 && *mb >= 0)		// if the button is being released
		*mb = -2;

	if (new_state && *mb <= 0)		// if the button is pressed
		*mb = 2;
}

void manymouse_poll()
{
	int i;

	// mouse_pre_event_proc() equivalent
	for (i=0; i < manymouse_state.mouse_count; i++)
	{
		manymouse_mouse_t *mouse = &manymouse_state.mouse[i];
		mouse->d = XYI0;
		mouse->b.wheel = 0;

		flag_update_mouse_button(&mouse->b.lmb, &mouse->b.lmf);
		flag_update_mouse_button(&mouse->b.mmb, &mouse->b.mmf);
		flag_update_mouse_button(&mouse->b.rmb, &mouse->b.rmf);
	}

	// Poll
	ManyMouseEvent event={0};

	while (ManyMouse_PollEvent(&event))
	{
		manymouse_mouse_t *mouse = &manymouse_state.mouse[event.device];

		switch (event.type)
		{
			case MANYMOUSE_EVENT_RELMOTION:
				if (event.item == 0)	// X axis
					manymouse_state.mouse[event.device].d.x += event.value;
				else			// Y axis
					manymouse_state.mouse[event.device].d.y += event.value;
				fprintf_rl(stdout, "Mouse #%u relative motion %s %d\n", event.device, event.item == 0 ? "X" : "Y", event.value);
				break;

			case MANYMOUSE_EVENT_ABSMOTION:
				if (event.item == 0)	// X axis
					manymouse_state.mouse[event.device].a.x = event.value;
				else			// Y axis
					manymouse_state.mouse[event.device].a.y = event.value;
				fprintf_rl(stdout, "Mouse #%u absolute motion %s %d\n", event.device, event.item == 0 ? "X" : "Y", event.value);
				break;

			case MANYMOUSE_EVENT_BUTTON:
				switch (event.item)
				{
					case 0: manymouse_button_update(&mouse->b.lmb, &mouse->b.lmf, event.value);	break;
					case 1: manymouse_button_update(&mouse->b.mmb, &mouse->b.mmf, event.value);	break;
					case 2: manymouse_button_update(&mouse->b.rmb, &mouse->b.rmf, event.value);	break;
				}
				fprintf_rl(stdout, "Mouse #%u button %d clicked\n", event.device, event.item);
				if (event.item == 3)
					SDL_SetRelativeMouseMode(1);
				break;

			case MANYMOUSE_EVENT_SCROLL:
				if (event.item == 0)		// Vertical scroll
					mouse->b.wheel += event.value;

				const char *wheel;
				const char *direction;
				if (event.item == 0)
				{
					wheel = "vertical";
					direction = event.value > 0 ? "up" : "down";
				}
				else
				{
					wheel = "horizontal";
					direction = event.value > 0 ? "right" : "left";
				}
				fprintf_rl(stdout, "Mouse #%u wheel %s %+d (%s)\n", event.device, wheel, event.value, direction);
				break;

			case MANYMOUSE_EVENT_DISCONNECT:
				fprintf_rl(stdout, "Mouse #%u disconnect\n", event.device);
				break;

			default:
				fprintf_rl(stdout, "Mouse #%u unhandled event type %d\n", event.device, event.type);
				break;
		}
	}
}

#undef init_mouse
#endif
