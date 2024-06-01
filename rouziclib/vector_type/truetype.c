#ifdef RL_TRUETYPE

#undef pi
#define optimize ttf_optimize
#define FAILED TTF_FAILED
#define TTF_NO_FILESYSTEM
#define TTF_NO_SIGNAL_H
#include "../libraries/orig/ttf2mesh.c"
#undef optimize
#undef FAILED
#undef pi
#define pi RL_PI

vector_font_t *load_truetype_font_as_triangles(const uint8_t *data, size_t size, xyi_t *range, int range_count, int quality)
{
	// Load TTF file
	ttf_t *ttf;
	if (ttf_load_from_mem(data, size, &ttf, 0) != TTF_DONE)
		return NULL;

	vector_font_t *font = init_font();

	// Find the ascender height to calculate the scale
	double scale = 6. / (double) ttf->os2.sTypoAscender;
	for (int ic=0; ic < ttf->nchars; ic++)
	{
		if (ttf->chars[ic] == 'H')
		{
			scale = 6. / (double) ttf->glyphs[ttf->char2glyph[ic]].ybounds[1];
			break;
		}
	}

	// Typesetting parameters
	font->letter_spacing = 0.;
	font->line_vspacing = scale * (ttf->os2.sTypoLineGap + ttf->os2.sTypoAscender - ttf->os2.sTypoDescender);

	for (int ic=0; ic < ttf->nchars; ic++)
	for (int ir=0; ir < range_count; ir++)
	{
		uint32_t cp = ttf->chars[ic];
		if (cp >= range[ir].x && cp <= range[ir].y)
		{
			ttf_glyph_t *g = &ttf->glyphs[ttf->char2glyph[ic]];

			// Alloc new letter in the vector font
			font_create_letter(font, cp);
			letter_t *l = &font->l[font->letter_count-1];

			// Set bounds
			l->bl = 0.;
			l->br = scale * g->advance;
			l->width = l->br - l->bl;
			l->tri_mesh.count = -1;

			// Turn the outline into a mesh of triangles
			if (g->outline && g->outline->total_points >= 3)
			{
				ttf_mesh_t *m;
				ttf_glyph2mesh(g, &m, quality, 0);

				if (m)
				{
					l->tri_mesh.count = m->nfaces;
					l->tri_mesh.tri = calloc(l->tri_mesh.count, sizeof(triangle_t));
					l->tri_mesh.tri_bound = calloc(l->tri_mesh.count, sizeof(rect_t));

					// Set the triangles
					for (int it=0; it < l->tri_mesh.count; it++)
					{
						l->tri_mesh.tri[it] = mul_triangle(triangle(
								xy(m->vert[m->faces[it].v1].x, m->vert[m->faces[it].v1].y), 
								xy(m->vert[m->faces[it].v2].x, m->vert[m->faces[it].v2].y), 
								xy(m->vert[m->faces[it].v3].x, m->vert[m->faces[it].v3].y)), set_xy(scale));
						l->tri_mesh.tri_bound[it].p0 = min_xy(l->tri_mesh.tri[it].a, min_xy(l->tri_mesh.tri[it].b, l->tri_mesh.tri[it].c));
						l->tri_mesh.tri_bound[it].p1 = max_xy(l->tri_mesh.tri[it].a, max_xy(l->tri_mesh.tri[it].b, l->tri_mesh.tri[it].c));
					}

					ttf_free_mesh(m);

					// Calculate mesh bounds
					l->tri_mesh.bound.p0 = l->tri_mesh.tri_bound[0].p0;
					l->tri_mesh.bound.p1 = l->tri_mesh.tri_bound[0].p1;
					for (int it=1; it < l->tri_mesh.count; it++)
					{
						l->tri_mesh.bound.p0 = min_xy(l->tri_mesh.bound.p0, l->tri_mesh.tri_bound[it].p0);
						l->tri_mesh.bound.p1 = max_xy(l->tri_mesh.bound.p1, l->tri_mesh.tri_bound[it].p1);
					}
				}
			}
		}
	}

        ttf_free(ttf);

	return font;
}

