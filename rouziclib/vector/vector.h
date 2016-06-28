typedef struct
{
	xyz_t p1, p2, r1, r2;
	double m;
} seg_t;

typedef struct
{
	int32_t count;	// number of segments
	double scale;	// on screen scale
	double cx, cy;	// centre of rotation/gravity
	seg_t *seg;	// segments
} vobj_t;

extern vobj_t *alloc_vobj(int32_t count);
extern void free_vobj(vobj_t *o);
extern seg_t seg_make_xy(xy_t p1, xy_t p2, double m);
