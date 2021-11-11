enum opcode_table
{
	table_none=0,
	table_vd,
	table_vi,
	table_pd,
	table_pi,
};

typedef struct 
{
	size_t op_count, op_as, vd_count, vd_as, vi_count, vi_as, pd_count, pd_as, pi_count, pi_as, reg_count, reg_as;
} rlip_counts_t;

typedef struct 
{
	char *name;
	uint64_t hash;
	char *type;
	enum opcode_table table;	// table is 0 for not in a table (like function pointers), 1 for vd, 2 for vi, 3 for pd, 4 for p_i
	int index;			// index is the index in the given table
} rlip_reg_t;

int rlip_find_value(const char *name, rlip_counts_t *counts, rlip_reg_t *reg)
{
	int i;
	uint64_t hash;

	if (name==NULL)
		return -1;

	hash = get_string_hash(name);

	// Look for existing instance of the value
	for (i=0; i < counts->reg_count; i++)
		if (hash == reg[i].hash)
			if (strcmp(name, reg[i].name)==0)
				return i;			// return reg index

	return -1;
}

void rlip_add_value_at_reg_index(int ir, const char *name, const void *ptr, const char *type, rlip_t *d, rlip_counts_t *counts, rlip_reg_t *reg)
{
	reg[ir].name = make_string_copy(name);				// name can be NULL in which case this is a nameless variable used only by the compiler for intermediary values
	if (name)
		reg[ir].hash = get_string_hash(name);
	reg[ir].type = make_string_copy(type);

	if (strcmp(type, "d")==0)		// add to vd table
	{
		reg[ir].table = table_vd;
		reg[ir].index = counts->vd_count;
		alloc_enough(&d->vd, counts->vd_count+=1, &counts->vd_as, sizeof(double), 1.5);
		if (ptr)
			d->vd[reg[ir].index] = *(double *) ptr;
	}

	else if (strcmp(type, "i")==0)		// add to vi table
	{
		reg[ir].table = table_vi;
		reg[ir].index = counts->vi_count;
		alloc_enough(&d->vi, counts->vi_count+=1, &counts->vi_as, sizeof(int64_t), 1.5);
		if (ptr)
			d->vi[reg[ir].index] = *(int64_t *) ptr;
	}

	else if (strcmp(type, "pd")==0)		// add to pd table
	{
		reg[ir].table = table_pd;
		reg[ir].index = counts->pd_count;
		alloc_enough((void **) &d->pd, counts->pd_count+=1, &counts->pd_as, sizeof(double *), 1.5);
		d->pd[reg[ir].index] = (double *) ptr;
	}

	else if (strcmp(type, "pi")==0)		// add to p_i table
	{
		reg[ir].table = table_pi;
		reg[ir].index = counts->pi_count;
		alloc_enough((void **) &d->p_i, counts->pi_count+=1, &counts->pi_as, sizeof(int64_t *), 1.5);
		d->p_i[reg[ir].index] = (int64_t *) ptr;
	}
}

int rlip_add_value(const char *name, const void *ptr, const char *type, rlip_t *d, rlip_counts_t *counts, rlip_reg_t **reg)
{
	int ir;

	// Look for existing instance of the value
	ir = rlip_find_value(name, counts, *reg);
	if (ir > -1)
		return ir;

	// Add new registry entry
	ir = counts->reg_count;
	alloc_enough(reg, counts->reg_count+=1, &counts->reg_as, sizeof(rlip_reg_t), 1.5);

	rlip_add_value_at_reg_index(ir, name, ptr, type, d, counts, *reg);

	return ir;
}

size_t alloc_opcode(rlip_t *d, rlip_counts_t *counts, size_t add_count)
{
	size_t index = counts->op_count;
	alloc_enough(&d->op, counts->op_count+=add_count, &counts->op_as, sizeof(uint64_t), 1.5);
	return index;
}

void prepend_opcode(rlip_t *d, rlip_counts_t *counts, size_t add_count)
{
	size_t move_count = alloc_opcode(d, counts, add_count);			// make room for the ops to prepend
	memmove(&d->op[add_count], d->op, move_count * sizeof(d->op[0]));	// move all ops to make room for the new ones at the beginning
}

