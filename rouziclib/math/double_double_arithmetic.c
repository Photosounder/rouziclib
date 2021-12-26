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
ddouble_t add_qd(ddouble_t a, double b)
{
	ddouble_t s = add_dd_q(a.hi, b);
	return add_dd_q_quick(s.hi, a.lo + s.lo);
}

ddouble_t sub_qd(ddouble_t a, double b)
{
	ddouble_t s = sub_dd_q(a.hi, b);
	return add_dd_q_quick(s.hi, a.lo + s.lo);
}

ddouble_t sub_dq(double a, ddouble_t b)
{
	// FIXME probably not ideal
	return add_qd(neg_q(b), a);
}

ddouble_t mul_qd(ddouble_t a, double b)
{
	ddouble_t c = mul_dd_q(a.hi, b);
	return add_dd_q_quick(c.hi, fma(a.lo, b, c.lo));
}

ddouble_t div_qd(ddouble_t a, double b)
{
	double t_hi = a.hi / b;
	ddouble_t p = mul_dd_q(t_hi, b);
	double d_hi = a.hi - p.hi;
	double d_lo = a.lo - p.lo;
	double t_lo = (d_hi + d_lo) / b;
	return add_dd_q_quick(t_hi, t_lo);
}

ddouble_t div_dq(double a, ddouble_t b)
{
	// FIXME probably not ideal
	return mul_qd(recip_q(b), a);
}

// Quad operations
ddouble_t neg_q(ddouble_t a)
{
	ddouble_t r;
	r.hi = -a.hi;
	r.lo = -a.lo;
	return r;
}

ddouble_t recip_q(ddouble_t b)
{
	double t_hi = 1.0 / b.hi;
	ddouble_t r = mul_qd(b, t_hi);
	double pi_hi = 1.0 - r.hi;
	double d = pi_hi - r.lo;
	double t_lo = d / b.hi;
	return add_dd_q_quick(t_hi, t_lo);
}

ddouble_t add_qq(ddouble_t a, ddouble_t b)
{
	ddouble_t s = add_dd_q(a.hi, b.hi);
	ddouble_t t = add_dd_q(a.lo, b.lo);
	ddouble_t v = add_dd_q_quick(s.hi, s.lo + t.hi);
	ddouble_t z = add_dd_q_quick(v.hi, t.lo + v.lo);
	return z;
}

ddouble_t sub_qq(ddouble_t a, ddouble_t b)
{
	ddouble_t s = sub_dd_q(a.hi, b.hi);
	ddouble_t t = sub_dd_q(a.lo, b.lo);
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

ddouble_t div_qq(ddouble_t a, ddouble_t b)
{
	double t_hi = a.hi / b.hi;
	ddouble_t r = mul_qd(b, t_hi);
	double pi_hi = a.hi - r.hi;
	double d = pi_hi + (a.lo - r.lo);
	double t_lo = d / b.hi;
	return add_dd_q_quick(t_hi, t_lo);
}

int cmp_qq(const ddouble_t *a, const ddouble_t *b)
{
	if (a->hi > b->hi) return 1;
	if (a->hi < b->hi) return -1;
	if (a->lo == b->lo) return 0;
	if (a->lo > b->lo) return 1;
	return -1;
}

int cmp_qd(const ddouble_t *a, const double *b)
{
	if (a->hi > *b) return 1;
	if (a->hi < *b) return -1;
	if (a->lo == 0.) return 0;
	if (a->lo > 0.) return 1;
	return -1;
}
