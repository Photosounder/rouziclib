void find_linear_fit(double (*f)(double), double start, double end, double *c0p, double *c1p)
{
	double c0, c1, middle;

	c1 = (f(end) - f(start)) / (end - start);

	middle = (end+start) * 0.5;
	c0 = ((f(start)-c1*start) + (f(middle)-c1*middle)) * 0.5;

	*c0p = c0;
	*c1p = c1;
}

void find_quadratic_fit(double (*f)(double), double start, double end, double *c0p, double *c1p, double *c2p)
{
	double c0, c1, c2, middle, p1, p2, height2, width2;

	c1 = (f(end) - f(start)) / (end - start);

	middle = (end+start) * 0.5;
	height2 = (f(end)-c1*end) - (f(middle)-c1*middle);
	width2 = end - middle;
	c2 = height2 / (width2*width2);

	p1 = 0.75*(end-start) + start;
	p2 = 0.25*(end-start) + start;
	c1 = ((f(p1)-c2*p1*p1) - (f(start)-c2*start*start)) / (p1-start);
	c1 += ((f(end)-c2*end*end) - (f(p2)-c2*p2*p2)) / (end-p2);
	c1 *= 0.5;

	c0 = ((f(end)-((c2*end + c1)*end)) + (f(start)-((c2*start + c1)*start))) * 0.5;

	*c0p = c0;
	*c1p = c1;
	*c2p = c2;
}