void convert_pointer_to_variable(int ir, rlip_t *d, rlip_counts_t *counts, rlip_reg_t *reg)
{
	// Convert a pointer to a variable forever
	if (reg[ir].type[0] == 'p')
	{
		// Prepend load opcode
		prepend_opcode(d, counts, 3);
		d->op[0] = reg[ir].type[1]=='d' ? op_load_v : op_load_i;
		d->op[2] = reg[ir].index;

		// Convert reg entry from pointer to variable
		rlip_reg_t old_entry = reg[ir];
		rlip_add_value_at_reg_index(ir, old_entry.name, NULL, old_entry.type[1]=='d' ? "d" : "i", d, counts, reg);
		free(old_entry.name);
		free(old_entry.type);

		// Set destination
		d->op[1] = reg[ir].index;
	}
}

void convert_expression_to_variable(const char *name, rlip_t *d, rlip_counts_t *counts, rlip_reg_t **reg)
{
	double v = te_interp(name, NULL);				// interpret name as an expression

	if (isnan(v)==0)						// if it's a valid expression
		rlip_add_value(name, &v, "d", d, counts, reg);	// add the value as a variable with the original expression as its name
}

int rlip_find_convert_value(const char *name, rlip_t *d, rlip_counts_t *counts, rlip_reg_t **reg)
{
	int ir;

	// It might be an expression, in which case make it a variable
	convert_expression_to_variable(name, d, counts, reg);

	ir = rlip_find_value(name, counts, *reg);

	if (ir > -1)
		// It might be a pointer, in which case make it a variable permanently
		convert_pointer_to_variable(ir, d, counts, *reg);

	return ir;
}

