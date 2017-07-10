void draw_rect_full_cl(raster_t fb, rect_t box, double radius, frgb_t colour, double intensity)
{
#ifdef RL_OPENCL
	float *df;
	double grad;
	int32_t i, ix, iy;
	rect_t bb, fr, screen_box = rect(XY0, xy(fb.w-1, fb.h-1));
	recti_t bbi, fri;

	if (intensity==0.)
		return ;
	
	grad = GAUSSRAD_HQ * radius;		// erfr and gaussian can go up to x = ±4

	box = sort_rect(box);

	// calculate the bounding box
	bb.p0 = ceil_xy(sub_xy(box.p0, set_xy(grad)));
	bb.p1 = floor_xy(add_xy(box.p1, set_xy(grad)));

	if (check_box_box_intersection(bb, screen_box)==0)
		return ;

	bbi.p0 = xy_to_xyi(max_xy(bb.p0, screen_box.p0));
	bbi.p1 = xy_to_xyi(min_xy(bb.p1, screen_box.p1));
	bbi = rshift_recti(bbi, fb.sector_size);

	// calculate the fill rectangle, which is the inner rectangle where sectors are just filled plainly
	fr.p0 = ceil_xy(add_xy(box.p0, set_xy(grad)));
	fr.p1 = floor_xy(sub_xy(box.p1, set_xy(grad)));

	fri.p0 = xy_to_xyi(max_xy(fr.p0, screen_box.p0));
	fri.p1 = xy_to_xyi(min_xy(fr.p1, screen_box.p1));
	fri = rshift_recti(fri, fb.sector_size);

	if (fri.p1.x <= fri.p0.x || fri.p1.y <= fri.p0.y)	// if there is no plain fill area
		fri = recti(xyi(-1,-1), xyi(-1,-1));

	// calculate the drawing parameters
	colour.r *= intensity;
	colour.g *= intensity;
	colour.b *= intensity;

	// store the drawing parameters in the main drawing queue
	df = drawq_add_to_main_queue(fb, DQT_RECT_FULL);
	df[0] = box.p0.x;
	df[1] = box.p0.y;
	df[2] = box.p1.x;
	df[3] = box.p1.y;
	df[4] = 1./radius;
	df[5] = colour.r;
	df[6] = colour.g;
	df[7] = colour.b;

	// find the affected sectors
	for (iy=bbi.p0.y; iy<=bbi.p1.y; iy++)
		for (ix=bbi.p0.x; ix<=bbi.p1.x; ix++)
			if (check_point_within_box_int(xyi(ix, iy), fri)!=1)	// if we're not inside the plain fill area
				drawq_add_sector_id(fb, iy*fb.sector_w + ix);	// add sector reference

	if (fri.p0.x == -1)
		return ;

	//************PLAIN FILL AREA************

	df = drawq_add_to_main_queue(fb, DQT_PLAIN_FILL);
	df[0] = colour.r;
	df[1] = colour.g;
	df[2] = colour.b;

	// find the affected sectors
	for (iy=fri.p0.y; iy<=fri.p1.y; iy++)
		for (ix=fri.p0.x; ix<=fri.p1.x; ix++)
			if (check_point_within_box_int(xyi(ix, iy), fri)==1)	// if we're inside the plain fill area
				drawq_add_sector_id(fb, iy*fb.sector_w + ix);	// add sector reference
#endif
}

void draw_rect_full(raster_t fb, rect_t box, double radius, col_t colour, double intensity)
{
	radius = drawing_focus_adjust(focus_rlg, radius, NULL, 0);	// adjusts the focus

	if (fb.use_cl)
		draw_rect_full_cl(fb, box, radius, col_to_frgb(colour), intensity);
}
