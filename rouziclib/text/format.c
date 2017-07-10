char *sprint_large_num_simple(char *string, double number)		// prints a large number in a simple human-readable way into string (overwrites)
{
	static char static_string[32];
	const char *large_units[] = {"thousand", "million", "billion", "trillion"};
	const char *small_units[] = {"thousandths", "millionths", "billionths", "trillionths"};
	char *ptr=NULL;
	int logv, logv3, name_index, neg=0;
	double vm, vexp;
	/*
		logv	number		result
		-5	0.000012345	12.3 millionths
		-4	0.00012345	0.123 thousandths
		-3	0.0012345	1.23 thousandths	<-- general rule ends here
		-2	0.012345	0.012
		-1	0.12345		0.123
		 0	1.2345		1.23
		 1	12.345		12.3
		 2	123.45		123
		 3	1,234.5		1,234
		 4	12,345		12.3 thousand		<-- general rule starts there
		 5	123,456		123 thousand
		 6	1,234,567	1.23 million
		 7	12,345,678	12.3 million
	*/

	if (string==NULL)
		string = static_string;
	string[0] = 0;

	if (number < 0.)
	{
		neg = 1;
		number = -number;
	}

	logv = log10(number);			// 16 million -> 7
	if (number < 1.)
		logv--;

	if (logv < -2)
		logv3 = ((logv-1) / 3) * 3;	// shifts logv3 a notch to our preference
	else
		logv3 = (logv / 3) * 3;		// 16 million -> 7 -> 6

	vexp = pow(10., (double) logv3);	// 16 million -> 1,000,000.
	vm = number / vexp;			// 16 million -> 16
	if (neg)
		vm = -vm;
	name_index = abs(logv3) / 3 - 1;	// 16 million -> 1 ("million")

	switch (logv)
	{
		// special cases
		case -2:
		case -1:
			sprintf(string, "%.3f", vm);
			break;
		case 0:
			sprintf(string, "%.2f", vm);
			break;
		case 1:
			sprintf(string, "%.1f", vm);
			break;
		case 2:
			sprintf(string, "%.0f", vm);
			break;
		case 3:
			sprintf(string, "%.3f", vm);
			ptr = strstr(string, ".");
			if (ptr)
				ptr[0] = ',';
			break;

		// general rule
		default:
			if (name_index > 3)
				sprintf(string, "%.3g e%+03d", vm, logv3);
			else if (logv > 0)
				sprintf(string, "%.3g %s", vm, large_units[name_index]);
			else
				sprintf(string, "%.3g %s", vm, small_units[name_index]);
	}

	return string;
}

char *sprint_fractional_12(char *string, double v)
{
	int d, p1, p2, neg=0, n;

	if (v < 0.)
	{
		neg = 1;
		v = -v;
	}

	d = nearbyint(v * 144.);
	p2 = d % 12;
	d /= 12;
	p1 = d % 12;
	d /= 12;

	string[0] = 0;
	if (neg)
		sprintf(string, "-");

	if (p2)
		sprintf(&string[strlen(string)], "%d;%d;%d", d, p1, p2);
	else if (p1)
		sprintf(&string[strlen(string)], "%d;%d", d, p1);
	else
		sprintf(&string[strlen(string)], "%d", d);

	return string;
}

char *sprint_compile_date(char *string, const char *location)
{
	sprintf(string, "Compiled on %s at %s", __DATE__, __TIME__);
	if (location)
		sprintf(&string[strlen(string)], " in %s", location);

	return string;
}

void print_valfmt(char *str, int str_size, double v, const int valfmt)
{
	switch (valfmt)
	{
		case VALFMT_DEFAULT:	snprintf(str, str_size, "%.2f", v);		break;
		case VALFMT_3F:		snprintf(str, str_size, "%.3f", v);		break;
		case VALFMT_PCT_2F:	snprintf(str, str_size, "%.2f%%", v*100.);	break;
		default:		snprintf(str, str_size, "%g", v);
	}
}
