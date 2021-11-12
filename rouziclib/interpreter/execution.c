int execute_bytecode(rlip_t *d)
{
	int inc = 1;
	uint64_t *op = d->op;
	double *vd = d->vd;
	int64_t *vi = d->vi;

	while (d->exec_on)
	{
		inc = 1;

		// Execute op
		switch (op[0])
		{
			// 1 word ops
			case op_end:	return 1;

			// 2 word ops
			break;	case op_ret_v:		d->return_value = vd[op[1]];					// return <var/ptr/expr>
			break;	case op_jmp:		op = &op[(ptrdiff_t) op[1]];	inc = 0;			// compiler-only
			break;	case op_set0_v:		vd[op[1]] = 0.;							// set0 <var>
			break;	case op_set0_i:		vi[op[1]] = 0;
			break;	case op_inc1_v:		vd[op[1]] += 1.;						// inc1 <var>
			break;	case op_inc1_i:		vi[op[1]] += 1;

			// 3 word ops
			break;	case op_load_v:		vd[op[1]] = *d->pd[op[2]];					// compiler-only
			break;	case op_load_i:		vi[op[1]] = *d->p_i[op[2]];					// compiler-only
			break;	case op_set_v:		vd[op[1]] = vd[op[2]];						// <var> = <var/ptr/expr>
			break;	case op_set_i:		vi[op[1]] = vi[op[2]];
			break;	case op_cvt_i_v:	vd[op[1]] = (double) vi[op[2]];					// <var> = <var/ptr/expr>
			break;	case op_cvt_v_i:	vi[op[1]] = (int64_t) nearbyint(vd[op[2]]);

			break;	case op_sq_v:		vd[op[1]] = sq(vd[op[2]]);					// <var> = sq <var/ptr/expr>
			break;	case op_sqrt_v:		vd[op[1]] = sqrt(vd[op[2]]);					// <var> = sqrt <var/ptr/expr>

			break;	case op_jmp_cond:	if (vi[op[1]])	op = &op[(ptrdiff_t) op[2]];	inc = 0;	// if <var> goto <loc>
			break;	case op_func0_v:	vd[op[1]] = ((double (*)(void))op[2])();

			// 4 word ops
			break;	case op_add_vv:		vd[op[1]] = vd[op[2]] + vd[op[3]];				// <var> = add <var/ptr/expr> <var/ptr/expr>
			break;	case op_add_ii:		vi[op[1]] = vi[op[2]] + vi[op[3]];				// <var> = addi <var/ptr/expr> <var/ptr/expr>
			break;	case op_sub_vv:		vd[op[1]] = vd[op[2]] - vd[op[3]];
			break;	case op_sub_ii:		vi[op[1]] = vi[op[2]] - vi[op[3]];
			break;	case op_mul_vv:		vd[op[1]] = vd[op[2]] * vd[op[3]];
			break;	case op_mul_ii:		vi[op[1]] = vi[op[2]] * vi[op[3]];
			break;	case op_div_vv:		vd[op[1]] = vd[op[2]] / vd[op[3]];
			break;	case op_div_ii:		vi[op[1]] = vi[op[2]] / vi[op[3]];
			break;	case op_mod_ii:		vi[op[1]] = vi[op[2]] % vi[op[3]];
			break;	case op_mod_vv:		vd[op[1]] = fmod(vd[op[2]], vd[op[3]]);
			break;	case op_pow_vv:		vd[op[1]] = pow(vd[op[2]], vd[op[3]]);

			break;	case op_cmp_vv_eq:	vi[op[1]] = (vd[op[2]] == vd[op[3]]);				// <var> = cmp <var/ptr/expr> == <var/ptr/expr>
			break;	case op_cmp_ii_eq:	vi[op[1]] = (vi[op[2]] == vi[op[3]]);				// <var> = cmpi <var/ptr/expr> == <var/ptr/expr>
			break;	case op_cmp_vv_ne:	vi[op[1]] = (vd[op[2]] != vd[op[3]]);				// <var> = cmp <var/ptr/expr> != <var/ptr/expr>
			break;	case op_cmp_ii_ne:	vi[op[1]] = (vi[op[2]] != vi[op[3]]);
			break;	case op_cmp_vv_lt:	vi[op[1]] = (vd[op[2]] <  vd[op[3]]);				// <var> = cmp <var/ptr/expr> < <var/ptr/expr>
			break;	case op_cmp_ii_lt:	vi[op[1]] = (vi[op[2]] <  vi[op[3]]);
			break;	case op_cmp_vv_le:	vi[op[1]] = (vd[op[2]] <= vd[op[3]]);				// <var> = cmp <var/ptr/expr> <= <var/ptr/expr>
			break;	case op_cmp_ii_le:	vi[op[1]] = (vi[op[2]] <= vi[op[3]]);
			break;	case op_cmp_vv_gt:	vi[op[1]] = (vd[op[2]] >  vd[op[3]]);				// <var> = cmp <var/ptr/expr> > <var/ptr/expr>
			break;	case op_cmp_ii_gt:	vi[op[1]] = (vi[op[2]] >  vi[op[3]]);
			break;	case op_cmp_vv_ge:	vi[op[1]] = (vd[op[2]] >= vd[op[3]]);				// <var> = cmp <var/ptr/expr> >= <var/ptr/expr>
			break;	case op_cmp_ii_ge:	vi[op[1]] = (vi[op[2]] >= vi[op[3]]);
			break;	case op_func1_vv:	vd[op[1]] = ((double (*)(double))op[2])(vd[op[3]]);		// <var> = <func> <var/ptr/expr>

			// 5 word ops
			break;	case op_func2_vvv:	vd[op[1]] = ((double (*)(double,double))op[2])(vd[op[3]], vd[op[4]]);

			// 6 word ops
			break;	case op_func3_vvvv:	vd[op[1]] = ((double (*)(double,double,double))op[2])(vd[op[3]], vd[op[4]], vd[op[5]]);

			// 7 word ops
		}

		// Increment to next op
		if (inc)
			op = &op[op[0] >> 10];
	}

	return 0;
}
