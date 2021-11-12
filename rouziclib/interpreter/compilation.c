enum opcode_table
{
	table_none=0,
	table_vd,
	table_vi,
	table_ptr,
	table_loc,
};

typedef struct 
{
	char *name;
	uint64_t hash;
	char *type;
	enum opcode_table table;	// table is 0 for not in a table (like function pointers), 1 for vd, 2 for vi, 3 for ptr
	size_t index;			// index is the index in the given table
} rlip_reg_t;

typedef struct 
{
	rlip_t *d;
	opint_t *op;
	rlip_reg_t *reg;
	int *loc;
	size_t op_count, op_as, vd_count, vd_as, vi_count, vi_as, ptr_count, ptr_as, loc_count, loc_as, reg_count, reg_as;
} rlip_data_t;

int rlip_find_value(const char *name, rlip_data_t *ed)
{
	int i;
	uint64_t hash;

	if (name==NULL)
		return -1;

	hash = get_string_hash(name);

	// Look for existing instance of the value
	for (i=0; i < ed->reg_count; i++)
		if (hash == ed->reg[i].hash)
			if (strcmp(name, ed->reg[i].name)==0)
				return i;			// return reg index

	return -1;
}

void rlip_add_value_at_reg_index(int ir, const char *name, const void *ptr, const char *type, rlip_data_t *ed)
{
	ed->reg[ir].name = make_string_copy(name);	// name can be NULL in which case this is a nameless variable only usable by the compiler
	if (name)
		ed->reg[ir].hash = get_string_hash(name);
	ed->reg[ir].type = make_string_copy(type);

	if (strcmp(type, "d")==0)			// add to vd table
	{
		ed->reg[ir].table = table_vd;
		ed->reg[ir].index = ed->vd_count;
		alloc_enough(&ed->d->vd, ed->vd_count+=1, &ed->vd_as, sizeof(double), 1.5);
		if (ptr)
			ed->d->vd[ed->reg[ir].index] = *(double *) ptr;
	}

	else if (strcmp(type, "i")==0)			// add to vi table
	{
		ed->reg[ir].table = table_vi;
		ed->reg[ir].index = ed->vi_count;
		alloc_enough(&ed->d->vi, ed->vi_count+=1, &ed->vi_as, sizeof(int64_t), 1.5);
		if (ptr)
			ed->d->vi[ed->reg[ir].index] = *(int64_t *) ptr;
	}

	else if (type[0] == 'p' || type[0] == 'f')	// add to ptr table
	{
		ed->reg[ir].table = table_ptr;
		ed->reg[ir].index = ed->ptr_count;
		alloc_enough((void **) &ed->d->ptr, ed->ptr_count+=1, &ed->ptr_as, sizeof(void *), 1.5);
		ed->d->ptr[ed->reg[ir].index] = (void *) ptr;
	}

	else if (strcmp(type, "l")==0)			// add to loc table
	{
		ed->reg[ir].table = table_loc;
		ed->reg[ir].index = ed->loc_count;
		alloc_enough((void **) &ed->loc, ed->loc_count+=1, &ed->loc_as, sizeof(void *), 1.5);
		if (ptr)
			ed->loc[ed->reg[ir].index] = *(int *) ptr;	// the ptr is missing if this is being called when adding a goto command to a forward location
	}
}

int rlip_add_value(const char *name, const void *ptr, const char *type, rlip_data_t *ed)
{
	int ir;

	// Look for existing instance of the value
	ir = rlip_find_value(name, ed);
	if (ir > -1)
		return ir;

	// Add new registry entry
	ir = ed->reg_count;
	alloc_enough(&ed->reg, ed->reg_count+=1, &ed->reg_as, sizeof(rlip_reg_t), 1.5);

	rlip_add_value_at_reg_index(ir, name, ptr, type, ed);

	return ir;
}

size_t alloc_opcode(rlip_data_t *ed, size_t add_count)
{
	size_t index = ed->op_count;
	alloc_enough(&ed->op, ed->op_count+=add_count, &ed->op_as, sizeof(opint_t), 1.5);
	return index;
}

void prepend_opcode(rlip_data_t *ed, size_t add_count)
{
	size_t i, move_count = alloc_opcode(ed, add_count);			// make room for the ops to prepend
	memmove(&ed->op[add_count], ed->op, move_count * sizeof(ed->op[0]));	// move all ops to make room for the new ones at the beginning

	// Shift all registered locations
	for (i=0; i < ed->loc_count; i++)
		ed->loc[i] += add_count;
}

