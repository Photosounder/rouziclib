// Based on https://github.com/tuwien-cms/xprec/blob/mainline/csrc/dd_arith.h itself based on https://github.com/scibuilder/QD/blob/master/include/qd/inline.h / https://github.com/scibuilder/QD/blob/master/include/qd/dd_inline.h

// in ../general/structs.h
// ddouble_t

extern ddouble_t add_dd_q_quick(double a, double b);
extern ddouble_t add_dd_q(double a, double b);
extern ddouble_t sub_dd_q(double a, double b);
extern ddouble_t mul_dd_q(double a, double b);

extern ddouble_t add_qd(ddouble_t a, double b);
extern ddouble_t sub_qd(ddouble_t a, double b);
extern ddouble_t sub_dq(double a, ddouble_t b);
extern ddouble_t mul_qd(ddouble_t a, double b);
extern ddouble_t div_qd(ddouble_t a, double b);
extern ddouble_t div_dq(double a, ddouble_t b);

extern ddouble_t neg_q(ddouble_t a);
extern ddouble_t recip_q(ddouble_t b);
extern ddouble_t add_qq(ddouble_t a, ddouble_t b);
extern ddouble_t sub_qq(ddouble_t a, ddouble_t b);
extern ddouble_t mul_qq(ddouble_t a, ddouble_t b);
extern ddouble_t div_qq(ddouble_t a, ddouble_t b);

extern int cmp_qq(const ddouble_t *a, const ddouble_t *b);
extern int cmp_qd(const ddouble_t *a, const double *b);

static const ddouble_t Q_ZERO =	{0., 0.};
static const ddouble_t Q_ONE =	{1., 0.};
static const ddouble_t Q_2PI =	{6.283185307179586232, 2.449293598294706414e-16};
static const ddouble_t Q_PI =	{3.141592653589793116, 1.224646799147353207e-16};
static const ddouble_t Q_PI_2 = {1.570796326794896558, 6.123233995736766036e-17};
static const ddouble_t Q_PI_4 = {0.785398163397448279, 3.061616997868383018e-17};
static const ddouble_t Q_E =	{2.718281828459045091, 1.445646891729250158e-16};
static const ddouble_t Q_LOG2 = {0.6931471805599452862, 2.319046813846299558e-17};
static const ddouble_t Q_LOG10 = {2.302585092994045901, -2.170756223382249351e-16};
