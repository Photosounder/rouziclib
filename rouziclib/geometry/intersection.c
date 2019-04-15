xy_t line_line_intersection(xy_t p1, xy_t p2, xy_t p3, xy_t p4)
{
	xy_t p;

	p.x = ((p1.x*p2.y-p1.y*p2.x)*(p3.x-p4.x)-(p1.x-p2.x)*(p3.x*p4.y-p3.y*p4.x)) / ((p1.x-p2.x)*(p3.y-p4.y)-(p1.y-p2.y)*(p3.x-p4.x));
	p.y = ((p1.x*p2.y-p1.y*p2.x)*(p3.y-p4.y)-(p1.y-p2.y)*(p3.x*p4.y-p3.y*p4.x)) / ((p1.x-p2.x)*(p3.y-p4.y)-(p1.y-p2.y)*(p3.x-p4.x));

	return p;
}

double pos_on_line(xy_t p1, xy_t p2, xy_t p)
{
	return ((p.x-p1.x)*(p2.x-p1.x) + (p.y-p1.y)*(p2.y-p1.y)) / ((p1.x-p2.x)*(p1.x-p2.x) + (p1.y-p2.y)*(p1.y-p2.y));
}

int check_line_collision(xy_t p1, xy_t p2, xy_t p3, xy_t p4, double *u, int32_t exclusive)
{
	double v;
	xy_t p;

	if ((p1.x==p2.x && p1.y==p2.y) || (p3.x==p4.x && p3.y==p4.y))
		return 0;

	p = line_line_intersection(p1, p2, p3, p4);

	*u = pos_on_line(p1, p2, p);
	v = pos_on_line(p3, p4, p);

	if (exclusive==0)
		if (*u>=0. && *u<=1. && v>=0. && v<=1.)
			return 1;
		else
			return 0;
	else
		if (*u>0. && *u<1. && v>0. && v<1.)
			return 1;
		else
			return 0;
}

double point_line_distance(xy_t l1, xy_t l2, xy_t p3)	// nearest point on the line
{
	double u;
	xy_t p;

	u = ((p3.x-l1.x)*(l2.x-l1.x) + (p3.y-l1.y)*(l2.y-l1.y)) / sq(hypot_xy(l2, l1));

	p = interpolate_xy(l1, l2, u);

	return hypot_xy(p, p3);
}

// Limits a line to the insides of a bounding box
// if the line is entirely outside then x1==x2 and y1==y2
void border_clip(double w, double h, xy_t *l1, xy_t *l2, double radius)
{
	line_rect_clip(l1, l2, rect( xy(0.-radius, 0.-radius), xy(w+radius, h+radius) ));
}

// Limits a line to the insides of a bounding rectangle
void line_rect_clip(xy_t *l1, xy_t *l2, rect_t br)
{
	double u;

	if (l1->x < br.p0.x)
	{
		if (l1->x - l2->x)
		{
			u = (br.p0.x - l2->x) / (l1->x - l2->x);
			l1->y = u * (l1->y-l2->y) + l2->y;
		}
		l1->x = br.p0.x;
	}

	if (l1->x > br.p1.x)
	{
		if (l1->x - l2->x)
		{
			u = (br.p1.x - l2->x) / (l1->x - l2->x);
			l1->y = u * (l1->y-l2->y) + l2->y;
		}
		l1->x = br.p1.x;
	}

	if (l1->y < br.p0.y)
	{
		if (l1->y - l2->y)
		{
			u = (br.p0.y - l2->y) / (l1->y - l2->y);
			l1->x = u * (l1->x-l2->x) + l2->x;
		}
		l1->y = br.p0.y;
	}

	if (l1->y > br.p1.y)
	{
		if (l1->y - l2->y)
		{
			u = (br.p1.y - l2->y) / (l1->y - l2->y);
			l1->x = u * (l1->x-l2->x) + l2->x;
		}
		l1->y = br.p1.y;
	}

	if (l2->x < br.p0.x)
	{
		if (l2->x - l1->x)
		{
			u = (br.p0.x - l1->x) / (l2->x - l1->x);
			l2->y = u * (l2->y-l1->y) + l1->y;
		}
		l2->x = br.p0.x;
	}

	if (l2->x > br.p1.x)
	{
		if (l2->x - l1->x)
		{
			u = (br.p1.x - l1->x) / (l2->x - l1->x);
			l2->y = u * (l2->y-l1->y) + l1->y;
		}
		l2->x = br.p1.x;
	}

	if (l2->y < br.p0.y)
	{
		if (l2->y - l1->y)
		{
			u = (br.p0.y - l1->y) / (l2->y - l1->y);
			l2->x = u * (l2->x-l1->x) + l1->x;
		}
		l2->y = br.p0.y;
	}

	if (l2->y > br.p1.y)
	{
		if (l2->y - l1->y)
		{
			u = (br.p1.y - l1->y) / (l2->y - l1->y);
			l2->x = u * (l2->x-l1->x) + l1->x;
		}
		l2->y = br.p1.y;
	}
}

