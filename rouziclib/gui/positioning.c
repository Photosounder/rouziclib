xy_t offset_scale(xy_t pos, xy_t offset, double sm)
{
	return add_xy(offset, mul_xy(pos, set_xy(sm)));
}

xy_t offset_scale2(xy_t pos, xy_t offset, xy_t sm2)
{
	return add_xy(offset, mul_xy(pos, sm2));
}

rect_t offset_scale_rect(rect_t r, xy_t offset, double sm)
{
	return rect(	offset_scale(r.p0, offset, sm),
			offset_scale(r.p1, offset, sm));
}

rect_t offset_scale2_rect(rect_t r, xy_t offset, xy_t sm2)
{
	return rect(	offset_scale2(r.p0, offset, sm2),
			offset_scale2(r.p1, offset, sm2));
}

xy_t fit_unscaled_rect(rect_t a, rect_t f, double *sm)	// find the offset-scale so that full_frame 'f' can fit inside the container area 'a'
{
	xy_t offset, ad, fd;

	a = sort_rect(a);
	f = sort_rect(f);

	ad = get_rect_dim(a);
	fd = get_rect_dim(f);

	*sm = min_of_xy(div_xy(ad, fd));
	offset = sub_xy(get_rect_centre(a), mul_xy(get_rect_centre(f), set_xy(*sm)));

	return offset;
}

void area_to_area_transform(rect_t a, rect_t b, xy_t *tmul, xy_t *tadd)		// find how to multiply and add in order to fit area B into area A
{
	xy_t ad, bd;

	a = sort_rect(a);
	b = sort_rect(b);

	ad = get_rect_dim(a);
	bd = get_rect_dim(b);

	*tmul = div_xy_0(ad, bd);
	*tadd = sub_xy(get_rect_centre(a), mul_xy(get_rect_centre(b), *tmul));
}