polynomial_grid_t mesh_to_polynomial(vobj_tri_t *mesh, double radius, double max_grid_step, int degree)
{
	polynomial_grid_t grid = {0};
	xyi_t ic, id, ip, p_count;
	xy_t pos;

	// Alloc cell samples and node positions
	p_count = set_xyi(degree + 1);
	double **z = (double **) calloc_2d_contig(p_count.y, p_count.x, sizeof(double));
	xy_t **node = (xy_t **) calloc_2d_contig(p_count.y, p_count.x, sizeof(xy_t));

	// Node scaling
	// 1 is normal, 1/chebyshev_node(degree+1, 0) spreads out the nodes to make cells seamless
	xy_t node_scale = XY1;
	//xy_t node_scale = inv_xy(xy(chebyshev_node(degree+1, 0.), chebyshev_node(degree+1, 0.)));

	// Calculate cell-relative node positions
	for (ip.y=0; ip.y < p_count.y; ip.y++)
		for (ip.x=0; ip.x < p_count.x; ip.x++)
			node[ip.y][ip.x] = mad_xy(mul_xy(node_scale, xy(chebyshev_node(p_count.x, ip.x), chebyshev_node(p_count.y, ip.y))), set_xy(0.5), set_xy(0.5));

	// Calculate grid and cell dimensions
	xy_t rad_margin = set_xy(radius * 3.);
	grid.bound = rect_add_margin(mesh->bound, rad_margin);
	grid.dim = xy_to_xyi(ceil_xy(div_xy(get_rect_dim(grid.bound), set_xy(max_grid_step))));
	grid.step = div_xy(get_rect_dim(grid.bound), xyi_to_xy(grid.dim));
	grid.inv_step = inv_xy(grid.step);
	grid.cell = (polynomial_cell_t **) calloc_2d_contig(grid.dim.y, grid.dim.x, sizeof(polynomial_cell_t));
	xy_t span = mul_xy(grid.step, mul_xy(node_scale, set_xy(0.5)));

	// Precalculate polynomial coefs for each input sample
	double *sample_coefs = calloc(mul_x_by_y_xyi(p_count) * mul_x_by_y_xyi(set_xyi(degree + 1)), sizeof(double));
	for (int i=0; i < mul_x_by_y_xyi(p_count); i++)
	{
		z[0][i] = 1.;
		double **c = polynomial_fit_on_points_by_dct_2d(z, p_count, neg_xy(span), span, NULL, set_xyi(degree));
		memcpy(&sample_coefs[i * mul_x_by_y_xyi(set_xyi(degree + 1))], *c, mul_x_by_y_xyi(set_xyi(degree + 1)) * sizeof(double));
		free_2d(c, 1);
		z[0][i] = 0.;
	}

	// Go through every cell
	for (ic.y=0; ic.y < grid.dim.y; ic.y++)
		for (ic.x=0; ic.x < grid.dim.x; ic.x++)
		{
			xy_t start = mad_xy(xyi_to_xy(ic), grid.step, grid.bound.p0);
			memset(z[0], 0, mul_x_by_y_xyi(p_count) * sizeof(double));

			// Compute the points by going through every triangle for every point of the cell
			rect_t node_lim = rect_add_margin(rect(mad_xy(node[0][0], grid.step, start), mad_xy(node[p_count.y - 1][p_count.x - 1], grid.step, start)), rad_margin);
			for (int it=0; it < mesh->count; it++)
				if (check_box_box_intersection(mesh->tri_bound[it], node_lim))
				{
					rect_t tri_bound_pad = rect_add_margin(mesh->tri_bound[it], rad_margin);

					for (ip.y=0; ip.y < p_count.y; ip.y++)
					{
						pos.y = node[ip.y][0].y * grid.step.y + start.y;
						if (tri_bound_pad.p0.y < pos.y && pos.y < tri_bound_pad.p1.y)
							for (ip.x=0; ip.x < p_count.x; ip.x++)
							{
								pos.x = node[ip.y][ip.x].x * grid.step.x + start.x;
								if (tri_bound_pad.p0.x < pos.x && pos.x < tri_bound_pad.p1.x)
									z[ip.y][ip.x] -= eval_polygon_at_pos((xy_t *) &mesh->tri[it], 3, radius, pos);
							}
					}
				}

			// Sqrt the samples
			for (ip.y=0; ip.y < p_count.y; ip.y++)
				for (ip.x=0; ip.x < p_count.x; ip.x++)
					z[ip.y][ip.x] = sqrt(MAXN(0., z[ip.y][ip.x]));

			// Fit points into polynomial coefs
			polynomial_cell_t *cell = &grid.cell[ic.y][ic.x];
			cell->bound = make_rect_off(start, grid.step, XY0);
			cell->eval_offset = get_rect_centre(cell->bound);
			cell->degree = set_xyi(degree);

			// Alloc in coef array
			cell->array_index = grid.coef_count;
			alloc_enough(&grid.coef, grid.coef_count += mul_x_by_y_xyi(add_xyi(cell->degree, XYI1)), &grid.coef_as, sizeof(double), 1.4);

			// Turn input samples into coefficients (only works if cell->degree = set_xyi(degree);)
			for (id.y=0; id.y <= degree; id.y++)
				for (id.x=0; id.x <= degree; id.x++)
				{
					double sum = 0.;
					for (ip.y=0; ip.y < p_count.y; ip.y++)
						for (ip.x=0; ip.x < p_count.x; ip.x++)
							sum += z[ip.y][ip.x] * sample_coefs[((ip.y * p_count.x + ip.x) * (degree+1) + id.y) * (degree+1) + id.x];
					grid.coef[cell->array_index + id.y * (cell->degree.x+1) + id.x] = sum;
				}
		}

	free(sample_coefs);
	free_2d(z, 1);
	free_2d(node, 1);
	grid.coef_as = grid.coef_count;
	grid.coef = realloc(grid.coef, grid.coef_as * sizeof(double));

	return grid;
}