rlip_t rlip_compile(const char *source, rlip_inputs_t *inputs, int input_count, buffer_t *log)
{
	int i, io, ir, il, ret, linecount, n, add_var_type, dest_ir, ret_cmd_done=0, cmd_found;
	int rd[8], ri[8];	// generic registers
	char **line = arrayise_text(make_string_copy(source), &linecount);
	rlip_t data={0}, *d=&data;
	rlip_counts_t counts={0};
	rlip_reg_t *reg=NULL;
	char *p, a[32], b[32], c[32], cmd_arg_type[16];
	int arg_ir[16];
	uint64_t hash;

	// Add nameless variables as generic registers to structure
	for (i=0; i < sizeof(rd)/sizeof(*rd); i++)
		rd[i] = reg[rlip_add_value(NULL, NULL, "d", d, &counts, &reg)].index;

	for (i=0; i < sizeof(ri)/sizeof(*ri); i++)
		ri[i] = reg[rlip_add_value(NULL, NULL, "i", d, &counts, &reg)].index;

	// Add inputs to structure
	for (i=0; i < input_count; i++)
		rlip_add_value(inputs[i].name, inputs[i].ptr, inputs[i].type, d, &counts, &reg);

	// Parse lines and compile them into opcodes
	for (il=0; il < linecount; il++)
	{
		add_var_type = 0;
		p = line[il];

line_proc_start:
		a[0] = '\0';
		b[0] = '\0';
		c[0] = '\0';
		n = 0;
		sscanf(p, "%30s = %n", a, &n);

		// Declaring a new variable by its type
		if (strcmp(a, "d")==0 || strcmp(a, "i")==0)
		{
			// Set flag for creation of variable
			if (a[0]=='d')
				add_var_type = table_vd;
			else
				add_var_type = table_vi;

			// Jump back to end up at the processing of "<var> = ..."
			sscanf(p, "%*s%n", &n);
			p = &p[n];

			goto line_proc_start;
		}

		// When doing "<var> = ..."
		else if (n)
		{
			p = &p[n];

			// Add or find destination var
			if (add_var_type)
				dest_ir = rlip_add_value(a, NULL, add_var_type==table_vd ? "d" : "i", d, &counts, &reg);
			else
				dest_ir = rlip_find_value(a, &counts, reg);
			
			if (dest_ir == -1)
			{
				bufprintf(log, "Undeclared variable '%s' used in line %d: '%s'\n", a, il, line[il]);
				goto invalid_prog;
			}

			// Read command
			ret = sscanf(p, "%30s %n", a, &n);
			cmd_found = 0;

			if (ret == 1)
			{
				int cmd_arg_count = 0;

				// Identify the command and find its argument count
				if (	strcmp(a, "sq")==0 ||
					strcmp(a, "sqrt")==0 )
				{
					cmd_arg_count = 1;
					sprintf(cmd_arg_type, "dd");
					cmd_found = 1;
				}

				if (	strcmp(a, "add")==0 ||
					strcmp(a, "sub")==0 ||
					strcmp(a, "mul")==0 ||
					strcmp(a, "div")==0 ||
					strcmp(a, "mod")==0 ||
					strcmp(a, "pow")==0 )
				{
					cmd_arg_count = 2;
					sprintf(cmd_arg_type, "ddd");
					cmd_found = 1;
				}

				if (	strcmp(a, "addi")==0 ||
					strcmp(a, "subi")==0 ||
					strcmp(a, "muli")==0 ||
					strcmp(a, "divi")==0 ||
					strcmp(a, "modi")==0 )
				{
					cmd_arg_count = 2;
					sprintf(cmd_arg_type, "iii");
					cmd_found = 1;
				}

				// Add command
				if (cmd_found == 1)
				{
					// Go through arguments to convert them and determine their types
					for (i=0; i < cmd_arg_count; i++)
					{
						p = &p[n];
						n = 0;
						ret = sscanf(p, "%30s %n", b, &n);

						if (ret)
						{
							arg_ir[i] = rlip_find_convert_value(b, d, &counts, &reg);

							if (arg_ir[i] == -1)
							{
								bufprintf(log, "Argument '%s' unidentified in line %d: '%s'\n", a, il, line[il]);
								goto invalid_prog;
							}

							// Convert the variable to a generic register if the type doesn't match the one expected
							if (reg[arg_ir[i]].type[0] != cmd_arg_type[i])
							{
								io = alloc_opcode(d, &counts, 3);	// add conversion opcode
								d->op[io+2] = reg[arg_ir[i]].index;	// source index

								if (cmd_arg_type[i] == 'd')
								{
									d->op[io] = op_cvt_i_v;
									d->op[io+1] = rd[1+i];		// destinaton index
								}
								else
								{
									d->op[io] = op_cvt_v_i;
									d->op[io+1] = ri[1+i];
								}

								arg_ir[i] = d->op[io+1];	// the ir for this argument is now the generic register
							}
						}
						else
						{
							bufprintf(log, "Argument missing (%d arguments expected) in line %d: '%s'\n", cmd_arg_count, il, line[il]);
							goto invalid_prog;
						}
					}

					// Add opcode
					io = alloc_opcode(d, &counts, 2+cmd_arg_count);

					if (strcmp(a, "sq")==0)		d->op[io] = op_sq_v;
					else if (strcmp(a, "sqrt")==0)	d->op[io] = op_sqrt_v;
					else if (strcmp(a, "add")==0)	d->op[io] = op_add_vv;
					else if (strcmp(a, "addi")==0)	d->op[io] = op_add_ii;
					else if (strcmp(a, "mul")==0)	d->op[io] = op_mul_vv;
					else if (strcmp(a, "muli")==0)	d->op[io] = op_mul_ii;
					// TODO

					// Add arguments to opcode
					for (i=0; i < cmd_arg_count; i++)
						d->op[io+2+i] = reg[arg_ir[i]].index;

					// Destination (assuming same type)
					d->op[io+1] = reg[dest_ir].index;

					// Convert result to destination type
					if (reg[dest_ir].type[0] != cmd_arg_type[0])
					{
						// Output to generic register
						d->op[io+1] = cmd_arg_type[0]=='d' ? rd[0] : ri[0];

						// Convert from generic register to destination
						io = alloc_opcode(d, &counts, 3);	// add conversion opcode
						d->op[io+1] = reg[dest_ir].index;	// destination index

						if (cmd_arg_type[0] == 'd')
						{
							d->op[io] = op_cvt_v_i;
							d->op[io+2] = rd[0];		// source index
						}
						else
						{
							d->op[io] = op_cvt_i_v;
							d->op[io+2] = ri[0];
						}
					}
				}

				// Check if it's a variable, pointer or expression rather than a command making "=" a set or cvt command
				if (cmd_found == 0)
				{
					ir = rlip_find_convert_value(a, d, &counts, &reg);	// converts pointers and expressions to variables
					if (ir > -1)
					{
						// Variable
						if (strcmp(reg[ir].type, "d")==0 || strcmp(reg[ir].type, "i")==0)
						{
							io = alloc_opcode(d, &counts, 3);

							if (reg[dest_ir].table == reg[ir].table)
								d->op[io] = reg[ir].type[0]=='d' ? op_set_v : op_set_i;
							else
								d->op[io] = reg[ir].type[0]=='d' ? op_cvt_v_i : op_cvt_i_v;

							d->op[io+1] = reg[dest_ir].index;
							d->op[io+2] = reg[ir].index;
							cmd_found = 2;
						}
					}
				}

				if (cmd_found == 0)
				{
					bufprintf(log, "Unidentified '%s' in line %d: '%s'\n", a, il, line[il]);
					goto invalid_prog;
				}
			}
		}
		else
		{
			// 2 word ops

			if (strcmp(a, "return")==0)
			{
				sscanf(p, "return %30s", b);
				ir = rlip_find_convert_value(b, d, &counts, &reg);

				// Add return opcode
				if (ir > -1)
				{
					if (strcmp(reg[ir].type, "d")==0)
					{
						// Add return opcode
						io = alloc_opcode(d, &counts, 2);
						d->op[io] = op_ret_v;
						d->op[io+1] = reg[ir].index;
						ret_cmd_done = 1;
					}
					else
					{
						// Convert from integer to double
						io = alloc_opcode(d, &counts, 3);	// add conversion opcode
						d->op[io] = op_cvt_i_v;
						d->op[io+1] = rd[0];			// convert to generic register
						d->op[io+2] = reg[ir].index;

						// Add return opcode
						io = alloc_opcode(d, &counts, 2);
						d->op[io] = op_ret_v;
						d->op[io+1] = rd[0];
						ret_cmd_done = 1;
					}
				}
				else
				{
					bufprintf(log, "Value not found for command 'return' in line %d: '%s'\n", il, line[il]);
					goto invalid_prog;
				}
			}

			if (strcmp(a, "set0")==0)
			{
				sscanf(p, "set0 %30s", b);
				ir = rlip_find_value(b, &counts, reg);

				// Add opcode
				if (ir > -1)
				{
					if (strcmp(reg[ir].type, "d")==0 || strcmp(reg[ir].type, "i")==0)
					{
						io = alloc_opcode(d, &counts, 2);
						d->op[io] = reg[ir].type[0]=='d' ? op_set0_v : op_set0_i;
						d->op[io+1] = reg[ir].index;
					}
					else
					{
						bufprintf(log, "Command 'set0' can't use type '%s' in line %d: '%s'\n", reg[ir].type, il, line[il]);
						goto invalid_prog;
					}
				}
				else
				{
					bufprintf(log, "Value not found for command 'set0' in line %d: '%s'\n", il, line[il]);
					goto invalid_prog;
				}
			}

			if (strcmp(a, "inc1")==0)
			{
				sscanf(p, "inc1 %30s", b);
				ir = rlip_find_value(b, &counts, reg);

				// Add opcode
				if (ir > -1)
				{
					if (strcmp(reg[ir].type, "d")==0 || strcmp(reg[ir].type, "i")==0)
					{
						io = alloc_opcode(d, &counts, 2);
						d->op[io] = reg[ir].type[0]=='d' ? op_inc1_v : op_inc1_i;
						d->op[io+1] = reg[ir].index;
					}
					else
					{
						bufprintf(log, "Command 'inc1' can't use type '%s' in line %d: '%s'\n", reg[ir].type, il, line[il]);
						goto invalid_prog;
					}
				}
				else
				{
					bufprintf(log, "Value not found for command 'inc1' in line %d: '%s'\n", il, line[il]);
					goto invalid_prog;
				}
			}

			// 3 word ops
		}
	}

	d->valid_prog = 1;

	// If the return command is missing
	if (ret_cmd_done==0)
	{
		bufprintf(log, "The 'return' command is missing or invalid\n");
invalid_prog:
		d->valid_prog = 0;
	}

	// Add end opcode
	io = alloc_opcode(d, &counts, 1);
	d->op[io] = op_end;

	// Free stuff
	free_2d(line, 1);
	for (i=0; i < counts.reg_count; i++)
	{
		free(reg[i].name);
		free(reg[i].type);
	}
	free(reg);

	return data;
}

void free_rlip(rlip_t *prog)
{
	free(prog->op);
	free(prog->vd);
	free(prog->vi);
	free(prog->pd);
	free(prog->p_i);
}