void convert_pointer_to_variable(int ir, rlip_data_t *ed)
{
	// Convert a pointer to a variable forever
	if (ed->reg[ir].type[0] == 'p')
	{
		// Prepend load opcode
		prepend_opcode(ed, 3);
		ed->op[0] = ed->reg[ir].type[1]=='d' ? op_load_v : op_load_i;
		ed->op[2] = ed->reg[ir].index;

		// Convert reg entry from pointer to variable
		rlip_reg_t old_entry = ed->reg[ir];
		rlip_add_value_at_reg_index(ir, old_entry.name, NULL, old_entry.type[1]=='d' ? "d" : "i", ed);
		free(old_entry.name);
		free(old_entry.type);

		// Set destination
		ed->op[1] = ed->reg[ir].index;
	}
}

void convert_expression_to_variable(const char *name, rlip_data_t *ed)
{
	double v = te_interp(name, NULL);			// interpret name as an expression

	if (isnan(v)==0)					// if it's a valid expression
		rlip_add_value(name, &v, "d", ed);		// add the value as a variable with the original expression as its name
}

int rlip_find_convert_value(const char *name, rlip_data_t *ed)
{
	int ir;

	// It might be an expression, in which case make it a variable
	convert_expression_to_variable(name, ed);

	ir = rlip_find_value(name, ed);

	if (ir > -1)
		// It might be a pointer, in which case make it a variable permanently
		convert_pointer_to_variable(ir, ed);

	return ir;
}

void rlip_convert_mismatched_var_to_register(int *ir, char expected_type, int *rd, int *ri, int i, rlip_data_t *ed)
{
	int io;

	// Convert the variable to a generic register if the type doesn't match the one expected
	if (ed->reg[*ir].type[0] != expected_type)
	{
		io = alloc_opcode(ed, 3);		// add conversion opcode
		ed->op[io+2] = ed->reg[*ir].index;	// source index

		if (expected_type == 'd')
		{
			ed->op[io] = op_cvt_i_v;
			ed->op[io+1] = rd[1+i];		// destinaton index
		}
		else
		{
			ed->op[io] = op_cvt_v_i;
			ed->op[io+1] = ri[1+i];
		}

		*ir = ed->op[io+1];	// the ir for this argument is now the generic register
	}
}

