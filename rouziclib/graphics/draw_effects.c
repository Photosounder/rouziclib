// Drawqueue
void draw_effect_noarg_dq(enum dq_type type)
{
	float *df = drawq_add_to_main_queue(type);
	if (df==NULL)
		return;

	drawq_add_sectors_for_already_set_sectors();
}

void draw_effect_arg_double_dq(enum dq_type type, double v)
{
	float *df = drawq_add_to_main_queue(type);
	if (df==NULL)
		return;
	df[0] = v;

	drawq_add_sectors_for_already_set_sectors();
}

void draw_colour_matrix_dq(double *matrix)
{
	float *df = drawq_add_to_main_queue(DQT_COL_MATRIX);
	if (df==NULL)
		return;

	for (int i=0; i < 9; i++)
		df[i] = isnan(matrix[i]) ? 0.f : matrix[i];

	drawq_add_sectors_for_already_set_sectors();
}

// Drawqueue-enqueue
void draw_effect_noarg_dqnq(enum dq_type type)
{
	// Get pointer to data buffer
	volatile uint8_t *entry = dqnq_new_entry(DQNQT_EFFECT_NOARG);
	uint8_t *p = (uint8_t *) entry;

	// Write arguments to buffer
	write_LE32(&p, type);

	dqnq_finish_entry();
}

void draw_effect_arg_double_dqnq(enum dq_type type, double v)
{
	// Get pointer to data buffer
	volatile uint8_t *entry = dqnq_new_entry(DQNQT_EFFECT_FL1);
	uint8_t *p = (uint8_t *) entry;

	// Write arguments to buffer
	write_LE32(&p, type);
	write_LE32(&p, float_as_u32(v));

	dqnq_finish_entry();
}

void draw_colour_matrix_dqnq(double *matrix)
{
	// Get pointer to data buffer
	volatile uint8_t *entry = dqnq_new_entry(DQNQT_COL_MATRIX);
	uint8_t *p = (uint8_t *) entry;

	// Write arguments to buffer
	for (int i=0; i < 9; i++)
		write_LE32(&p, float_as_u32(matrix[i]));

	dqnq_finish_entry();
}

// Generic
void draw_effect_noarg(enum dq_type type)
{
	if (fb->use_dqnq)
		draw_effect_noarg_dqnq(type);
	else
		draw_effect_noarg_dq(type);
}

void draw_effect_arg_double(enum dq_type type, double v)
{
	if (fb->use_dqnq)
		draw_effect_arg_double_dqnq(type, v);
	else
		draw_effect_arg_double_dq(type, v);
}

void draw_colour_matrix(double *matrix)
{
	if (fb->use_dqnq)
		draw_colour_matrix_dqnq(matrix);
	else
		draw_colour_matrix_dq(matrix);
}