int check_point_within_box(xy_t p, rect_t box)
{
	box = sort_rect(box);

	if (p.x < box.p0.x) return 0;
	if (p.y < box.p0.y) return 0;
	if (p.x > box.p1.x) return 0;
	if (p.y > box.p1.y) return 0;

	if (p.x == box.p0.x) return 2;
	if (p.y == box.p0.y) return 2;
	if (p.x == box.p1.x) return 2;
	if (p.y == box.p1.y) return 2;

	return 1;
}

int check_point_within_box_int(xyi_t p, recti_t box)
{
	box = sort_recti(box);

	if (p.x < box.p0.x) return 0;
	if (p.y < box.p0.y) return 0;
	if (p.x > box.p1.x) return 0;
	if (p.y > box.p1.y) return 0;

	if (p.x == box.p0.x) return 2;
	if (p.y == box.p0.y) return 2;
	if (p.x == box.p1.x) return 2;
	if (p.y == box.p1.y) return 2;

	return 1;
}

int check_point_within_circle(xy_t p, xy_t circ_centre, double radius)
{
	if (hypot_xy2(p, circ_centre) <= sq(radius))
		return 1;

	return 0;
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

int check_box_box_intersection(rect_t box0, rect_t box1)
{
	box0 = sort_rect(box0);
	box1 = sort_rect(box1);

	if (box0.p0.x <= box1.p1.x && box0.p1.x >= box1.p0.x)
	if (box0.p0.y <= box1.p1.y && box0.p1.y >= box1.p0.y)
		return 1;

	return 0;
}

int check_box_box_intersection_int(recti_t box0, recti_t box1)
{
	box0 = sort_recti(box0);
	box1 = sort_recti(box1);

	if (box0.p0.x <= box1.p1.x && box0.p1.x >= box1.p0.x)
	if (box0.p0.y <= box1.p1.y && box0.p1.y >= box1.p0.y)
		return 1;

	return 0;
}

rect_t rect_intersection(rect_t r1, rect_t r2)		// boolean intersection
{
	rect_t ri;

	if (check_box_box_intersection(r1, r2)==0)
		return rect( xy(NAN, NAN) , xy(NAN, NAN) );

	r1 = sort_rect(r1);
	r2 = sort_rect(r2);

	ri.p0 = max_xy(r1.p0, r2.p0);
	ri.p1 = min_xy(r1.p1, r2.p1);

	return ri;
}

recti_t recti_intersection(recti_t r1, recti_t r2)		// boolean intersection
{
	recti_t ri;

	if (check_box_box_intersection_int(r1, r2)==0)
		return recti( XYI0 , XYI0 );

	r1 = sort_recti(r1);
	r2 = sort_recti(r2);

	ri.p0 = max_xyi(r1.p0, r2.p0);
	ri.p1 = min_xyi(r1.p1, r2.p1);

	return ri;
}

int check_box_circle_intersection(rect_t box, xy_t circ, double rad)
{
	int sector;
	double d, d2;

	box = sort_rect(box);

	/*	sectors:
	 * 	0  1  2
	 * 	3  4  5
	 * 	6  7  8
	 */

	sector = (circ.y >= box.p0.y) + (circ.y > box.p1.y);
	sector *= 3;
	sector += (circ.x >= box.p0.x) + (circ.x > box.p1.x);

	switch (sector)
	{
		case 4:	return 1;

		case 0:	d2 = hypot_xy2(circ, box.p0);		break;
		case 2:	d2 = hypot_xy2(circ, rect_p10(box));	break;
		case 6:	d2 = hypot_xy2(circ, rect_p01(box));	break;
		case 8:	d2 = hypot_xy2(circ, box.p1);		break;

		case 1:	d = box.p0.y - circ.y;	break;
		case 3:	d = box.p0.x - circ.x;	break;
		case 5:	d = circ.x - box.p1.x;	break;
		case 7:	d = circ.y - box.p1.y;	break;
	}

	if (sector&1)
		return d <= rad;
	else
		return d2 <= rad*rad;
}

int check_box_wholly_inside_circle(rect_t box, xy_t circ, double rad)
{
	int sector;
	double d2;
	xy_t rc;

	box = sort_rect(box);
	rc = get_rect_centre(box);

	/*	sectors:
	 * 	0  1
	 * 	2  3
	 */

	sector = (circ.y >= rc.y)*2 + (circ.x >= rc.x);

	switch (sector)
	{
		case 0:	d2 = hypot_xy2(circ, box.p1);		break;
		case 1:	d2 = hypot_xy2(circ, rect_p01(box));	break;
		case 2:	d2 = hypot_xy2(circ, rect_p10(box));	break;
		case 3:	d2 = hypot_xy2(circ, box.p0);		break;
	}

	return d2 < rad*rad;
}

int check_pixel_within_image(xyi_t pos, xyi_t im_dim)
{
	if (pos.x < 0)
		return 0;

	if (pos.y < 0)
		return 0;

	if (pos.x >= im_dim.x)
		return 0;

	if (pos.y >= im_dim.y)
		return 0;

	return 1;
}
