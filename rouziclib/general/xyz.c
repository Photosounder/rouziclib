xy_t xy(double x, double y)
{
	xy_t out;

	out.x = x;
	out.y = y;
	return out;
}

xyi_t xyi(int32_t x, int32_t y)
{
	xyi_t out;

	out.x = x;
	out.y = y;
	return out;
}

xyz_t xyz(double x, double y, double z)
{
	xyz_t out;

	out.x = x;
	out.y = y;
	out.z = z;
	return out;
}

xy_t xyz_to_xy(xyz_t in)
{
	xy_t out;

	out.x = in.x;
	out.y = in.y;

	return out;
}

xyz_t xy_to_xyz(xy_t in)
{
	xyz_t out;

	out.x = in.x;
	out.y = in.y;
	out.z = 0.;

	return out;
}

xy_t xyi_to_xy(xyi_t in)
{
	xy_t out;

	out.x = in.x;
	out.y = in.y;

	return out;
}

xyi_t xy_to_xyi(xy_t in)
{
	xyi_t out;

	out.x = in.x;
	out.y = in.y;

	return out;
}

xy_t set_xy(double v)
{	return xy(v, v);
}

xyz_t set_xyz(double v)
{	return xyz(v, v, v);
}

xyi_t set_xyi(int32_t v)
{	return xyi(v, v);
}

int equal_xy(xy_t a, xy_t b)
{
	return (a.x==b.x) && (a.y==b.y);
}

int equal_xyz(xyz_t a, xyz_t b)
{
	return (a.x==b.x) && (a.y==b.y) && (a.z==b.z);
}

int equal_xyi(xyi_t a, xyi_t b)
{
	return (a.x==b.x) && (a.y==b.y);
}

xy_t add_xy(xy_t a, xy_t b)
{
	a.x += b.x;
	a.y += b.y;
	return a;
}

xyz_t add_xyz(xyz_t a, xyz_t b)
{
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	return a;
}

xyi_t add_xyi(xyi_t a, xyi_t b)
{
	a.x += b.x;
	a.y += b.y;
	return a;
}

xy_t sub_xy(xy_t a, xy_t b)
{
	a.x -= b.x;
	a.y -= b.y;
	return a;
}

xyz_t sub_xyz(xyz_t a, xyz_t b)
{
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	return a;
}

xyi_t sub_xyi(xyi_t a, xyi_t b)
{
	a.x -= b.x;
	a.y -= b.y;
	return a;
}

xy_t mul_xy(xy_t a, xy_t b)
{
	a.x *= b.x;
	a.y *= b.y;
	return a;
}

xyz_t mul_xyz(xyz_t a, xyz_t b)
{
	a.x *= b.x;
	a.y *= b.y;
	a.z *= b.z;
	return a;
}

xyi_t mul_xyi(xyi_t a, xyi_t b)
{
	a.x *= b.x;
	a.y *= b.y;
	return a;
}

xy_t div_xy(xy_t a, xy_t b)
{
	a.x /= b.x;
	a.y /= b.y;
	return a;
}

xy_t div_xy_0(xy_t a, xy_t b)
{
	if (b.x!=0.)
		a.x /= b.x;
	else
		a.x = 0.;

	if (b.y!=0.)
		a.y /= b.y;
	else
		a.y = 0.;
	return a;
}

xyz_t div_xyz(xyz_t a, xyz_t b)
{
	a.x /= b.x;
	a.y /= b.y;
	a.z /= b.z;
	return a;
}

xyi_t div_xyi(xyi_t a, xyi_t b)
{
	a.x /= b.x;
	a.y /= b.y;
	return a;
}

xy_t neg_xy(xy_t a)
{
	a.x = -a.x;
	a.y = -a.y;
	return a;
}

xyz_t neg_xyz(xyz_t a)
{
	a.x = -a.x;
	a.y = -a.y;
	a.z = -a.z;
	return a;
}

xyi_t neg_xyi(xyi_t a)
{
	a.x = -a.x;
	a.y = -a.y;
	return a;
}

xy_t inv_xy(xy_t a)
{
	a.x = 1. / a.x;
	a.y = 1. / a.y;
	return a;
}

xyz_t inv_xyz(xyz_t a)
{
	a.x = 1. / a.x;
	a.y = 1. / a.y;
	a.z = 1. / a.z;
	return a;
}

xyi_t inv_xyi(xyi_t a)
{
	a.x = 1. / a.x;
	a.y = 1. / a.y;
	return a;
}

xy_t neg_x(xy_t a)
{
	a.x = -a.x;
	return a;
}

xy_t neg_y(xy_t a)
{
	a.y = -a.y;
	return a;
}

xy_t func1_xy(xy_t a, double (*f)(double))
{
	a.x = f(a.x);
	a.y = f(a.y);
	return a;
}

xyz_t func1_xyz(xyz_t a, double (*f)(double))
{
	a.x = f(a.x);
	a.y = f(a.y);
	a.z = f(a.z);
	return a;
}

xyi_t func1_xyi(xyi_t a, double (*f)(double))
{
	a.x = f(a.x);
	a.y = f(a.y);
	return a;
}

xy_t func2_xy(xy_t a, xy_t b, double (*f)(double,double))
{
	a.x = f(a.x, b.x);
	a.y = f(a.y, b.y);
	return a;
}

xyz_t func2_xyz(xyz_t a, xyz_t b, double (*f)(double,double))
{
	a.x = f(a.x, b.x);
	a.y = f(a.y, b.y);
	a.z = f(a.z, b.z);
	return a;
}

