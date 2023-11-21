char *wahe_execute_thread(wahe_thread_t *thread)
{
	wahe_group_t *group = thread->parent_group;
	char *last_msg = NULL;

	wahe_cur_thread = thread;

	// Execute all orders
	for (int ie=0; ie < thread->exec_order_count; ie++)
	{
		wahe_exec_order_t *eo = &thread->exec_order[ie];
		wahe_connection_t *conn = NULL;
		int ic;

		// Find the one connection to this execution order
		if (eo->type == WAHE_EO_KB_MOUSE)				// if this EO takes no inputs we look for the outgoing connection
		{
			for (ic=0; ic < thread->conn_count; ic++)
				if (thread->connection[ic].src_eo == ie)
				{
					conn = &thread->connection[ic];
					break;
				}
		}
		else
		{
			for (int ic=0; ic < thread->conn_count; ic++)
				if (thread->connection[ic].dst_eo == ie)
				{
					conn = &thread->connection[ic];
					break;
				}
		}

		// If the execution order concerns a module function
		if (eo->type == WAHE_EO_MODULE_FUNC)
		{
			wahe_module_t *exec_module = &group->module[eo->module_id];
			wahe_module_t *src_module = NULL, *dst_module = NULL;

			// Handle the connection that feeds into this EO's function
			if (conn)
			{
				// Copy message to module memory
				switch (thread->exec_order[conn->src_eo].type)
				{
					case WAHE_EO_MODULE_FUNC:
						src_module = &group->module[thread->exec_order[conn->src_eo].module_id];
						dst_module = &group->module[thread->exec_order[conn->dst_eo].module_id];

						call_module_free(dst_module, eo->dst_msg_addr);
						eo->dst_msg_addr = 0;

						size_t src_addr = thread->exec_order[conn->src_eo].ret_msg_addr;

						if (src_addr)
						{
							size_t copy_size = strlen(&src_module->memory_ptr[src_addr]) + 1;

							eo->dst_msg_addr = call_module_malloc(dst_module, copy_size);
							wahe_copy_between_memories(src_module, src_addr, copy_size, dst_module, eo->dst_msg_addr);
						}
						break;
				}
			}

			// Call function
			thread->current_eo = ie;
			thread->current_cmd_proc_id = 0;
			last_msg = call_module_func(exec_module, eo->dst_msg_addr, eo->func_id, 1);
			thread->current_eo = -1;
		}

		// If the execution order is to put keyboard-mouse messages in a module's memory
		if (eo->type == WAHE_EO_KB_MOUSE && conn)
		{
			wahe_make_keyboard_mouse_messages(thread, eo->module_id, eo->display_id, ic);
		}

		// If the execution order is to display the image found in a return message
		if (eo->type == WAHE_EO_IMAGE_DISPLAY && conn)
		{
			// Make display image from message in the source module memory
			if (thread->exec_order[conn->src_eo].type == WAHE_EO_MODULE_FUNC)
			{
				wahe_module_t *src_module = &group->module[thread->exec_order[conn->src_eo].module_id];
				size_t src_addr = thread->exec_order[conn->src_eo].ret_msg_addr;

				// Make raster from message
				raster_t *r = &group->image[eo->display_id].fb;
				wahe_message_to_raster(src_module, src_addr, r);

				// Calculate actual image rectangle inside the display area
				group->image[eo->display_id].fb_rect = fit_rect_in_area(xyi_to_xy(r->dim), group->image[eo->display_id].fb_area, xy(0.5, 0.5));
			}
		}
	}

	return last_msg;
}

void wahe_blit_group_displays(wahe_group_t *group)
{
	for (int i=0; i < group->image_count; i++)
	{
		blit_in_rect(&group->image[i].fb, sc_rect(group->image[i].fb_rect), 1, AA_NEAREST_INTERP);
	}
}
