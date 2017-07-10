void draw_titled_roundrect_frame(raster_t fb, xy_t pos, double radius, xy_t c, xy_t space, lrgb_t colour, const blend_func_t bf)
{
	xy_t fr;

	fr = sub_xy(space, set_xy(3.5));
	c = sub_xy(c, set_xy(1.));
	pos.x -= 0.5*fr.x + 0.5;
	pos.y -= 0.5*fr.y + 3.5;

	draw_roundrect_frame(fb,
			rect( pos, xy(pos.x+fr.x + c.x*space.x, pos.y+fr.y + c.y*space.y) ),
			rect( xy(pos.x+1., pos.y+17.5), xy(pos.x+fr.x-1. + c.x*space.x, pos.y+fr.y-1. + c.y*space.y)),
			9., 8., radius, colour, bf, 1.);
}

void draw_label_fullarg(raster_t fb, zoom_t zc, vector_font_t *font, double drawing_thickness, 
		uint8_t *label, rect_t box, col_t colour, const int mode)
{
	double intensity, scale = rect_min_side(box), total_scale = scale*zc.scrscale;

	if (total_scale < 1.)
		return ;

	intensity = intensity_scaling(total_scale, 24.);

	//box.p0.x += 2.*scale/LINEVSPACING;
	//box.p1.x -= 2.*scale/LINEVSPACING;

	draw_string_bestfit(fb, font, label, sc_rect(box), 0., 1e30*zc.scrscale, colour, 1.*intensity, drawing_thickness, mode, NULL);
}

void display_dialog_enclosing_frame_fullarg(raster_t fb, vector_font_t *font, double drawing_thickness, 
		rect_t box_os, double scale, char *label, col_t colour)
{
	rect_t label_box;
	double intensity;

	intensity = intensity_scaling(rect_max_side(box_os), 32.);

	draw_rect(fb, box_os, drawing_thickness, colour, blend_add, intensity);

	label_box = rect_size_mul(box_os, set_xy(1. - 16./144.));

	draw_string_bestfit(fb, font, label, label_box, 0., 1e30, colour, intensity * (0.5+0.5*erf((0.75-scale)*6.)), drawing_thickness, ALIG_CENTRE, NULL);
}