xyi_t func2_xyi(xyi_t a, xyi_t b, double (*f)(double,double))
{
	a.x = f(a.x, b.x);
	a.y = f(a.y, b.y);
	return a;
}

xyi_t rshift_xyi(xyi_t a, int sh)
{
	a.x >>= sh;
	a.y >>= sh;

	return a;
}

// the following functions return individual minimums or maximums
xy_t min_xy(xy_t a, xy_t b)
{
	a.x = MINN(a.x, b.x);
	a.y = MINN(a.y, b.y);
	return a;
}

xyz_t min_xyz(xyz_t a, xyz_t b)
{
	a.x = MINN(a.x, b.x);
	a.y = MINN(a.y, b.y);
	a.z = MINN(a.z, b.z);
	return a;
}

xyi_t min_xyi(xyi_t a, xyi_t b)
{
	a.x = MINN(a.x, b.x);
	a.y = MINN(a.y, b.y);
	return a;
}

xy_t max_xy(xy_t a, xy_t b)
{
	a.x = MAXN(a.x, b.x);
	a.y = MAXN(a.y, b.y);
	return a;
}

xyz_t max_xyz(xyz_t a, xyz_t b)
{
	a.x = MAXN(a.x, b.x);
	a.y = MAXN(a.y, b.y);
	a.z = MAXN(a.z, b.z);
	return a;
}

xyi_t max_xyi(xyi_t a, xyi_t b)
{
	a.x = MAXN(a.x, b.x);
	a.y = MAXN(a.y, b.y);
	return a;
}

void minmax_xy(xy_t *a, xy_t *b)
{
	minmax_double(&a->x, &b->x);
	minmax_double(&a->y, &b->y);
}

void minmax_xyz(xyz_t *a, xyz_t *b)
{
	minmax_double(&a->x, &b->x);
	minmax_double(&a->y, &b->y);
	minmax_double(&a->z, &b->z);
}

void minmax_xyi(xyi_t *a, xyi_t *b)
{
	minmax_i32(&a->x, &b->x);
	minmax_i32(&a->y, &b->y);
}

// the following functions return minimums or maximums of all components
double min_of_xy(xy_t a)
{
	return MINN(a.x, a.y);
}

double max_of_xy(xy_t a)
{
	return MAXN(a.x, a.y);
}

double min_of_xyz(xyz_t a)
{
	return MINN(MINN(a.x, a.y), a.z);
}

double max_of_xyz(xyz_t a)
{
	return MAXN(MAXN(a.x, a.y), a.z);
}

int mul_x_by_y_xyi(xyi_t a)
{
	return a.x * a.y;
}

xy_t rangelimit_xy(xy_t v, xy_t l0, xy_t l1)
{
	return xy(	rangelimit( v.x, l0.x, l1.x ), 
			rangelimit( v.y, l0.y, l1.y ) 
		 );
}

xyz_t rangelimit_xyz(xyz_t v, xyz_t l0, xyz_t l1)
{
	return xyz(	rangelimit( v.x, l0.x, l1.x ), 
			rangelimit( v.y, l0.y, l1.y ), 
			rangelimit( v.z, l0.z, l1.z )
		 );
}

xyi_t rangelimit_xyi(xyi_t v, xyi_t l0, xyi_t l1)
{
	return xyi(	rangelimit_i32( v.x, l0.x, l1.x ), 
			rangelimit_i32( v.y, l0.y, l1.y ) 
		 );
}

xy_t fma_xy(xy_t pos, xy_t tmul, xy_t tadd)
{
	return add_xy(tadd, mul_xy(pos, tmul));
}

double get_xyz_index(xyz_t v, const int index)
{
	switch (index)
	{
		case 0:	return v.x;
		case 1:	return v.y;
		case 2:	return v.z;
	}

	return 0.;
}

matrix_t matrix_xyz(xyz_t x, xyz_t y, xyz_t z)
{
	matrix_t out;

	out.x = x;
	out.y = y;
	out.z = z;
	return out;
}

matrix_t matrices_mul(matrix_t a, matrix_t b)
{
	matrix_t c;

	c.x = xyz( a.x.x*b.x.x + a.x.y*b.y.x + a.x.z*b.z.x , a.x.x*b.x.y + a.x.y*b.y.y + a.x.z*b.z.y , a.x.x*b.x.z + a.x.y*b.y.z + a.x.z*b.z.z );
	c.y = xyz( a.y.x*b.x.x + a.y.y*b.y.x + a.y.z*b.z.x , a.y.x*b.x.y + a.y.y*b.y.y + a.y.z*b.z.y , a.y.x*b.x.z + a.y.y*b.y.z + a.y.z*b.z.z );
	c.z = xyz( a.z.x*b.x.x + a.z.y*b.y.x + a.z.z*b.z.x , a.z.x*b.x.y + a.z.y*b.y.y + a.z.z*b.z.y , a.z.x*b.x.z + a.z.y*b.y.z + a.z.z*b.z.z );
	return c;
}

xyz_t matrix_mul(xyz_t v, matrix_t m)
{
	xyz_t r;

	r.x = v.x*m.x.x + v.y*m.x.y + v.z*m.x.z;
	r.y = v.x*m.y.x + v.y*m.y.y + v.z*m.y.z;
	r.z = v.x*m.z.x + v.y*m.z.y + v.z*m.z.z;

	return r;
}
