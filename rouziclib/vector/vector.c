vobj_t *alloc_vobj(int32_t count)
{
	vobj_t *o;

	o = calloc (1, sizeof(vobj_t));
	o->count = count;
	o->scale = 1.;
	o->cx = 0.;
	o->cy = 0.;

	o->seg = calloc (o->count, sizeof(seg_t));

	return o;
}

void free_vobj(vobj_t *o)
{
	if (o==NULL)
		return ;

	free(o->seg);
	free(o);
}

seg_t seg_make_xy(xy_t p1, xy_t p2, double m)
{
	seg_t s;

	s.p1 = xy_to_xyz(p1);
	s.p2 = xy_to_xyz(p2);
	s.m = m;

	return s;
}
