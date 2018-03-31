#ifdef RL_OPENCL

void cl_copy_buffer_to_device(framebuffer_t fb, void *buffer, size_t offset, size_t size)
{
	cl_int ret = clEnqueueWriteBuffer(fb.clctx.command_queue, fb.data_cl, CL_FALSE, offset, size, buffer, 0, NULL, NULL);
	CL_ERR_NORET("clEnqueueWriteBuffer (in cl_copy_buffer_to_device, for fb.data_cl)", ret);
}

void cl_copy_raster_to_device(framebuffer_t fb, raster_t r, size_t offset)
{
	if (r.sq)
		cl_copy_buffer_to_device(fb, r.sq, offset, mul_x_by_y_xyi(r.dim)*sizeof(sqrgb_t));
	else if (r.f)
		cl_copy_buffer_to_device(fb, r.f, offset, mul_x_by_y_xyi(r.dim)*sizeof(frgb_t));
}

void data_cl_alloc(framebuffer_t *fb, int mb)
{
	cl_int ret;

	fb->data_cl_as = mb * 1024*1024;
	fb->data_cl = clCreateBuffer(fb->clctx.context, CL_MEM_READ_WRITE, fb->data_cl_as, NULL, &ret);
	CL_ERR_NORET("clCreateBuffer (in data_cl_alloc, for fb->data_cl)", ret);

	fb->data_alloc_table_count = 0;
	fb->data_alloc_table_as = 128;
	fb->data_alloc_table = calloc(fb->data_alloc_table_as, sizeof(cl_data_alloc_t));
}

void cl_data_table_remove_entry(framebuffer_t *fb, int i)
{
	fb->data_alloc_table_count--;
	memmove(&fb->data_alloc_table[i], &fb->data_alloc_table[i+1], (fb->data_alloc_table_count - i) * sizeof(cl_data_alloc_t));
}

void cl_data_table_prune_unused(framebuffer_t *fb)
{
	int i;

	// increment .unused in data_cl, remove the unused entries
	for (i=0; i < fb->data_alloc_table_count; i++)
	{
		fb->data_alloc_table[i].unused++;

		if (fb->data_alloc_table[i].unused >= 2)
			cl_data_table_remove_entry(fb, i);
	}
}

void cl_data_table_remove_entry_by_host_ptr(framebuffer_t *fb, void *host_ptr)
{
	int i;

	if (fb==NULL || host_ptr==NULL)
		return ;

	// find if buffer is already in the table
	for (i=0; i < fb->data_alloc_table_count; i++)
	{
		if (fb->data_alloc_table[i].host_ptr==host_ptr)	// if it's there
		{
			cl_data_table_remove_entry(fb, i);
			return ;
		}
	}
}

uint64_t cl_add_data_table_entry(framebuffer_t *fb, size_t table_index, size_t prev_end, void *buffer, size_t size)
{
	cl_data_alloc_t *entry = &fb->data_alloc_table[table_index];

	entry->start = prev_end;
	entry->end = entry->start + size;
	entry->unused = 0;
	entry->host_ptr = buffer;

//fprintf_rl(stdout, "Copying buffer 0x%08lx +%d to offset %d - %d, index %d in table\n", buffer, size, entry->start, entry->end, table_index);
	cl_copy_buffer_to_device(*fb, buffer, entry->start, size);	// since the entry is new the data should be copied

	return entry->start;
}

uint64_t cl_add_buffer_to_data_table(framebuffer_t *fb, void *buffer, size_t elem_count, size_t elem_size)
{
	int i;
	size_t size = elem_count*elem_size, prev_end=0;

	// find if buffer is already in the table
	for (i=0; i < fb->data_alloc_table_count; i++)
	{
		if (fb->data_alloc_table[i].host_ptr==buffer)	// if it's there
		{
			fb->data_alloc_table[i].unused = 0;	// we just need to indicate it's still in use
			return fb->data_alloc_table[i].start;	// and that's it
		}
	}

	alloc_enough(&fb->data_alloc_table, fb->data_alloc_table_count+=1, &fb->data_alloc_table_as, sizeof(cl_data_alloc_t), 1.5);

	// look for a free space to insert
	for (i=0, prev_end=0; i < fb->data_alloc_table_count; i++)
	{
		if (fb->data_alloc_table[i].start - prev_end >= size)	// if there's a free space before this one
		{
			memmove(&fb->data_alloc_table[i+1], &fb->data_alloc_table[i], (fb->data_alloc_table_count-1 - i) * sizeof(cl_data_alloc_t));
			return cl_add_data_table_entry(fb, i, prev_end, buffer, size);
		}

		prev_end = next_aligned_offset(fb->data_alloc_table[i].end, elem_size);
	}

	// add it at the end if there's enough space
	prev_end = 0;
	if (fb->data_alloc_table_count > 1)
		prev_end = next_aligned_offset(fb->data_alloc_table[fb->data_alloc_table_count-2].end, elem_size);

	if (fb->data_cl_as - prev_end >= size)
		return cl_add_data_table_entry(fb, fb->data_alloc_table_count-1, prev_end, buffer, size);

	// TODO enlarge device buffer when needed
	return 0;
}

uint64_t cl_add_raster_to_data_table(framebuffer_t *fb, raster_t r)
{
	if (r.sq)
		return cl_add_buffer_to_data_table(fb, r.sq, mul_x_by_y_xyi(r.dim), sizeof(sqrgb_t));

	else if (r.f)
		return cl_add_buffer_to_data_table(fb, r.f, mul_x_by_y_xyi(r.dim), sizeof(frgb_t));

	return 0;
}

#endif