rlip_t rlip_compile(const char *source, rlip_inputs_t *inputs, int input_count, buffer_t *log)
{
	int i, io, ir, il, ret, linecount, n, add_var_type, dest_ir, ret_cmd_done=0, cmd_found, fwd_jumps=0;
	int rd[8], ri[8];	// generic registers
	char **line = arrayise_text(make_string_copy(source), &linecount);
	rlip_t data={0};
	rlip_data_t extra_data={0}, *ed=&extra_data;
	char *p, s0[32], s1[32], s2[32], s3[32], cmd_arg_type[16];
	int arg_ir[16];
	uint64_t hash;
	enum opcode new_opcode;

	ed->d = &data;

	// Add generic registers
	for (i=0; i < sizeof(rd)/sizeof(*rd); i++)
		rd[i] = ed->reg[rlip_add_value(sprintf_ret(s0, "rd%d", i), NULL, "d", ed)].index;

	for (i=0; i < sizeof(ri)/sizeof(*ri); i++)
		ri[i] = ed->reg[rlip_add_value(sprintf_ret(s0, "ri%d", i), NULL, "i", ed)].index;

	// Add inputs to structure
	for (i=0; i < input_count; i++)
		rlip_add_value(inputs[i].name, inputs[i].ptr, inputs[i].type, ed);

	// Parse lines and compile them into opcodes
	for (il=0; il < linecount; il++)
	{
		add_var_type = 0;
		p = line[il];

line_proc_start:
		s0[0] = '\0';
		s1[0] = '\0';
		s2[0] = '\0';
		s3[0] = '\0';
		n = 0;
		sscanf(p, "%30s = %n", s0, &n);

		// Declaring a new variable by its type
		if (strcmp(s0, "d")==0 || strcmp(s0, "i")==0)
		{
			// Set flag for creation of variable
			if (s0[0]=='d')
				add_var_type = table_vd;
			else
				add_var_type = table_vi;

			// Jump back to end up at the processing of "<var> = ..."
			sscanf(p, "%*s%n", &n);
			p = &p[n];

			goto line_proc_start;
		}

		// Declaring a location
		else if (s0[strlen(s0)-1] == ':')
		{
			s0[strlen(s0)-1] = '\0';	// remove : to make location name

			// Check for previous declaration
			ir = rlip_find_value(s0, ed);
			if (ir > -1)	// this most likely is the result of a forward jump meaning we must go through previous opcodes to add in the correct offsets
			{
				ed->loc[ed->reg[ir].index] = ed->op_count;	// add the correct current location

				// Go through all previous opcodes to look for forward jumps to this location
				for (io=0; io < ed->op_count;)
				{
					if (ed->op[io] == op_jmp)
						if (ed->op[io+1] == ir)
						{
							ed->op[io+1] = ed->op_count - io;
							fwd_jumps--;
						}

					if (ed->op[io] == op_jmp_cond)
						if (ed->op[io+2] == ir)
						{
							ed->op[io+2] = ed->op_count - io;
							fwd_jumps--;
						}

					io += ed->op[io] >> 10;
				}
			}
			else
			{
				// Add location in table
				ir = rlip_add_value(s0, &ed->op_count, "l", ed);
			}
		}

		// When doing "<var> = ..."
		else if (n)
		{
			p = &p[n];

			// Add or find destination var
			if (add_var_type)
				dest_ir = rlip_add_value(s0, NULL, add_var_type==table_vd ? "d" : "i", ed);
			else
				dest_ir = rlip_find_value(s0, ed);
			
			if (dest_ir == -1)
			{
				bufprintf(log, "Undeclared variable '%s' used in line %d: '%s'\n", s0, il, line[il]);
				goto invalid_prog;
			}

			if (ed->reg[dest_ir].type[0]!='d' && ed->reg[dest_ir].type[0]!='i')
			{
				bufprintf(log, "Assignment to variable '%s' of invalid type '%s' used in line %d: '%s'\n", s0, ed->reg[dest_ir].type, il, line[il]);
				goto invalid_prog;
			}

			// Read command
			ret = sscanf(p, "%30s %n", s0, &n);
			cmd_found = 0;

			if (ret == 1)
			{
				int cmd_arg_count = 0;

				// Identify the command and find its argument count
				if (	strcmp(s0, "sq")==0 ||
					strcmp(s0, "sqrt")==0 )
				{
					cmd_arg_count = 1;
					sprintf(cmd_arg_type, "dd");
					cmd_found = 1;
				}

				if (	strcmp(s0, "add")==0 ||
					strcmp(s0, "sub")==0 ||
					strcmp(s0, "mul")==0 ||
					strcmp(s0, "div")==0 ||
					strcmp(s0, "mod")==0 )
				{
					cmd_arg_count = 2;
					sprintf(cmd_arg_type, "ddd");
					cmd_found = 1;
				}

				if (	strcmp(s0, "addi")==0 ||
					strcmp(s0, "subi")==0 ||
					strcmp(s0, "muli")==0 ||
					strcmp(s0, "divi")==0 ||
					strcmp(s0, "modi")==0 ||
					strcmp(s0, "and")==0 ||
					strcmp(s0, "or")==0 )
				{
					cmd_arg_count = 2;
					sprintf(cmd_arg_type, "iii");
					cmd_found = 1;
				}

				// Add command
				if (cmd_found == 1)
				{
					new_opcode = 0;

					if (strcmp(s0, "sq")==0)		new_opcode = op_sq_v;
					else if (strcmp(s0, "sqrt")==0)	new_opcode = op_sqrt_v;
					else if (strcmp(s0, "add")==0)	new_opcode = op_add_vv;
					else if (strcmp(s0, "addi")==0)	new_opcode = op_add_ii;
					else if (strcmp(s0, "sub")==0)	new_opcode = op_sub_vv;
					else if (strcmp(s0, "subi")==0)	new_opcode = op_sub_ii;
					else if (strcmp(s0, "mul")==0)	new_opcode = op_mul_vv;
					else if (strcmp(s0, "muli")==0)	new_opcode = op_mul_ii;
					else if (strcmp(s0, "div")==0)	new_opcode = op_div_vv;
					else if (strcmp(s0, "divi")==0)	new_opcode = op_div_ii;
					else if (strcmp(s0, "mod")==0)	new_opcode = op_mod_vv;
					else if (strcmp(s0, "modi")==0)	new_opcode = op_mod_ii;
					else if (strcmp(s0, "and")==0)	new_opcode = op_and_ii;
					else if (strcmp(s0, "or")==0)	new_opcode = op_or_ii;
add_command:
					// Go through arguments to convert them and determine their types
					for (i=0; i < cmd_arg_count; i++)
					{
						p = &p[n];
						n = 0;
						ret = sscanf(p, "%30s %n", s1, &n);

						if (ret)
						{
							arg_ir[i] = rlip_find_convert_value(s1, ed);

							if (arg_ir[i] == -1)
							{
								bufprintf(log, "Argument '%s' unidentified in line %d: '%s'\n", s1, il, line[il]);
								goto invalid_prog;
							}

							// Convert the variable to a generic register if the type doesn't match the one expected
							if (cmd_arg_type[1+i] != 'f')
								rlip_convert_mismatched_var_to_register(&arg_ir[i], cmd_arg_type[1+i], rd, ri, i, ed);
						}
						else
						{
							bufprintf(log, "Argument missing (%d arguments expected) in line %d: '%s'\n", cmd_arg_count, il, line[il]);
							goto invalid_prog;
						}
					}

					// Add opcode
					io = alloc_opcode(ed, 2+cmd_arg_count);
					ed->op[io] = new_opcode;

					// Add arguments to opcode
					for (i=0; i < cmd_arg_count; i++)
						ed->op[io+2+i] = ed->reg[arg_ir[i]].index;

					// Destination (assuming same type)
					ed->op[io+1] = ed->reg[dest_ir].index;

					// Convert result to destination type
					if (ed->reg[dest_ir].type[0] != cmd_arg_type[0])
					{
						// Output to generic register
						ed->op[io+1] = cmd_arg_type[0]=='d' ? rd[0] : ri[0];

						// Convert from generic register to destination
						io = alloc_opcode(ed, 3);	// add conversion opcode
						ed->op[io+1] = ed->reg[dest_ir].index;	// destination index

						if (cmd_arg_type[0] == 'd')
						{
							ed->op[io] = op_cvt_v_i;
							ed->op[io+2] = rd[0];		// source index
						}
						else
						{
							ed->op[io] = op_cvt_i_v;
							ed->op[io+2] = ri[0];
						}
					}
				}

				// Commands less typical needs
				if (cmd_found == 0)
				{
					if (strcmp(s0, "cmp")==0 || strcmp(s0, "cmpi")==0)
					{
						cmd_found = 1;
						if (strcmp(s0, "cmp")==0)
							sprintf(cmd_arg_type, "idd");
						else
							sprintf(cmd_arg_type, "iii");

						n = 0;
						ret = sscanf(p, "%*s %30s %30s %30s %n", s1, s2, s3, &n);
						
						if (ret == 3)
						{
							// Find / convert variables
							for (i=0; i < 2; i++)
							{
								arg_ir[i] = rlip_find_convert_value(i==0 ? s1 : s3, ed);

								if (arg_ir[i] == -1)
								{
									bufprintf(log, "Argument '%s' unidentified in line %d: '%s'\n", i==0 ? s1 : s3, il, line[il]);
									goto invalid_prog;
								}

								// Convert the variable to a generic register if the type doesn't match the one expected
								rlip_convert_mismatched_var_to_register(&arg_ir[i], cmd_arg_type[1+i], rd, ri, i, ed);
							}

							// Add cmp opcode
							io = alloc_opcode(ed, 4);

							// Select correct opcode
							if (strcmp(s2, "==")==0)	ed->op[io] = cmd_arg_type[1]=='d' ? op_cmp_vv_eq : op_cmp_ii_eq;
							if (strcmp(s2, "!=")==0)	ed->op[io] = cmd_arg_type[1]=='d' ? op_cmp_vv_ne : op_cmp_ii_ne;
							if (strcmp(s2, "<")==0) 	ed->op[io] = cmd_arg_type[1]=='d' ? op_cmp_vv_lt : op_cmp_ii_lt;
							if (strcmp(s2, "<=")==0)	ed->op[io] = cmd_arg_type[1]=='d' ? op_cmp_vv_le : op_cmp_ii_le;
							if (strcmp(s2, ">")==0) 	ed->op[io] = cmd_arg_type[1]=='d' ? op_cmp_vv_gt : op_cmp_ii_gt;
							if (strcmp(s2, ">=")==0)	ed->op[io] = cmd_arg_type[1]=='d' ? op_cmp_vv_ge : op_cmp_ii_ge;

							ed->op[io+1] = ed->reg[dest_ir].index;
							ed->op[io+2] = ed->reg[arg_ir[0]].index;
							ed->op[io+3] = ed->reg[arg_ir[1]].index;
							
							// Convert integer result to double if needed
							if (ed->reg[dest_ir].type[0] == 'd')
							{
								// Output to generic register
								ed->op[io+1] = rd[0];

								// Convert from generic register to destination
								io = alloc_opcode(ed, 3);		// add conversion opcode
								ed->op[io] = op_cvt_v_i;
								ed->op[io+1] = ed->reg[dest_ir].index;	// destination index
								ed->op[io+2] = rd[0];			// source index
							}
						}
						else
						{
							bufprintf(log, "Argument missing (2 arguments expected) in line %d: '%s'\n", il, line[il]);
							goto invalid_prog;
						}
					}
				}

				// Check if it's a variable, pointer, expression or function pointer rather than a command making "=" a set, cvt or func command
				if (cmd_found == 0)
				{
					ir = rlip_find_convert_value(s0, ed);	// converts pointers and expressions to variables
					if (ir > -1)
					{
						// Variable
						if (strcmp(ed->reg[ir].type, "d")==0 || strcmp(ed->reg[ir].type, "i")==0)
						{
							io = alloc_opcode(ed, 3);

							if (ed->reg[dest_ir].table == ed->reg[ir].table)
								ed->op[io] = ed->reg[ir].type[0]=='d' ? op_set_v : op_set_i;
							else
								ed->op[io] = ed->reg[ir].type[0]=='d' ? op_cvt_v_i : op_cvt_i_v;

							ed->op[io+1] = ed->reg[dest_ir].index;
							ed->op[io+2] = ed->reg[ir].index;
							cmd_found = 2;
						}

						// Provided function pointer
						if (ed->reg[ir].type[0]=='f')
						{
							cmd_arg_count = strlen(ed->reg[ir].type) - 1;
							sprintf(cmd_arg_type, "%s", ed->reg[ir].type);
							swap_char(&cmd_arg_type[0], &cmd_arg_type[1]);
							cmd_found = 3;

							// FIXME only functions with double format arguments are supported
							switch (cmd_arg_count)
							{
								case 1:	new_opcode = op_func0_v;	break;
								case 2:	new_opcode = op_func1_vv;	break;
								case 3:	new_opcode = op_func2_vvv;	break;
								case 4:	new_opcode = op_func3_vvvv;	break;
								
								default:
									bufprintf(log, "Argument count (%d) not supported in line %d: '%s'\n", cmd_arg_count, il, line[il]);
									goto invalid_prog;
							}

							n = 0;			// makes the parsing keep the name of the function as an argument
							goto add_command;
						}
					}
				}

				if (cmd_found == 0)
				{
					bufprintf(log, "Unidentified '%s' in line %d: '%s'\n", s0, il, line[il]);
					goto invalid_prog;
				}
			}
		}

		// Commands starting with the command name
		else
		{
			if (strcmp(s0, "return")==0)
			{
				sscanf(p, "%*s %30s", s1);
				ir = rlip_find_convert_value(s1, ed);

				// Add return opcode
				if (ir > -1)
				{
					if (strcmp(ed->reg[ir].type, "d")==0)
					{
						// Add return opcode
						io = alloc_opcode(ed, 2);
						ed->op[io] = op_ret_v;
						ed->op[io+1] = ed->reg[ir].index;
						ret_cmd_done = 1;
					}
					else
					{
						// Convert from integer to double
						io = alloc_opcode(ed, 3);	// add conversion opcode
						ed->op[io] = op_cvt_i_v;
						ed->op[io+1] = rd[0];			// convert to generic register
						ed->op[io+2] = ed->reg[ir].index;

						// Add return opcode
						io = alloc_opcode(ed, 2);
						ed->op[io] = op_ret_v;
						ed->op[io+1] = rd[0];
						ret_cmd_done = 1;
					}
				}
				else
				{
					bufprintf(log, "Value not found for command 'return' in line %d: '%s'\n", il, line[il]);
					goto invalid_prog;
				}
			}

			if (strcmp(s0, "if")==0)
			{
				ret = sscanf(p, "%*s %30s goto %30s", s1, s2);

				if (ret == 2)
				{
					arg_ir[0] = rlip_find_value(s1, ed);
					//arg_ir[1] = rlip_find_value(s2, ed);
					arg_ir[1] = rlip_add_value(s2, NULL, "l", ed);

					for (i=0; i < 2; i++)
					{
						if (arg_ir[i] == -1)
						{
							bufprintf(log, "Variable '%s' not found in line %d: '%s'\n", i==0 ? s1 : s2, il, line[il]);
							goto invalid_prog;
						}
					}

					if (strcmp(ed->reg[arg_ir[0]].type, "i") != 0)	// s1 must be an integer variable
					{
						bufprintf(log, "Not an integer variable '%s' in line %d: '%s'\n", s1, il, line[il]);
						goto invalid_prog;
					}

					if (strcmp(ed->reg[arg_ir[1]].type, "l") != 0)	// s2 must be a location
					{
						bufprintf(log, "Not a location '%s' in line %d: '%s'\n", s2, il, line[il]);
						goto invalid_prog;
					}

					io = alloc_opcode(ed, 3);
					ed->op[io] = op_jmp_cond;
					ed->op[io+1] = ed->reg[arg_ir[0]].index;					// integer variable
					if (ed->loc[ed->reg[arg_ir[1]].index])
						ed->op[io+2] = (opint_t) ed->loc[ed->reg[arg_ir[1]].index] - io;	// signed backward jump offset
					else
					{
						ed->op[io+2] = arg_ir[1];	// in case of a forward jump the jump offset will have to be specified later
						fwd_jumps++;
					}
				}
				else
				{
					bufprintf(log, "Incorrect 'if <integer variable> goto <loc>' command in line %d: '%s'\n", il, line[il]);
					goto invalid_prog;
				}
			}

			if (strcmp(s0, "set0")==0)
			{
				sscanf(p, "%*s %30s", s1);
				ir = rlip_find_value(s1, ed);

				// Add opcode
				if (ir > -1)
				{
					if (strcmp(ed->reg[ir].type, "d")==0 || strcmp(ed->reg[ir].type, "i")==0)
					{
						io = alloc_opcode(ed, 2);
						ed->op[io] = ed->reg[ir].type[0]=='d' ? op_set0_v : op_set0_i;
						ed->op[io+1] = ed->reg[ir].index;
					}
					else
					{
						bufprintf(log, "Command 'set0' can't use type '%s' in line %d: '%s'\n", ed->reg[ir].type, il, line[il]);
						goto invalid_prog;
					}
				}
				else
				{
					bufprintf(log, "Value not found for command 'set0' in line %d: '%s'\n", il, line[il]);
					goto invalid_prog;
				}
			}

			if (strcmp(s0, "inc1")==0)
			{
				sscanf(p, "%*s %30s", s1);
				ir = rlip_find_value(s1, ed);

				// Add opcode
				if (ir > -1)
				{
					if (strcmp(ed->reg[ir].type, "d")==0 || strcmp(ed->reg[ir].type, "i")==0)
					{
						io = alloc_opcode(ed, 2);
						ed->op[io] = ed->reg[ir].type[0]=='d' ? op_inc1_v : op_inc1_i;
						ed->op[io+1] = ed->reg[ir].index;
					}
					else
					{
						bufprintf(log, "Command 'inc1' can't use type '%s' in line %d: '%s'\n", ed->reg[ir].type, il, line[il]);
						goto invalid_prog;
					}
				}
				else
				{
					bufprintf(log, "Value not found for command 'inc1' in line %d: '%s'\n", il, line[il]);
					goto invalid_prog;
				}
			}
		}
	}

	data.valid_prog = 1;

	// If not all forward jumps were resolved
	if (fwd_jumps != 0)
	{
		bufprintf(log, "There are %d unresolved forward jumps\n", fwd_jumps);
		data.valid_prog = 0;
	}

	// If the return command is missing
	if (ret_cmd_done==0)
	{
		bufprintf(log, "The 'return' command is missing or invalid\n");
invalid_prog:
		data.valid_prog = 0;
	}

	// Add end opcode
	io = alloc_opcode(ed, 1);
	ed->op[io] = op_end;

	// Free stuff
	free_2d(line, 1);

	for (i=0; i < ed->reg_count; i++)
	{
		free(ed->reg[i].name);
		free(ed->reg[i].type);
	}
	free(ed->loc);
	free(ed->reg);

	data.op = ed->op;
	return data;
}
