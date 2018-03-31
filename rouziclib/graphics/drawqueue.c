#ifdef RL_OPENCL
void drawq_reinit(framebuffer_t *fb)
{
	memset(fb->drawq_data, 0, fb->drawq_data[DQ_END] * sizeof(int32_t));
	memset (fb->sector_count, 0, fb->sectors * sizeof(int32_t));
	memset (fb->entry_list, 0, fb->entry_list_end * sizeof(int32_t));
	memset (fb->sector_list, 0, fb->sector_list[DQ_END] * sizeof(int32_t));

	fb->sector_list[DQ_END] = DQ_END_HEADER_SL;
	fb->sector_list[DQ_ENTRY_START] = DQ_END_HEADER_SL;
	fb->drawq_data[DQ_END] = 1;

	cl_data_table_prune_unused(fb);
}

void drawq_alloc(framebuffer_t *fb, int size)
{
	int32_t i;
	cl_int ret;
	int32_t *dataint;
	double ss;

	clFinish(fb->clctx.command_queue);	// wait for end of queue

	fb->drawq_size = size * 100;
	fb->list_alloc_size = size * 100;
	fb->max_sector_count = 16000;

	drawq_free(fb);

	fb->sector_size = 4;				// can't be smaller than 4 (16x16 px)

ss_calc:
	ss = (double) (1<<fb->sector_size);
	fb->sectors = ceil((double) fb->maxdim.x / ss) * ceil((double) fb->maxdim.y / ss);
	fb->sector_w = ceil((double) fb->maxdim.x / ss);
	if (fb->sectors >= fb->max_sector_count)
	{
		fb->sector_size++;
		goto ss_calc;
	}

	fb->drawq_data = calloc (fb->drawq_size, sizeof(int32_t));
	fb->sector_pos = calloc (fb->max_sector_count, sizeof(int32_t));
	fb->entry_list = calloc (fb->list_alloc_size, sizeof(int32_t));
	fb->sector_list = calloc (fb->list_alloc_size, sizeof(int32_t));
	fb->sector_count = calloc (fb->max_sector_count, sizeof(int32_t));

	fb->drawq_data_cl = clCreateBuffer(fb->clctx.context, CL_MEM_READ_ONLY, fb->drawq_size*sizeof(int32_t), NULL, &ret);
	CL_ERR_NORET("clCreateBuffer (in drawq_alloc, for fb->drawq_data_cl)", ret);
	fb->sector_pos_cl = clCreateBuffer(fb->clctx.context, CL_MEM_READ_ONLY, fb->max_sector_count*sizeof(int32_t), NULL, &ret);
	CL_ERR_NORET("clCreateBuffer (in drawq_alloc, for fb->sector_pos_cl)", ret);
	fb->entry_list_cl = clCreateBuffer(fb->clctx.context, CL_MEM_READ_ONLY, fb->list_alloc_size*sizeof(int32_t), NULL, &ret);
	CL_ERR_NORET("clCreateBuffer (in drawq_alloc, for fb->entry_list_cl)", ret);

	drawq_reinit(fb);
}

void drawq_free(framebuffer_t *fb)
{
	if (fb->drawq_data)
	{
		free (fb->drawq_data);
		free (fb->sector_pos);
		free (fb->entry_list);
		free (fb->sector_list);
		free (fb->sector_count);

		clReleaseMemObject(fb->drawq_data_cl);
		clReleaseMemObject(fb->sector_pos_cl);
		clReleaseMemObject(fb->entry_list_cl);

		fb->drawq_data = NULL;
	}
}

