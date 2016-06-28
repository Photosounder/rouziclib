void line_line_intersection(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4, double *x, double *y)
{
	*x = ((x1*y2-y1*x2)*(x3-x4)-(x1-x2)*(x3*y4-y3*x4)) / ((x1-x2)*(y3-y4)-(y1-y2)*(x3-x4));
	*y = ((x1*y2-y1*x2)*(y3-y4)-(y1-y2)*(x3*y4-y3*x4)) / ((x1-x2)*(y3-y4)-(y1-y2)*(x3-x4));
}

double point_line_distance(double x1, double y1, double x2, double y2, double x3, double y3)	// nearest point on the line
{
	double u, xp, yp;

	u = hypot(x2-x1, y2-y1);
	u = ((x3-x1)*(x2-x1) + (y3-y1)*(y2-y1)) / (u * u);

	xp = x1 + u * (x2-x1);
	yp = y1 + u * (y2-y1);

	return hypot(xp-x3, yp-y3);
}

// Limits a line to the insides of a bounding box
// if the line is entirely outside then x1==x2 and y1==y2
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

// Limits a line to the insides of a bounding rectangle
void line_rect_clip(xy_t *l1, xy_t *l2, xy_t b1, xy_t b2)
{
	double u;

	if (l1->x < b1.x)
	{
		if (l1->x - l2->x)
		{
			u = (b1.x - l2->x) / (l1->x - l2->x);
			l1->y = u * (l1->y-l2->y) + l2->y;
		}
		l1->x = b1.x;
	}

	if (l1->x > b2.x)
	{
		if (l1->x - l2->x)
		{
			u = (b2.x - l2->x) / (l1->x - l2->x);
			l1->y = u * (l1->y-l2->y) + l2->y;
		}
		l1->x = b2.x;
	}

	if (l1->y < b1.y)
	{
		if (l1->y - l2->y)
		{
			u = (b1.y - l2->y) / (l1->y - l2->y);
			l1->x = u * (l1->x-l2->x) + l2->x;
		}
		l1->y = b1.y;
	}

	if (l1->y > b2.y)
	{
		if (l1->y - l2->y)
		{
			u = (b2.y - l2->y) / (l1->y - l2->y);
			l1->x = u * (l1->x-l2->x) + l2->x;
		}
		l1->y = b2.y;
	}

	if (l2->x < b1.x)
	{
		if (l2->x - l1->x)
		{
			u = (b1.x - l1->x) / (l2->x - l1->x);
			l2->y = u * (l2->y-l1->y) + l1->y;
		}
		l2->x = b1.x;
	}

	if (l2->x > b2.x)
	{
		if (l2->x - l1->x)
		{
			u = (b2.x - l1->x) / (l2->x - l1->x);
			l2->y = u * (l2->y-l1->y) + l1->y;
		}
		l2->x = b2.x;
	}

	if (l2->y < b1.y)
	{
		if (l2->y - l1->y)
		{
			u = (b1.y - l1->y) / (l2->y - l1->y);
			l2->x = u * (l2->x-l1->x) + l1->x;
		}
		l2->y = b1.y;
	}

	if (l2->y > b2.y)
	{
		if (l2->y - l1->y)
		{
			u = (b2.y - l1->y) / (l2->y - l1->y);
			l2->x = u * (l2->x-l1->x) + l1->x;
		}
		l2->y = b2.y;
	}
}

int check_point_within_box(xy_t p, xy_t box0, xy_t box1)
{
	xy_t bmin, bmax;

	bmin = min_xy(box0, box1);
	bmax = max_xy(box0, box1);

	if (p.x < bmin.x) return 0;
	if (p.y < bmin.y) return 0;
	if (p.x > bmax.x) return 0;
	if (p.y > bmax.y) return 0;

	return 1;
}

int plane_line_clip_far_z(xyz_t *p1, xyz_t *p2, double zplane)
{
	double u;

	if (p1->z < zplane && p2->z < zplane)	// if it's all outside the plane
		return 0;

	if (p1->z < zplane)
	{
		u = (zplane - p2->z) / (p1->z-p2->z);
		p1->z = zplane;
		p1->x = u * (p1->x-p2->x) + p2->x;
		p1->y = u * (p1->y-p2->y) + p2->y;
	}

	if (p2->z < zplane)
	{
		u = (zplane - p1->z) / (p2->z-p1->z);
		p2->z = zplane;
		p2->x = u * (p2->x-p1->x) + p1->x;
		p2->y = u * (p2->y-p1->y) + p1->y;
	}

	return 1;
}
