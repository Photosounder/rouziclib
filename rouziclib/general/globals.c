framebuffer_t fbs={0};
_Thread_local framebuffer_t *fb=&fbs;
_Thread_local zoom_t zc={0};
_Thread_local mouse_t mouse={0};
vector_font_t *font=NULL;
double drawing_thickness=0.8;
_Thread_local audiosys_t audiosys={0};
_Thread_local window_manager_t wind_man;

static _Thread_local rl_gui_context_t *rl_bound_gui_context=NULL;
static _Thread_local rl_gui_context_t rl_legacy_gui_context={0};

static void rl_gui_context_store_active(rl_gui_context_t *context)
{
	// Copy the active compatibility globals back into their owning context
	context->fb = fb;
	context->zc = zc;
	context->mouse = mouse;
	context->cur_textedit = cur_textedit;
	context->prev_textedit = prev_textedit;
	context->next_textedit = next_textedit;

	// Keep the stored zoom state pointing at the stored mouse state
	context->zc.mouse = &context->mouse;
}

static void rl_gui_context_load_active(rl_gui_context_t *context)
{
	// Load one context into the compatibility globals used throughout rouziclib
	fb = context->fb;
	zc = context->zc;
	mouse = context->mouse;
	cur_textedit = context->cur_textedit;
	prev_textedit = context->prev_textedit;
	next_textedit = context->next_textedit;

	// Repair the zoom pointer after copying the context mouse into active storage
	zc.mouse = &mouse;
}

rl_gui_context_t *rl_gui_context_bind(rl_gui_context_t *context)
{
	rl_gui_context_t *previous;

	// Leave nested bindings of the same context in place
	previous = rl_bound_gui_context;
	if (previous == context)
		return previous;

	// Save whichever context currently owns the compatibility globals
	if (previous)
		rl_gui_context_store_active(previous);
	else
		rl_gui_context_store_active(&rl_legacy_gui_context);

	// Load the requested context or restore the legacy unbound state
	rl_bound_gui_context = context;
	if (context)
		rl_gui_context_load_active(context);
	else
		rl_gui_context_load_active(&rl_legacy_gui_context);

	return previous;
}

rl_gui_context_t *rl_gui_context_get_bound(void)
{
	// Report the owner of the active compatibility globals
	return rl_bound_gui_context;
}

void rl_gui_context_init(rl_gui_context_t *context)
{
	rl_gui_context_t *previous;

	// Ignore missing storage before initializing an owned GUI context
	if (context==NULL)
		return;

	// Start with an empty context before using the normal mouse initializer
	memset(context, 0, sizeof(*context));
	context->initialized = 1;
	previous = rl_gui_context_bind(context);
	init_mouse();
	rl_gui_context_bind(previous);
}

void rl_gui_context_destroy(rl_gui_context_t *context)
{
	// Ignore missing or inactive context storage
	if (context==NULL || !context->initialized)
		return;

	// Restore legacy globals before releasing a context that is still bound
	if (rl_bound_gui_context == context)
		rl_gui_context_bind(NULL);

	// Release the mouse control stack owned by this context
	if (context->mouse.ctrl_id)
	{
		free(context->mouse.ctrl_id->stack);
		free(context->mouse.ctrl_id);
	}

	// Clear stale pointers so accidental reuse cannot reach released state
	memset(context, 0, sizeof(*context));
}