void drawq_run(framebuffer_t *fb)
{
	const char clsrc_draw_queue[] =
	#include "drawqueue.cl.h"

	int32_t i;
	cl_int ret, randseed;
	cl_event ev;
	static int init=1;
	static cl_program program;
	static cl_kernel kernel;
	size_t global_work_offset[2], global_work_size[2];

	if (init)
	{
		init=0;
		ret = build_cl_program(&fb->clctx, &program, clsrc_draw_queue);
		CL_ERR_NORET("build_cl_program (in drawq_run)", ret);

		ret = create_cl_kernel(&fb->clctx, program, &kernel, "draw_queue_srgb_kernel");
		CL_ERR_NORET("create_cl_kernel (in drawq_run)", ret);
	}
	if (kernel==NULL)
		return ;

	uint32_t z = 0;
	glClearTexImage(fb->gltex, 0, GL_RGBA, GL_UNSIGNED_BYTE, &z);
	
	ret = clEnqueueAcquireGLObjects(fb->clctx.command_queue, 1,  &fb->cl_srgb, 0, 0, NULL);		// get the ownership of cl_srgb
	CL_ERR_NORET("clEnqueueAcquireGLObjects (in drawq_run, for fb->cl_srgb)", ret);
	glFlush();
	glFinish();
	
	// enqueues copying result to fb->srgb FIXME not good if it was just resized up
//	ret = clEnqueueReadBuffer (fb->clctx.command_queue, fb->cl_srgb, CL_FALSE, 0, fb->w*fb->h*4*sizeof(uint8_t), fb->srgb, 0, NULL, NULL);
//	CL_ERR_RET("clEnqueueReadBuffer (in drawq_run, for fb->cl_srgb)", ret);

	// make entry list for each sector
	drawq_compile_lists(fb);

	// copy queue data to device
	ret = clEnqueueWriteBuffer(fb->clctx.command_queue, fb->drawq_data_cl, CL_FALSE, 0, fb->drawq_data[DQ_END]*sizeof(int32_t), fb->drawq_data, 0, NULL, NULL);
	CL_ERR_NORET("clEnqueueWriteBuffer (in drawq_run, for fb->drawq_data)", ret);
	ret = clEnqueueWriteBuffer(fb->clctx.command_queue, fb->sector_pos_cl, CL_FALSE, 0, fb->sectors*sizeof(int32_t), fb->sector_pos, 0, NULL, NULL);
	CL_ERR_NORET("clEnqueueWriteBuffer (in drawq_run, for fb->sector_pos)", ret);
	ret = clEnqueueWriteBuffer(fb->clctx.command_queue, fb->entry_list_cl, CL_FALSE, 0, fb->entry_list_end*sizeof(int32_t), fb->entry_list, 0, NULL, &ev);
	CL_ERR_NORET("clEnqueueWriteBuffer (in drawq_run, for fb->entry_list)", ret);

	// compute the random seed
	randseed = rand32();
	i = 24;
	while (fb->w*fb->h / (1<<i) < 4)	// while the period would be less than 4 times the number of pixels
		i++;				// double the period
	randseed %= ((1<<i) - fb->w*fb->h);	// seed + fbi will fit inside i bits to speed up the PRNG

	// Run the kernel
	ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), &fb->drawq_data_cl);	//CL_ERR_NORET("clSetKernelArg (in drawq_run, for fb->drawq_data_cl)", ret);
	ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), &fb->sector_pos_cl);	CL_ERR_NORET("clSetKernelArg (in drawq_run, for fb->sector_pos_cl)", ret);
	ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), &fb->entry_list_cl);	CL_ERR_NORET("clSetKernelArg (in drawq_run, for fb->entry_list_cl)", ret);
	ret = clSetKernelArg(kernel, 3, sizeof(cl_mem), &fb->data_cl);		CL_ERR_NORET("clSetKernelArg (in drawq_run, for fb->data_cl)", ret);
	ret = clSetKernelArg(kernel, 4, sizeof(cl_mem), &fb->cl_srgb);		CL_ERR_NORET("clSetKernelArg (in drawq_run, for fb->cl_srgb)", ret);
	ret = clSetKernelArg(kernel, 5, sizeof(cl_int), &fb->sector_w);		CL_ERR_NORET("clSetKernelArg (in drawq_run, for fb->sector_w)", ret);
	ret = clSetKernelArg(kernel, 6, sizeof(cl_int), &fb->sector_size);	CL_ERR_NORET("clSetKernelArg (in drawq_run, for fb->sector_size)", ret);
	ret = clSetKernelArg(kernel, 7, sizeof(cl_int), &randseed);		CL_ERR_NORET("clSetKernelArg (in drawq_run, for randseed)", ret);

	global_work_offset[0] = 0;
	global_work_offset[1] = 0;
	global_work_size[0] = fb->w;
	global_work_size[1] = fb->h;
	ret = clEnqueueNDRangeKernel(fb->clctx.command_queue, kernel, 2, global_work_offset, global_work_size, NULL, 0, NULL, NULL);
	CL_ERR_NORET("clEnqueueNDRangeKernel (in drawq_run)", ret);

	ret = clFlush(fb->clctx.command_queue);
	CL_ERR_NORET("clFlush (in drawq_run)", ret);

	// wait for srgb copy and draw queue copy to end
	clWaitForEvents(1, &ev);
	clReleaseEvent(ev);

	drawq_reinit(fb);	// clear/reinit the buffers
}

