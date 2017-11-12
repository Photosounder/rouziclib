xy_t offset_scale2(xy_t pos, xy_t offset, xy_t sm2)
{
	return add_xy(offset, mul_xy(pos, sm2));
}

xy_t offset_scale(xy_t pos, xy_t offset, double sm)
{
	return offset_scale2(pos, offset, set_xy(sm));
}

rect_t offset_scale2_rect(rect_t r, xy_t offset, xy_t sm2)
{
	return rect(	offset_scale2(r.p0, offset, sm2),
			offset_scale2(r.p1, offset, sm2));
}

rect_t offset_scale_rect(rect_t r, xy_t offset, double sm)
{
	return offset_scale2_rect(r, offset, set_xy(sm));
}

xy_t offset_scale_inv(xy_t pos, xy_t offset, double sm)
{
	return div_xy( sub_xy(pos, offset) , set_xy(sm));
}

rect_t offset_scale_inv_rect(rect_t r, xy_t offset, double sm)
{
	return rect(	offset_scale_inv(r.p0, offset, sm),
			offset_scale_inv(r.p1, offset, sm));
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

void rect_code_tool_fullarg(raster_t fb, zoom_t zc, mouse_t mouse, double drawing_thickness,
		xy_t offset, double sm)
{
	static rect_t r;
	char b[4][32], string[256];

	draw_unit_grid(offset, sm);

	if (mouse.b.rmb == 2)
		r.p0 = offset_scale_inv(mouse.u, offset, sm);

	if (mouse.b.rmb == -2)
	{
		r.p1 = offset_scale_inv(mouse.u, offset, sm);

		sprint_fractional_12(b[0], r.p0.x);
		sprint_fractional_12(b[1], r.p0.y);
		sprint_fractional_12(b[2], r.p1.x-r.p0.x);
		sprint_fractional_12(b[3], r.p1.y-r.p0.y);

		sprintf(string, "make_rect_off( doztof_xy(\"%s\", \"%s\"), doztof_xy(\"%s\", \"%s\"), XY0 );", b[0], b[1], b[2], b[3]);

		fprintf_rl(stdout, "%s\n", string);

		#ifdef RL_SDL
		SDL_SetClipboardText(string);
		#endif
	}
}
