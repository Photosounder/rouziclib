float gaussian(float x)
{
	return native_exp(-x*x);	// ~6 FR
}

float erf_fast(float x)
{
	float y, xa = min(fabs(x), 4.45f);

	// erf(x) ~= 1 - (<polynomial>)^8 for x >= 0, max error 8.4e-6
	// 5 mad, 3 mul, 1 +-, 1 min, 1 copysign = ~11 FR
	y = ((((-0.001133515f*xa + 0.00830125f)*xa - 0.003218f)*xa - 0.06928657f)*xa - 0.14108256f)*xa + 1.f;

	// y = y^8
	y = y*y;
	y = y*y;
	y = y*y;

	y = 1.f - y;
	y = copysign(y, x);

	return y;
}

float angle_from_norm_coord_xpositive(float x, float y)
{
	float pa, paa, pas, ta;

	pa = (x - 2.f) * y;	// pseudo-angle in [-2 , 2] range
	paa = fabs(pa);
	pas = sign(pa);

	// True angle approximation, [0 , 0.5] range for [0 deg , 180 deg], error 1/17828 (0.02 deg)
	ta = (((((0.0059196463f*paa - 0.034352064f)*paa + 0.079805054f)*paa - 0.080373173f)*paa + 0.0047036615f)*paa + 0.15882123f)*paa;
	ta = ta*pas + 0.25f;

	return ta;
}

float angle_from_norm_coord(float x, float y)	// (x,y) is anywhere on the unit circle, output is in turns in the [-0.5 , 0.5] range
{
	float ta;
	ta = angle_from_norm_coord_xpositive(fabs(x), y);
	ta = copysign(ta, x);

	return ta;
}
