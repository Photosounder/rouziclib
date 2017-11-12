rect_t *rect_insert=NULL;	// array that contains the rectangles matching the screen positions of the provided insert-space characters
int insert_rect_alloc=0;

void reset_insert_rect_array()
{
	if (rect_insert)
		memset(rect_insert, 0xFF, insert_rect_alloc*sizeof(rect_t));
}

void report_insert_rect_pos(xy_t pos, xy_t dim, int bidi, uint32_t cp, int index)
{
	if (rect_insert==NULL)
		rect_insert = calloc(insert_rect_alloc=4, sizeof(rect_t));

	index -= cp_ins_index_base;

	if (index < 0 || index > 239)	// limiting index to the range of supplemental variation selectors
		return ;

	alloc_enough(&rect_insert, index+1, &insert_rect_alloc, sizeof(rect_t), 2);

	if (cp==cp_ins_nul)
		dim.x = 0.;

	rect_insert[index] = make_rect_off( pos, dim, xy(bidi==-2 ? 1. : 0., 0.) );
}

rect_t get_insert_rect_zc(zoom_t zc, int index)
{
	if (index < 0 || index >= insert_rect_alloc || rect_insert==NULL)
		return rect( xy(NAN, NAN) , xy(NAN, NAN) );

	return wc_rect(rect_insert[index]);
}

rect_t insert_rect_change_height(rect_t r, double low, double high)
{
	double y, m;

	y = r.p0.y;
	m = (r.p1.y - r.p0.y) / 6.;

	r.p0.y = low * m + y;
	r.p1.y = high * m + y;

	return r;
}