int32_t drawq_entry_size(const int32_t type)
{
	switch (type)
	{
		case DQT_BRACKET_OPEN:		return 0;
		case DQT_BRACKET_CLOSE:		return 1;
		case DQT_LINE_THIN_ADD:		return 8;
		case DQT_POINT_ADD:		return 6;
		case DQT_RECT_FULL:		return 8;
		case DQT_RECT_BLACK:		return 5;
		case DQT_PLAIN_FILL:		return 3;
		case DQT_CIRCLE_FULL:		return 7;
		//case DQT_BLIT_BILINEAR:		return 8;
		case DQT_BLIT_FLATTOP:		return 11;
		//case DQT_BLIT_PHOTO:		return 13;
		default:			return 0;
	}
}

void *drawq_add_to_main_queue(framebuffer_t fb, const int dqtype)
{
	int32_t end, *di = fb.drawq_data;
	int entry_size = drawq_entry_size(dqtype);

	// store the drawing parameters in the main drawing queue
	end = di[DQ_END];
	if (end + entry_size + 2 >= fb.drawq_size)		// if there's not enough room left
	{
		fprintf(stderr, "Draw queue size exceeded, %d numbers already in (%02d)\n", di[0], rand()%100);
		return di;
	}

	di[end] = dqtype;
	end++;

	di[DQ_END] += entry_size + 1;

	fb.sector_list[DQ_ENTRY_START] = fb.sector_list[DQ_END];	// set the start of the new entry
	fb.sector_list[DQ_END]++;					// sector_list becomes 1 larger by having the sector count (=0) for the new entry added

	return &di[end];
}

void drawq_add_sector_id(framebuffer_t fb, int32_t sector_id)
{
	if (fb.sector_list[DQ_END]+1 >= fb.list_alloc_size)
	{
		fprintf(stderr, "Sector list size exceeded, %d numbers already in (%02d)\n", fb.sector_list[DQ_END], rand()%100);
		return ;
	}

	fb.sector_count[sector_id]++;				// increment the count of entries for this sector

	fb.sector_list[fb.sector_list[DQ_END]] = sector_id;	// add the sector to the list
	fb.sector_list[fb.sector_list[DQ_ENTRY_START]]++;	// increment the sector count for this entry
	fb.sector_list[DQ_END]++;				// sector_list becomes 1 larger
}