void font_gen_polynomial_grids(vector_font_t *font, double asc_height_px, double radius)
{
	letter_t *l;
	double scaled_radius = radius * 6. / asc_height_px;

	for (int il=0; il < font->letter_count; il++)
	{
		l = &font->l[il];

		if (l->tri_mesh.tri)
			l->polynomial_grid = mesh_to_polynomial(&l->tri_mesh, scaled_radius, scaled_radius*0.8, 2);

		/*
plot v0 = sqrt(erfr(x))
fit sqrt(erfr(t))
plot v1 = v0^2
plot v3 = v0^2 + (poly(x)^2-v0^2)*10

			err:	1/200
			Deg 1	0.400 @ 0.379	3.90"
			Deg 2	1.222 @-0.343	1.96"
			Deg 3	1.705 @ 0.246	1.77"
			Deg 4	2.376 @-0.329	2.08"
			Deg 5	
			Deg 6	
			Deg 9	
		   */
	}
}

#endif

double eval_polynomial_2d_flat_array(xy_t p, double *c, xyi_t degree)
{
	double sum=0., sum_line=0.;
	xyi_t id;

	for (id.y=degree.y; id.y >= 0; id.y--)
	{
		sum_line = 0.;

		for (id.x=degree.x; id.x > 0; id.x--)
			sum_line = (sum_line + c[id.y*(degree.x+1) + id.x]) * p.x;
		sum_line += c[id.y*(degree.x+1) + 0];

		sum = sum * p.y + sum_line;
	}

	return sum;
}

void draw_polynomial_grid_lrgb(polynomial_grid_t *grid, xy_t pos, double scale, double angle, lrgb_t colour, const blend_func_t bf)
{
	xyi_t ip, ic;
	xy_t p;
	double pix;
	rect_t area = sort_rect(mad_rect_xy(grid->bound, neg_y(set_xy(scale)), pos));
	recti_t bound = recti(xy_to_xyi(ceil_xy(area.p0)), xy_to_xyi(area.p1));
	bound = recti_intersection(bound, recti(XYI0, sub_xyi(fb->r.dim, XYI1)));

	xy_t inv_scale = neg_y(set_xy(scale)), inv_offset = pos;
	inverse_scale_offset(&inv_scale, &inv_offset);

	for (ip.y = bound.p0.y; ip.y <= bound.p1.y; ip.y++)
	{
		p.y = (double) ip.y * inv_scale.y + inv_offset.y;

		for (ip.x = bound.p0.x; ip.x <= bound.p1.x; ip.x++)
		{
			p.x = (double) ip.x * inv_scale.x + inv_offset.x;

			ic = xy_to_xyi(mul_xy(sub_xy(p, grid->bound.p0), grid->inv_step));

			ic = rangelimit_xyi(ic, XYI0, sub_xyi(grid->dim, XYI1));

			polynomial_cell_t *cell = &grid->cell[ic.y][ic.x];
			pix = eval_polynomial_2d_flat_array(sub_xy(p, cell->eval_offset), &grid->coef[cell->array_index], cell->degree);
			pix *= pix;

			bf(&fb->r.l[ip.y*fb->r.dim.x + ip.x], colour, pix * ONEF + 0.5);
		}
	}
}

void draw_polynomial_grid(polynomial_grid_t *grid, xy_t pos, double scale, double angle, col_t colour, const blend_func_t bf)
{
	if (fb->use_drawq)
		if (fb->use_dqnq)
			return;
		else
			return;
	else if (fb->r.use_frgb)
		return;
	else
		draw_polynomial_grid_lrgb(grid, pos, scale, angle, col_to_lrgb(colour), bf);
}
