#ifdef RL_OPENCL

void cl_copy_buffer_to_device(framebuffer_t fb, void *buffer, size_t offset, size_t size)
{
	cl_int ret = clEnqueueWriteBuffer(fb.clctx.command_queue, fb.data_cl, CL_FALSE, offset, size, buffer, 0, NULL, NULL);
	CL_ERR_NORET("clEnqueueWriteBuffer (in cl_copy_buffer_to_device, for fb.data_cl)", ret);
}

void cl_copy_raster_to_device(framebuffer_t fb, raster_t r, size_t offset)
{
	if (r.buf)
		cl_copy_buffer_to_device(fb, r.buf, offset, r.buf_size);
	else if (r.sq)
		cl_copy_buffer_to_device(fb, r.sq, offset, mul_x_by_y_xyi(r.dim)*sizeof(sqrgb_t));
	else if (r.srgb)
		cl_copy_buffer_to_device(fb, r.srgb, offset, mul_x_by_y_xyi(r.dim)*sizeof(srgb_t));
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

uint64_t cl_add_data_table_entry(framebuffer_t *fb, size_t table_index, size_t prev_end, void *buffer, size_t size, int *table_index_p)
{
	cl_data_alloc_t *entry = &fb->data_alloc_table[table_index];

	entry->start = prev_end;
	entry->end = entry->start + size;
	entry->unused = 0;
	entry->host_ptr = buffer;
	if (table_index_p)
		*table_index_p = table_index;

//fprintf_rl(stdout, "Copying buffer 0x%08lx +%d to offset %d - %d, index %d in table\n", buffer, size, entry->start, entry->end, table_index);
	cl_copy_buffer_to_device(*fb, buffer, entry->start, size);	// since the entry is new the data should be copied

	return entry->start;
}

hash_table_elem_t *hash_table_get_elem(framebuffer_t *fb, void *buffer, int hash)
{
	int index, ofi;
	hash_table_t *ht = &fb->hash_table;

	// look for the hash in the table
	index = ht->elem[hash].index;
	if (index >= 0)
		if (fb->data_alloc_table[index].host_ptr==buffer)
			return &ht->elem[hash];

	// look throw the chain in the overflow table
	ofi = ht->elem[hash].next_index;
	while (ofi >= 0)
	{
		index = ht->overflow[ofi].index;

		if (index < 0)
			return NULL;			// if it's the end of the chain and we found nothing

		if (fb->data_alloc_table[index].host_ptr==buffer)
			return &ht->overflow[ofi];

		ofi = ht->overflow[ofi].next_index;
	}

	return NULL;
}

int hash_table_get_index(framebuffer_t *fb, void *buffer, int hash)
{
	hash_table_elem_t *hte;

	hte = hash_table_get_elem(fb, buffer, hash);
	if (hte==NULL)
		return -1;

	return hte->index;
}

#endif

uint64_t cl_add_buffer_to_data_table(framebuffer_t *fb, void *buffer, size_t buffer_size, size_t align_size, int *table_index)
{
#ifdef RL_OPENCL
	int i, hash;
	size_t prev_end=0;
	const int ht_size = 1 << 16;
	const int ht_mask = ht_size - 1;

	// check the provided table index
	if (table_index)
	{
		i = *table_index;

		if (i >=0 && i < fb->data_alloc_table_count)
		if (fb->data_alloc_table[i].host_ptr==buffer)	// if it's there
		{
			fb->data_alloc_table[i].unused = 0;	// we just need to indicate it's still in use
			return fb->data_alloc_table[i].start;	// and that's it
		}
	}

	// check hash table
	hash = get_pointer_hash(buffer) & ht_mask;

	if (fb->hash_table.elem==NULL)	// alloc hash table
	{
		fb->hash_table.elem = calloc(ht_size, sizeof(hash_table_elem_t));
		alloc_enough(&fb->hash_table.overflow, 16, &fb->hash_table.overflow_as, sizeof(hash_table_elem_t), 2.);
	}

	// find if buffer is already in the table
	for (i=0; i < fb->data_alloc_table_count; i++)
	{
		if (fb->data_alloc_table[i].host_ptr==buffer)	// if it's there
		{
//fprintf_rl(stdout, "buffer is at index %d (rand %02d)\n", i, rand()%100);
			if (table_index)
				*table_index = i;

			fb->data_alloc_table[i].unused = 0;	// we just need to indicate it's still in use
			return fb->data_alloc_table[i].start;	// and that's it
		}
	}

	alloc_enough(&fb->data_alloc_table, fb->data_alloc_table_count+=1, &fb->data_alloc_table_as, sizeof(cl_data_alloc_t), 1.5);

	// look for a free space to insert
	for (i=0, prev_end=0; i < fb->data_alloc_table_count; i++)
	{
		if (fb->data_alloc_table[i].start - prev_end >= buffer_size)	// if there's a free space before this one
		{
			memmove(&fb->data_alloc_table[i+1], &fb->data_alloc_table[i], (fb->data_alloc_table_count-1 - i) * sizeof(cl_data_alloc_t));
			return cl_add_data_table_entry(fb, i, prev_end, buffer, buffer_size, table_index);
		}

		prev_end = next_aligned_offset(fb->data_alloc_table[i].end, align_size);
	}

	// add it at the end if there's enough space
	prev_end = 0;
	if (fb->data_alloc_table_count > 1)
		prev_end = next_aligned_offset(fb->data_alloc_table[fb->data_alloc_table_count-2].end, align_size);

	if (fb->data_cl_as - prev_end >= buffer_size)
		return cl_add_data_table_entry(fb, fb->data_alloc_table_count-1, prev_end, buffer, buffer_size, table_index);

	// TODO enlarge device buffer when needed
	return 0;
#else
	return (uint64_t) buffer;
#endif
}

uint64_t cl_add_raster_to_data_table(framebuffer_t *fb, raster_t *r)
{
	if (r->buf)
		return cl_add_buffer_to_data_table(fb, r->buf, r->buf_size, 4, &r->table_index);

	if (r->sq)
		return cl_add_buffer_to_data_table(fb, r->sq, mul_x_by_y_xyi(r->dim) * sizeof(sqrgb_t), sizeof(sqrgb_t), &r->table_index);

	if (r->srgb)
		return cl_add_buffer_to_data_table(fb, r->srgb, mul_x_by_y_xyi(r->dim) * sizeof(srgb_t), sizeof(srgb_t), &r->table_index);

	if (r->f)
		return cl_add_buffer_to_data_table(fb, r->f, mul_x_by_y_xyi(r->dim) * sizeof(frgb_t), sizeof(frgb_t), &r->table_index);

	return 0;
}
