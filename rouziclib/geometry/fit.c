double fit_n_squares_in_area(xy_t area_dim, int count, xyi_t *grid_count_ptr)
{
	double n, size, max_size=0.;
	xy_t iv, grid_count, g, d1;

	if (count==1)
	{
		*grid_count_ptr = xyi(1, 1);
		return min_of_xy(area_dim);
	}

	n = (double) count;
	size = sqrt(area_dim.x*area_dim.y / n);
	grid_count = floor_xy(d1=div_xy(area_dim, set_xy(size)));	// starting point with minimum size

	for (iv.x=0.; iv.x <= grid_count.x; iv.x+=1.)
		for (iv.y=0.; iv.y <= grid_count.y; iv.y+=1.)
		{
			g = add_xy(grid_count, iv);
			if (g.x*g.y >= n)
			{
				size = min_of_xy(div_xy(area_dim, g));
				if (size > max_size)
				{
					max_size = size;
					if (grid_count_ptr)
						*grid_count_ptr = xy_to_xyi(g);
				}
			}
		}

	return max_size;
}

double fit_n_squares_in_area_fill_x(xy_t area_dim, int count, xyi_t *grid_count_ptr)
{
	double n, size, max_size=0.;
	xy_t iv, grid_count, g, d1;

	if (count==1 || min_of_xy(area_dim)==0.)
	{
		*grid_count_ptr = xyi(1, 1);
		return min_of_xy(area_dim);
	}

	n = (double) count;
	size = sqrt(area_dim.x*area_dim.y / n);
	grid_count = floor_xy(d1=div_xy(area_dim, set_xy(size)));	// starting point with minimum size
	grid_count = max_xy(grid_count, xy(1., 1.));

	for (iv.x=0.; iv.x <= grid_count.x; iv.x+=1.)
		for (iv.y=0.; iv.y <= grid_count.y; iv.y+=1.)
		{
			g = add_xy(grid_count, iv);
			if (g.x*g.y >= n)
			{
				size = min_of_xy(div_xy(area_dim, g));

				if (size * g.x >= double_add_ulp(area_dim.x, -40))
				if (size > max_size)
				{
					max_size = size;
					if (grid_count_ptr)
						*grid_count_ptr = xy_to_xyi(g);
				}
			}
		}

	return max_size;
}

xy_t fit_n_rects_in_area_fill_x(xy_t area_dim, xy_t rect_dim, int count, xyi_t *grid_count_ptr)
{
	double unit_size_sq = fit_n_squares_in_area_fill_x(div_xy(area_dim, rect_dim), count, grid_count_ptr);

	return mul_xy(set_xy(unit_size_sq), rect_dim);
}