void drawq_compile_lists(framebuffer_t *fb)		// makes entry_list and sector_pos
{
	int32_t i, j, end_i, end_j, main_i, sector, secpos, *count, pos=0;

	for (i=0; i<fb->sectors; i++)
	{
		// fill the sector position list
		if (fb->sector_count[i] > 0)			// if the sector contains entries
		{
			fb->sector_pos[i] = pos;		// this is where the sector starts in entry_list
			pos += fb->sector_count[i] + 1;		// add to the position the count of entries for each sector + 1 for the count of entries
		}
		else						// if the sector is empty
		{
			fb->sector_pos[i] = -1;			// it is nowhere in entry_list
		}
	}
	fb->entry_list_end = pos;				// remembers how filled entry_list is

	end_i = fb->sector_list[DQ_END];
	main_i = 1;						// the first entry in the main queue starts at that position
	for (i=DQ_END_HEADER_SL; i <= end_i; i++)		// go through each entry
	{
		end_j = fb->sector_list[i];
		for (j=1; j <= end_j; j++)			// go through each sector for this entry
		{
			sector = fb->sector_list[i+j];

			if (sector >= 0)			// negative sector id indicates a removed sector
			{
				secpos = fb->sector_pos[sector];
				count = &fb->entry_list[secpos];		// count of entry references for this sector
				(*count)++;
				fb->entry_list[secpos + *count] = main_i;	// store the position of this entry in the main queue
			}
		}

		i += end_j;

		// increment the main queue entry position by the size of the entry for its type + 1 (its type takes 1)
		main_i += drawq_entry_size(fb->drawq_data[main_i]) + 1;
	}
}

void drawq_bracket_open(framebuffer_t fb)
{
	int32_t ix, iy;
	xyi_t bb0, bb1;

	bb0 = xyi(0, 0);
	bb1 = xyi(fb.w-1 >> fb.sector_size, fb.h-1 >> fb.sector_size);
	
	// store the drawing parameters in the main drawing queue
	drawq_add_to_main_queue(fb, DQT_BRACKET_OPEN);

	// go through the affected sectors
	for (iy=bb0.y; iy<=bb1.y; iy++)
		for (ix=bb0.x; ix<=bb1.x; ix++)
			drawq_add_sector_id(fb, iy*fb.sector_w + ix);	// add sector reference
}

void drawq_bracket_close(framebuffer_t fb, int32_t blending_mode)
{
	int32_t ix, iy;
	int32_t *di;
	xyi_t bb0, bb1;

	bb0 = xyi(0, 0);
	bb1 = xyi(fb.w-1 >> fb.sector_size, fb.h-1 >> fb.sector_size);
	
	// store the drawing parameters in the main drawing queue
	di = drawq_add_to_main_queue(fb, DQT_BRACKET_CLOSE);
	di[0] = blending_mode;

	// go through the affected sectors
	for (iy=bb0.y; iy<=bb1.y; iy++)
		for (ix=bb0.x; ix<=bb1.x; ix++)
			drawq_add_sector_id(fb, iy*fb.sector_w + ix);	// add sector reference
}

void drawq_test1(framebuffer_t fb)		// BRDF test
{
	int32_t ix, iy;
	xyi_t bb0, bb1;

	bb0 = xyi(0, 0);
	bb1 = xyi(fb.w-1 >> fb.sector_size, fb.h-1 >> fb.sector_size);
	
	// store the drawing parameters in the main drawing queue
	drawq_add_to_main_queue(fb, DQT_TEST1);

	// go through the affected sectors
	for (iy=bb0.y; iy<=bb1.y; iy++)
		for (ix=bb0.x; ix<=bb1.x; ix++)
			drawq_add_sector_id(fb, iy*fb.sector_w + ix);	// add sector reference
}

int drawq_get_bounding_box(framebuffer_t fb, rect_t box, xy_t rad, recti_t *bbi)
{
	rect_t bb, screen_box = rect(XY0, xy(fb.w-1, fb.h-1));

	box = sort_rect(box);

	// calculate the bounding box
	bb.p0 = ceil_xy(sub_xy(box.p0, rad));
	bb.p1 = floor_xy(add_xy(box.p1, rad));

	if (check_box_box_intersection(bb, screen_box)==0)
		return 0;

	bbi->p0 = xy_to_xyi(max_xy(bb.p0, screen_box.p0));
	bbi->p1 = xy_to_xyi(min_xy(bb.p1, screen_box.p1));
	*bbi = rshift_recti(*bbi, fb.sector_size);

	return 1;
}

#endif
