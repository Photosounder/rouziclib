void line_line_intersection(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4, double *x, double *y)
{
	*x = ((x1*y2-y1*x2)*(x3-x4)-(x1-x2)*(x3*y4-y3*x4)) / ((x1-x2)*(y3-y4)-(y1-y2)*(x3-x4));
	*y = ((x1*y2-y1*x2)*(y3-y4)-(y1-y2)*(x3*y4-y3*x4)) / ((x1-x2)*(y3-y4)-(y1-y2)*(x3-x4));
}

double point_line_distance(double x1, double y1, double x2, double y2, double x3, double y3)	// nearest point on the line
{
	double u, xp, yp, dist;

	u = hypot(x2-x1, y2-y1);
	u = ((x3-x1)*(x2-x1) + (y3-y1)*(y2-y1)) / (u * u);

	xp = x1 + u * (x2-x1);
	yp = y1 + u * (y2-y1);

	return hypot(xp-x3, yp-y3);
}

// Limits a line to the insides of a bounding box
void border_clip(double w, double h, double *x1, double *y1, double *x2, double *y2, double radius)
{
	double u;
	double bx1, by1, bx2, by2;	// coordinates of the bounding box

	bx1 = 0. - radius;	bx2 = w + radius;
	by1 = 0. - radius;	by2 = h + radius;

	if (*x1 < bx1)
	{
		if (*x1 - *x2)
		{
			u = (bx1 - *x2) / (*x1 - *x2);
			*y1 = u * (*y1-*y2) + *y2;
		}
		*x1 = bx1;
	}

	if (*x1 > bx2)
	{
		if (*x1 - *x2)
		{
			u = (bx2 - *x2) / (*x1 - *x2);
			*y1 = u * (*y1-*y2) + *y2;
		}
		*x1 = bx2;
	}

	if (*y1 < by1)
	{
		if (*y1 - *y2)
		{
			u = (by1 - *y2) / (*y1 - *y2);
			*x1 = u * (*x1-*x2) + *x2;
		}
		*y1 = by1;
	}

	if (*y1 > by2)
	{
		if (*y1 - *y2)
		{
			u = (by2 - *y2) / (*y1 - *y2);
			*x1 = u * (*x1-*x2) + *x2;
		}
		*y1 = by2;
	}

	if (*x2 < bx1)
	{
		if (*x2 - *x1)
		{
			u = (bx1 - *x1) / (*x2 - *x1);
			*y2 = u * (*y2-*y1) + *y1;
		}
		*x2 = bx1;
	}

	if (*x2 > bx2)
	{
		if (*x2 - *x1)
		{
			u = (bx2 - *x1) / (*x2 - *x1);
			*y2 = u * (*y2-*y1) + *y1;
		}
		*x2 = bx2;
	}

	if (*y2 < by1)
	{
		if (*y2 - *y1)
		{
			u = (by1 - *y1) / (*y2 - *y1);
			*x2 = u * (*x2-*x1) + *x1;
		}
		*y2 = by1;
	}

	if (*y2 > by2)
	{
		if (*y2 - *y1)
		{
			u = (by2 - *y1) / (*y2 - *y1);
			*x2 = u * (*x2-*x1) + *x1;
		}
		*y2 = by2;
	}
}
