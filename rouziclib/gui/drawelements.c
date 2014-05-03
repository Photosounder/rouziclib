void draw_titled_roundrect_frame(lrgb_t *fb, int32_t kW, int32_t kH, double x1, double y1, double radius, double xc, double yc, double spacex, double spacey)
{
	double frw = spacex-3.5, frh = spacey-3.5;
	lrgb_t colour = make_colour_lin(0.02, 0.02, 0.02, 1.);

	xc--;
	yc--;
	x1 -= 0.5 * frw + 0.5;
	y1 -= 0.5 * frh + 3.5;

	draw_roundrect_frame(fb, kW, kH,
			x1, y1,
			x1+frw + xc*spacex, y1+frh + yc*spacey,
			x1+1., y1+17.5,
			x1+frw-1. + xc*spacex, y1+frh-1. + yc*spacey,
						9., 8., radius, colour, MUL, 1.);
}
