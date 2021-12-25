// Double to quad basic operations
ddouble_t add_dd_q_quick(double a, double b)
{
	ddouble_t r;
	r.hi = a + b;
	r.lo = b - (r.hi - a);
	return r;
}

ddouble_t add_dd_q(double a, double b)
{
	ddouble_t r;
	r.hi = a + b;
	double v = r.hi - a;
	r.lo = (a - (r.hi - v)) + (b - v);
	return r;
}

ddouble_t sub_dd_q(double a, double b)
{
	ddouble_t r;
	r.hi = a - b;
	double v = r.hi - a;
	r.lo = (a - (r.hi - v)) - (b + v);
	return r;
}

ddouble_t mul_dd_q(double a, double b)
{
	ddouble_t r;
	r.hi = a * b;
	r.lo = fma(a, b, -r.hi);
	return r;
}

// Mixed quad/double operations
ddouble_t add_qd(ddouble_t x, double y)
{
	ddouble_t s = add_dd_q(x.hi, y);
	return add_dd_q_quick(s.hi, x.lo + s.lo);
}

ddouble_t sub_qd(ddouble_t x, double y)
{
	ddouble_t s = sub_dd_q(x.hi, y);
	return add_dd_q_quick(s.hi, x.lo + s.lo);
}

ddouble_t sub_dq(double x, ddouble_t y)
{
	// FIXME probably not ideal
	return add_qd(neg_q(y), x);
}

ddouble_t mul_qd(ddouble_t x, double y)
{
	ddouble_t c = mul_dd_q(x.hi, y);
	return add_dd_q_quick(c.hi, fma(x.lo, y, c.lo));
}

ddouble_t div_qd(ddouble_t x, double y)
{
	double t_hi = x.hi / y;
	ddouble_t p = mul_dd_q(t_hi, y);
	double d_hi = x.hi - p.hi;
	double d_lo = x.lo - p.lo;
	double t_lo = (d_hi + d_lo) / y;
	return add_dd_q_quick(t_hi, t_lo);
}

ddouble_t div_dq(double x, ddouble_t y)
{
	// FIXME probably not ideal
	return mul_qd(recip_q(y), x);
}

// Quad operations
ddouble_t neg_q(ddouble_t a)
{
	ddouble_t r;
	r.hi = -a.hi;
	r.lo = -a.lo;
	return r;
}

ddouble_t recip_q(ddouble_t y)
{
	double t_hi = 1.0 / y.hi;
	ddouble_t r = mul_qd(y, t_hi);
	double pi_hi = 1.0 - r.hi;
	double d = pi_hi - r.lo;
	double t_lo = d / y.hi;
	return add_dd_q_quick(t_hi, t_lo);
}

ddouble_t add_qq(ddouble_t x, ddouble_t y)
{
	ddouble_t s = add_dd_q(x.hi, y.hi);
	ddouble_t t = add_dd_q(x.lo, y.lo);
	ddouble_t v = add_dd_q_quick(s.hi, s.lo + t.hi);
	ddouble_t z = add_dd_q_quick(v.hi, t.lo + v.lo);
	return z;
}

ddouble_t sub_qq(ddouble_t x, ddouble_t y)
{
	ddouble_t s = sub_dd_q(x.hi, y.hi);
	ddouble_t t = sub_dd_q(x.lo, y.lo);
	ddouble_t v = add_dd_q_quick(s.hi, s.lo + t.hi);
	ddouble_t z = add_dd_q_quick(v.hi, t.lo + v.lo);
	return z;
}

ddouble_t mul_qq(ddouble_t a, ddouble_t b)
{
	ddouble_t c = mul_dd_q(a.hi, b.hi);
	double t = a.hi * b.lo;
	t = fma(a.lo, b.hi, t);
	return add_dd_q_quick(c.hi, c.lo + t);
}

ddouble_t div_qq(ddouble_t x, ddouble_t y)
{
	double t_hi = x.hi / y.hi;
	ddouble_t r = mul_qd(y, t_hi);
	double pi_hi = x.hi - r.hi;
	double d = pi_hi + (x.lo - r.lo);
	double t_lo = d / y.hi;
	return add_dd_q_quick(t_hi, t_lo);
}
