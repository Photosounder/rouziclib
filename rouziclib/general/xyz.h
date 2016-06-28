#define XY0	xy(0.,0.)

typedef struct
{
	double x, y;
} xy_t;

typedef struct
{
	int32_t x, y;
} xyi_t;

typedef struct
{
	double x, y, z;
} xyz_t;

typedef struct
{
	xyz_t x, y, z;
} matrix_t;

static xy_t xy(double x, double y)
{
	xy_t out;

	out.x = x;
	out.y = y;
	return out;
}

static xyi_t xyi(int32_t x, int32_t y)
{
	xyi_t out;

	out.x = x;
	out.y = y;
	return out;
}

static xyz_t xyz(double x, double y, double z)
{
	xyz_t out;

	out.x = x;
	out.y = y;
	out.z = z;
	return out;
}

static xy_t xyz_to_xy(xyz_t in)
{
	xy_t out;

	out.x = in.x;
	out.y = in.y;

	return out;
}

static xyz_t xy_to_xyz(xy_t in)
{
	xyz_t out;

	out.x = in.x;
	out.y = in.y;
	out.z = 0.;

	return out;
}

static xy_t xyi_to_xy(xyi_t in)
{
	xy_t out;

	out.x = in.x;
	out.y = in.y;

	return out;
}

static xyi_t xy_to_xyi(xy_t in)
{
	xyi_t out;

	out.x = in.x;
	out.y = in.y;

	return out;
}

#define zyx(z, y, x)	xyz(x, y, z)

static matrix_t matrix_xyz(xyz_t x, xyz_t y, xyz_t z)
{
	matrix_t out;

	out.x = x;
	out.y = y;
	out.z = z;
	return out;
}

static xy_t set_xy(double v)
{	return xy(v, v);
}

static xyz_t set_xyz(double v)
{	return xyz(v, v, v);
}

static int equal_xy(xy_t a, xy_t b)
{
	return (a.x==b.x) && (a.y==b.y);
}

static int equal_xyz(xyz_t a, xyz_t b)
{
	return (a.x==b.x) && (a.y==b.y) && (a.z==b.z);
}

static int equal_xyi(xyi_t a, xyi_t b)
{
	return (a.x==b.x) && (a.y==b.y);
}

static xy_t add_xy(xy_t a, xy_t b)
{
	a.x += b.x;
	a.y += b.y;
	return a;
}

static xyz_t add_xyz(xyz_t a, xyz_t b)
{
	a.x += b.x;
	a.y += b.y;
	a.z += b.z;
	return a;
}

static xy_t sub_xy(xy_t a, xy_t b)
{
	a.x -= b.x;
	a.y -= b.y;
	return a;
}

static xyz_t sub_xyz(xyz_t a, xyz_t b)
{
	a.x -= b.x;
	a.y -= b.y;
	a.z -= b.z;
	return a;
}

static xy_t mul_xy(xy_t a, xy_t b)
{
	a.x *= b.x;
	a.y *= b.y;
	return a;
}

static xyz_t mul_xyz(xyz_t a, xyz_t b)
{
	a.x *= b.x;
	a.y *= b.y;
	a.z *= b.z;
	return a;
}

static xy_t div_xy(xy_t a, xy_t b)
{
	a.x /= b.x;
	a.y /= b.y;
	return a;
}

static xyz_t div_xyz(xyz_t a, xyz_t b)
{
	a.x /= b.x;
	a.y /= b.y;
	a.z /= b.z;
	return a;
}

static xy_t neg_xy(xy_t a)
{
	a.x = -a.x;
	a.y = -a.y;
	return a;
}

static xyz_t neg_xyz(xyz_t a)
{
	a.x = -a.x;
	a.y = -a.y;
	a.z = -a.z;
	return a;
}

static xy_t inv_xy(xy_t a)
{
	a.x = 1. / a.x;
	a.y = 1. / a.y;
	return a;
}

static xyz_t inv_xyz(xyz_t a)
{
	a.x = 1. / a.x;
	a.y = 1. / a.y;
	a.z = 1. / a.z;
	return a;
}

static xy_t neg_x(xy_t a)
{
	a.x = -a.x;
	return a;
}

static xy_t neg_y(xy_t a)
{
	a.y = -a.y;
	return a;
}

#define abs_xy(a)	func1_xy(a, fabs)
#define abs_xyz(a)	func1_xyz(a, fabs)
#define cos_xy(a)	func1_xy(a, cos)
#define cos_xyz(a)	func1_xyz(a, cos)
#define sin_xy(a)	func1_xy(a, sin)
#define sin_xyz(a)	func1_xyz(a, sin)
#define floor_xy(a)	func1_xy(a, floor)
#define ceil_xy(a)	func1_xy(a, ceil)

static xy_t func1_xy(xy_t a, double (*f)(double))
{
	a.x = f(a.x);
	a.y = f(a.y);
	return a;
}

static xyz_t func1_xyz(xyz_t a, double (*f)(double))
{
	a.x = f(a.x);
	a.y = f(a.y);
	a.z = f(a.z);
	return a;
}

static xy_t min_xy(xy_t a, xy_t b)	// returns individual minimums
{
	a.x = MINN(a.x, b.x);
	a.y = MINN(a.y, b.y);
	return a;
}

static xyz_t min_xyz(xyz_t a, xyz_t b)	// returns individual minimums
{
	a.x = MINN(a.x, b.x);
	a.y = MINN(a.y, b.y);
	a.z = MINN(a.z, b.z);
	return a;
}

static xy_t max_xy(xy_t a, xy_t b)	// returns individual maximums
{
	a.x = MAXN(a.x, b.x);
	a.y = MAXN(a.y, b.y);
	return a;
}

static xyz_t max_xyz(xyz_t a, xyz_t b)	// returns individual maximums
{
	a.x = MAXN(a.x, b.x);
	a.y = MAXN(a.y, b.y);
	a.z = MAXN(a.z, b.z);
	return a;
}

static matrix_t matrices_mul(matrix_t a, matrix_t b)
{
	matrix_t c;

	c.x = xyz( a.x.x*b.x.x + a.x.y*b.y.x + a.x.z*b.z.x , a.x.x*b.x.y + a.x.y*b.y.y + a.x.z*b.z.y , a.x.x*b.x.z + a.x.y*b.y.z + a.x.z*b.z.z );
	c.y = xyz( a.y.x*b.x.x + a.y.y*b.y.x + a.y.z*b.z.x , a.y.x*b.x.y + a.y.y*b.y.y + a.y.z*b.z.y , a.y.x*b.x.z + a.y.y*b.y.z + a.y.z*b.z.z );
	c.z = xyz( a.z.x*b.x.x + a.z.y*b.y.x + a.z.z*b.z.x , a.z.x*b.x.y + a.z.y*b.y.y + a.z.z*b.z.y , a.z.x*b.x.z + a.z.y*b.y.z + a.z.z*b.z.z );
	return c;
}
